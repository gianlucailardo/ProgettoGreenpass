#include<sys/types.h> /* predefined types */
#include<unistd.h> /* include unix standard library */
#include<arpa/inet.h> /* IP addresses conversion utililites */
#include<sys/socket.h> /* socket library */
#include<stdio.h> /* include standard I/O library */
#include <stdlib.h>
#include <string.h>
#include <errno.h>	 /* include error codes */
#include <string.h>

#define LUNGHEZZACODICE 15
ssize_t FullWrite(int fd, const void *buf, size_t count);
ssize_t FullRead(int fd, void *buf, size_t count);

//definizione del pacchetto applicazione
struct pacchettoGreenPass {
    char codiceFiscale[LUNGHEZZACODICE+1];
    int mesiGreenPass;
    int validita;
    int ID;
};


int main (int argc , char *argv[])
{
 int list_fd,conn_fd, serverV_fd;
 int nread;
 struct sockaddr_in serverG_add, serverV_add, client;
 socklen_t len;
 pid_t pid;

 //configurazione dell'indirizzo del serverG  
 //SOCKET
 if ( ( list_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    perror("socket");
    exit(1);
    }
 //SIN_FAMILY
 serverG_add.sin_family      = AF_INET;
 //SIN_ADDR.S_ADDR
 serverG_add.sin_addr.s_addr = htonl(INADDR_ANY);
 //SIN_PORT
 serverG_add.sin_port        = htons(1026);

 //BIND
 if ( bind(list_fd, (struct sockaddr *) &serverG_add, sizeof(serverG_add)) < 0 ) {
    perror("bind");
    exit(1);
  }

  //configurazione della lista d'attesa dei client
 //LISTEN
 if ( listen(list_fd, 1024) < 0 ) {
    perror("listen");
    exit(1);
  }


 while(1)
	{
	 len = sizeof ( client );
	 //accettazione delle richieste dei client
	 //ACCEPT
	 if ( ( conn_fd = accept(list_fd, (struct sockaddr *) &client, &len) ) < 0 ) {
      		perror("accept");
      		exit(1);
    	}	

	 /* fork del processo per generare un processo figlio che gestirà le richieste dei client */
	 if((pid= fork())<0)
		{
		 perror (" fork error ");
		 exit ( -1);
		}
	 if(pid==0)
		{ /* operazioni eseguite nel processo figlio*/
		 close (list_fd);
		
		//connessione col serverV
  		if ( ( serverV_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    			perror("socket");
    			exit(1);
    	    	}

	 	/* inizializzazione dell'indirizzo di serverV */
    	    	//SIN_FAMILY
    	    	serverV_add.sin_family = AF_INET;
	    	//SIN_PORT
    	    	serverV_add.sin_port   = htons(1025);

	    	
    	    	//INET_PTON
	    	if (inet_pton(AF_INET, argv[1], &serverV_add.sin_addr) < 0) {
      			fprintf(stderr,"inet_pton error for %s\n", argv[1]);
      			exit (1);
    	    	}

    	    	/* si stabilisce una connessione col server */
    	    	//CONNECT
    	    	if (connect(serverV_fd, (struct sockaddr *) &serverV_add, sizeof(serverV_add)) < 0) {
    			fprintf(stderr,"connect error\n");
    			exit(1);
  	    	}

		 struct pacchettoGreenPass ricezioneClient;
			//lettura della richiesta del client attraverso la FullRead
             		if ((nread = FullRead(conn_fd, &ricezioneClient, sizeof(struct pacchettoGreenPass))) < 0) {
                		printf("Connection interrupted by client.\n");
                		printf("Child process exit...\n\n");
                		close(conn_fd);
                		exit(0);
            		}
		//gestione delle richieste del clientS
         	if(ricezioneClient.ID == 2) {
            		printf("In attesa di dati dal clientS...\n");
			//dichiarazione dei pacchetti applicazione per la ricezione e l'invio di informazioni
			struct pacchettoGreenPass ricezioneServerV, invioClientS;

			//invio della richiesta di clientS al serverV attraverso la FullWrite
         		if ((FullWrite(serverV_fd, &ricezioneClient, sizeof(struct pacchettoGreenPass))) < 0) {
                		fprintf(stderr, "FULLWRITE: error\n");
                		exit(1);
            		}

         		printf("Dati inviati al serverV, in attesa di una sua risposta\n");
			//ricezione del pacchetto di risposta da serverV
         		if ((nread = FullRead(serverV_fd, &ricezioneServerV, sizeof(struct pacchettoGreenPass))) < 0) {
                		printf("Connection interrupted by client.\n");
                		printf("Child process exit...\n\n");
                		close(conn_fd);
                		exit(0);
            		}
		
			//invio del pacchetto di risposta ricevuto a clientS
        		if ((FullWrite(conn_fd, &ricezioneServerV, sizeof(struct pacchettoGreenPass))) < 0) {
                		fprintf(stderr, "FULLWRITE: error\n");
                		exit(1);
            		}
        		printf("Risposta ricevuta ed inoltrata a clientS\n");

		 //gestione delle richieste di clientT
         	} else if(ricezioneClient.ID == 3) {
             		printf("In attesa di dati dal clientT...\n");
			//dichiarazione dei pacchetti applicazione per la ricezione e l'invio di informazioni
			struct pacchettoGreenPass ricezioneServerV, invioClientS;

			//invio della richiesta di clientT al serverV attraverso la FullWrite
         		if ((FullWrite(serverV_fd, &ricezioneClient, sizeof(struct pacchettoGreenPass))) < 0) {
                		fprintf(stderr, "FULLWRITE: error\n");
                		exit(1);
            		}

         		printf("Dati inviati al serverV, in attesa di una sua risposta\n");
			
			//ricezione del pacchetto di risposta da serverV
         		if ((nread = FullRead(serverV_fd, &ricezioneServerV, sizeof(struct pacchettoGreenPass))) < 0) {
                		printf("Connection interrupted by client.\n");
                		printf("Child process exit...\n\n");
                		close(conn_fd);
                		exit(0);
            		}
		
			//invio del pacchetto di risposta ricevuto a clientT
        		if ((FullWrite(conn_fd, &ricezioneServerV, sizeof(struct pacchettoGreenPass))) < 0) {
                		fprintf(stderr, "FULLWRITE: error\n");
                		exit(1);
            		}
        		printf("Risposta ricevuta ed inoltrata a clientT\n");




              }

 		 close (conn_fd);
 		 close(serverV_fd);
 		 exit (0);
	}
	 else
		{ /* operazioni del processo padre*/
		 close (conn_fd);
		 close (serverV_fd);
		}
	}
 /* normal exit , never reached */
 exit (0);

}

ssize_t FullWrite(int fd, const void *buf, size_t count)
{
    size_t nleft;
    ssize_t nwritten;
    nleft = count;
    while (nleft > 0) {             /* repeat until no left */
        if ( (nwritten = write(fd, buf, nleft)) < 0) {
            if (errno == EINTR) {   /* if interrupted by system call */
                continue;           /* repeat the loop */
            } else {
                return(nwritten);   /* otherwise exit with error */
            }
        }
        nleft -= nwritten;          /* set left to write */
        buf +=nwritten;             /* set pointer */
    }
    return (nleft);
}

ssize_t FullRead(int fd, void *buf, size_t count)
{
    size_t nleft;
    ssize_t nread;
    nleft = count;
    while (nleft > 0) {             /* repeat until no left */

	if((nread = read(fd,buf,nleft))<0) {
        if(errno == EINTR) {
            continue;
        } else {
            exit(nread);
        }
        }else if (nread == 0) {
            break;
        }
        nleft -= nread;
        buf += nread;
	}
	buf = 0;
    return (nleft);
}




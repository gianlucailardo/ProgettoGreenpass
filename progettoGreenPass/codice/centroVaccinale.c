#include<sys/types.h> /* predefined types */
#include<unistd.h> /* include unix standard library */
#include<arpa/inet.h> /* IP addresses conversion utililites */
#include<sys/socket.h> /* socket library */
#include<stdio.h> /* include standard I/O library */
#include <stdlib.h>
#include <string.h>
#include <errno.h>	 /* include error codes */

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
 struct sockaddr_in centroVaccinale_add, client, serverV_add;
 char bufferStr[LUNGHEZZACODICE+1];
 socklen_t len;
 pid_t pid;


 //SOCKET
 if ( ( list_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    perror("socket");
    exit(1);
    }

 //configurazione dell'indirizzo del centro vaccinale
 //SIN_FAMILY
 centroVaccinale_add.sin_family      = AF_INET;
 //SIN_ADDR.S_ADDR
 centroVaccinale_add.sin_addr.s_addr = htonl(INADDR_ANY);
 //SIN_PORT
 centroVaccinale_add.sin_port        = htons(1024);

 //BIND
 if ( bind(list_fd, (struct sockaddr *) &centroVaccinale_add, sizeof(centroVaccinale_add)) < 0 ) {
    perror("bind");
    exit(1);
  }

 //configurazione della lista d'attesa dei client
 //LISTEN
 if ( listen(list_fd, 1024) < 0 ) {
    perror("listen");
    exit(1);
  }
	//socket
	
    	   

 while(1)
	{
	 len = sizeof ( client );

	 //accettazione delle richieste dei client
	 //ACCEPT
	 if ( ( conn_fd = accept(list_fd, (struct sockaddr *) &client, &len) ) < 0 ) {
      perror("accept");
      exit(1);
         }

	 printf("client connesso\n");
	 /* fork del processo per generare un processo figlio che gestirà le richieste dei client */
	 if((pid= fork())<0)
		{
		 perror (" fork error ");
		 exit ( -1);
		}
	 if(pid==0)
		{ /* operazioni eseguite nel processo figlio */
		close (list_fd);

		 /* inizializzazione dell'indirizzo del serverV*/
		if ( ( serverV_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    			perror("socket");
    			exit(1);
    	    	}

    	    	//SIN_FAMILY
    	    	serverV_add.sin_family = AF_INET;
	    	//SIN_PORT
    	    	serverV_add.sin_port   = htons(1025);
		

	    	
    	    	//INET_PTON
	    	if (inet_pton(AF_INET, argv[1], &serverV_add.sin_addr) < 0) {
      			fprintf(stderr,"inet_pton error for %s\n", argv[1]);
      			exit (1);
    	    	}

    	    	/* si stabilisce una connessione col serverV */
    	    	//CONNECT
		 if (connect(serverV_fd, (struct sockaddr *) &serverV_add, sizeof(serverV_add)) < 0) {
    		 	fprintf(stderr,"connect error\n");
    		 	exit(1);
  	    	 }
		 
		 printf("In attesa di una richiesta del client...\n");

		 //lettura della richiesta del client attraverso la FullRead
		 if ((nread = FullRead(conn_fd, (void *) bufferStr, (size_t) sizeof(bufferStr))) < 0) {

                	printf("Connection interrupted by client.\n");
                	printf("Child process exit...\n\n");
                	close(conn_fd);
                	exit(0);
            	}
	    //dichiarazione del pacchetto applicazione da inviare al serverV
            struct pacchettoGreenPass invioServerV;
	    invioServerV.ID = 1;
            strcpy(invioServerV.codiceFiscale, (char *) bufferStr);
            invioServerV.mesiGreenPass = 6;
            invioServerV.validita = 1;
           
            //invio della richiesta del clientUtente al serverV attraverso la FullWrite
            if ((FullWrite(serverV_fd, &invioServerV, sizeof(struct pacchettoGreenPass))) < 0) {
                fprintf(stderr, "FULLWRITE: error\n");
                exit(1);
            }

            printf("Dati inviati al serverV\n");



 		 close (conn_fd);
		 close (serverV_fd);
 		 exit (0);
	}
	 else
		{ /* operazioni del processo padre */
		 close (conn_fd);
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

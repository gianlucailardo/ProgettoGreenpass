#include <sys/types.h>   /* predefined types */
#include <unistd.h>      /* include unix standard library */
#include <arpa/inet.h>   /* IP addresses conversion utiliites */
#include <sys/socket.h>  /* socket library */
#include <stdio.h>	 /* include standard I/O library */
#include <errno.h>	 /* include error codes */
#include <string.h>	 /* include erroro strings definitions */
#include <stdlib.h>

#define LUNGHEZZACODICE 15
//definizione del pacchetto applicazione
struct pacchettoGreenPass {
    char codiceFiscale[LUNGHEZZACODICE+1];
    int mesiGreenPass;
    int validita;
    int ID;
};

void VerificaGreenPass(FILE * filein, int socket);
ssize_t FullWrite(int fd, const void *buf, size_t count);
ssize_t FullRead(int fd, void *buf, size_t count);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serverG_add, clientT_add;

    /*SOCKET*/
    if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"socket error\n");
    exit (1);
  }

  
	/* inizializzazione dell'indirizzo di serverG */
    //SIN_FAMILY
    serverG_add.sin_family = AF_INET;
    //SIN_PORT
    serverG_add.sin_port   = htons(1026);


    //INET_PTON
	if (inet_pton(AF_INET, argv[1], &serverG_add.sin_addr) < 0) {
      		fprintf(stderr,"inet_pton error for %s\n", argv[1]);
      		exit (1);
    	}

    /* si stabilisce una connessione col serverG */
    //CONNECT
    if (connect(sock, (struct sockaddr *) &serverG_add, sizeof(serverG_add)) < 0) {
    fprintf(stderr,"connect error\n");
    exit(1);
  }
    /* lancio della funzione dove verranno eseguite le operazioni di lettura e scrittura */
    VerificaGreenPass(stdin, sock);
    /* normal exit */
    return 0;
}

void VerificaGreenPass(FILE * filein, int socket)
{
	//dichiarazione dei pacchetti applicazione per l'invio e la ricezione dei dati
    	struct pacchettoGreenPass invioServerG, ricezioneServerG;
	invioServerG.ID = 3;
    	int nread;	
        fflush(stdin);
        printf("Per effettuare il cambio di validita' del tuo green pass, immetti il tuo codice fiscale: \n");
	//copia del codice fiscale da linea di comando
        strcpy(invioServerG.codiceFiscale, "");
	    if (fgets(invioServerG.codiceFiscale, LUNGHEZZACODICE+1, filein) == NULL) { /* se non ci sono input si 												ferma il client */
		close(socket);
		return;             
	    } else {               
                 //invio del pacchetto al serverG attraverso la fullwrite    
            	if ((FullWrite(socket, &invioServerG, sizeof(struct pacchettoGreenPass))) < 0) {
                	fprintf(stderr, "FULLWRITE: error\n");
                	exit(1);
            	}

	    	printf("dati inviti al serverG, in attesa di risposta\n");

		//ricezione del pacchetto da serverG attraverso la fullread
	    	if ((nread = FullRead(socket, &ricezioneServerG, sizeof(struct pacchettoGreenPass))) < 0) {
                		printf("Connection interrupted by client.\n");
                		printf("Child process exit...\n\n");
                		close(socket);
                		exit(0);
            		}
	   	printf("I dati ricevuti dal serverG sono i seguenti:\n");
	  	
		//verifica dell'esito dell'operazione richiesta (cambio di validità)
	   	if(ricezioneServerG.validita == 1)
	   		printf("il tuo greenpass è stato riattivato\n");
	  	 else if(ricezioneServerG.validita == 0)
			printf("il tuo greenpass è stato disabilitato\n");	

		
		
	    }

        



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



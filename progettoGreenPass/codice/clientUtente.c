#include <sys/types.h>   /* predefined types */
#include <unistd.h>      /* include unix standard library */
#include <arpa/inet.h>   /* IP addresses conversion utiliites */
#include <sys/socket.h>  /* socket library */
#include <stdio.h>	 /* include standard I/O library */
#include <errno.h>	 /* include error codes */
#include <string.h>	 /* include erroro strings definitions */
#include <stdlib.h>

#define LUNGHEZZACODICE 15

void InvioCodice(FILE * filein, int socket);
ssize_t FullWrite(int fd, const void *buf, size_t count);
ssize_t FullRead(int fd, void *buf, size_t count);

int main(int argc, char *argv[])
{
    int sock, vaccinato;
    struct sockaddr_in centroVaccinale_add;

    /*SOCKET*/
    if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"socket error\n");
    exit (1);
  }
	/* inizializzazione indirizzo del centro vaccinale */
    //SIN_FAMILY
    centroVaccinale_add.sin_family = AF_INET;
	//SIN_PORT
    centroVaccinale_add.sin_port   = htons(1024);

	
    //INET_PTON
	if (inet_pton(AF_INET, argv[1], &centroVaccinale_add.sin_addr) < 0) {
      fprintf(stderr,"inet_pton error for %s\n", argv[1]);
      exit (1);
    }

    /* si stabilisce una connessione col centro vaccinale */
    //CONNECT
    if (connect(sock, (struct sockaddr *) &centroVaccinale_add, sizeof(centroVaccinale_add)) < 0) {
    fprintf(stderr,"connect error\n");
    exit(1);
  }
    //la variabile vaccinato viene messa a 1 come indicazione che l'utente è stato vaccinato e può proseguire
    vaccinato = 1;

    /* la funzione che viene lanciata al suo interno avrà le operazioni di lettura e scrittura */
    InvioCodice(stdin, sock);
    /* normal exit */
    return 0;
}

void InvioCodice(FILE * filein, int socket)
{
    char sendbuff[LUNGHEZZACODICE+1];
        fflush(stdin);
        printf("immetti il codice fiscale: \n");
	//codice fiscale preso da linea di comando
	strcpy(sendbuff, "");	
	    if (fgets(sendbuff, LUNGHEZZACODICE+1, filein) == NULL) { /* se non ci sono input si ferma il client */
		close(socket);
		return;               
	    } else {                   
            //fullwrite per inviare il codice fiscale al centroVaccinale
            if ((FullWrite(socket, (void *) sendbuff, (size_t) strlen(sendbuff))) < 0) {
                fprintf(stderr, "FULLWRITE: error\n");
                exit(1);
            }
	 
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

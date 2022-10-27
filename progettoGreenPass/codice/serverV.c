#include<sys/types.h> /* predefined types */
#include<unistd.h> /* include unix standard library */
#include<arpa/inet.h> /* IP addresses conversion utililites */
#include<sys/socket.h> /* socket library */
#include<stdio.h> /* include standard I/O library */
#include <stdlib.h>
#include <string.h>
#include <errno.h>	 /* include error codes */
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <my_global.h>
#include <mysql.h>


#define LUNGHEZZACODICE 15


ssize_t FullWrite(int fd, const void *buf, size_t count);
ssize_t FullRead(int fd, void *buf, size_t count);
void *gestioneClient(void * arg);

//definizione del pacchetto applicazione
struct pacchettoGreenPass {
    char codiceFiscale[LUNGHEZZACODICE+1];
    int mesiGreenPass;
    int validita;
    int ID;
};

 pthread_mutex_t lock;

int main (int argc , char *argv[])
{
 int list_fd,conn_fd;
 struct sockaddr_in serv_add,client;
 socklen_t len;
 pthread_t thread;


 //configurazione dell'indirizzo di serverV
 //SOCKET
 if ( ( list_fd = socket(AF_INET, SOCK_STREAM, 0) ) < 0 ) {
    perror("socket");
    exit(1);
    }
 //SIN_FAMILY
 serv_add.sin_family      = AF_INET;
 //SIN_ADDR.S_ADDR
 serv_add.sin_addr.s_addr = htonl(INADDR_ANY);
 //SIN_PORT
 serv_add.sin_port        = htons(1025);

 //BIND
 if ( bind(list_fd, (struct sockaddr *) &serv_add, sizeof(serv_add)) < 0 ) {
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
	 
	 /* generazione di un nuovo trhead che andrà a gestire le richieste dei client */
	 int * desc_to_thread = (int *) malloc(sizeof(int));
	 desc_to_thread = &conn_fd;
	 if(pthread_create(&thread, NULL, gestioneClient, (void *) desc_to_thread)!=0)
		{
		 perror (" pthread_create errorr ");
		 exit ( -1);
		}
         
		 /* parent */
		 pthread_join(thread, NULL);
		 close (conn_fd);
		
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


void *gestioneClient(void * arg) {
			int * filedes = (int *) arg;
			int nread;
		 	struct pacchettoGreenPass ricezioneClient;
			//ricezione della richiesta del client attraverso la FullRead
             		if ((nread = FullRead(*filedes, (void *) &ricezioneClient, sizeof(struct pacchettoGreenPass))) < 0) {
                		printf("Connection interrupted by client.\n");
                		printf("Child process exit...\n\n");
                		close(*filedes);
                		exit(0);
            		}

		//gestione della richiesta del centro vaccinale (arrivata da clientUtente)
         	if(ricezioneClient.ID == 1) {
            		printf("dati ricevuti dal centro vaccinale...\n");			
  			char sql_statement[2048];
			MYSQL *con;
  			con = mysql_init(NULL);
			MYSQL_ROW row;
  			MYSQL_RES *result;
  			if (con == NULL)
  			{
      				fprintf(stderr, "%s\n", mysql_error(con));
     				 exit(1);
  			}
			//da questo punto le operazioni sono eseguite in mutua esclusione
			pthread_mutex_lock(&lock);
			//connessione al database
  			if (mysql_real_connect(con, "localhost", "root", "parthenope",
          		    "greenPassDB", 0, NULL, 0) == NULL)
  			{
      				fprintf(stderr, "%s\n", mysql_error(con));
      				mysql_close(con);
      				exit(1);
  			}  
			//query per registrare il green pass del nuovo utente nel database
  			sprintf(sql_statement, "INSERT INTO USER(ID, CODICEFISCALE, MESI, VALIDITA) VALUES('%d', '%s', '%d', '%d')", ricezioneClient.ID, ricezioneClient.codiceFiscale, ricezioneClient.mesiGreenPass, 																	     ricezioneClient.validita);
  			if(mysql_query(con, sql_statement) !=0)
  			{
    				printf("Query failed  with the following message:\n");
    				printf("%s\n", mysql_error(con));
    				exit(1);
  			}
			//query per visualizzare i dati inseriti
  			sprintf(sql_statement, "SELECT CODICEFISCALE, MESI FROM USER WHERE CODICEFISCALE = '%s'", ricezioneClient.codiceFiscale);
  			if(mysql_query(con, sql_statement) !=0)
  			{
    				printf("Query failed  with the following message:\n");
    				printf("%s\n", mysql_error(con));
    				exit(1);
  			}

  			result = mysql_store_result(con);

  			printf("numero di occorrenze nel DB: %ld\n", (long) mysql_num_rows(result));
			//scorrimento dei dati nel database
  			while((row = mysql_fetch_row(result)) != NULL)
  			{
    				printf("codice fiscale salvato: %s\n", row[0]);
    				printf("durata del green pass: %s mesi\n", row[1]);
    				printf("\n");
  			}
  			
  			mysql_close(con);
			pthread_mutex_unlock(&lock);


		  //gestione della richiesta di serverG (arrivata da clientS)
		} else if(ricezioneClient.ID == 2) {
             		printf("In attesa di dati dal serverG...\n");
			//dichiarazione del pacchetto applicazione da inviare al serverG
			struct pacchettoGreenPass invioServerG;

			char sql_statement[2048];
			MYSQL *con;
  			con = mysql_init(NULL);
			MYSQL_ROW row;
  			MYSQL_RES *result;
  			if (con == NULL)
  			{
      				fprintf(stderr, "%s\n", mysql_error(con));
     				 exit(1);
  			}
			//da questo punto le operazioni sono eseguite in mutua esclusione
			pthread_mutex_lock(&lock);

			//connessione al database
  			if (mysql_real_connect(con, "localhost", "root", "parthenope",
          		    "greenPassDB", 0, NULL, 0) == NULL)
  			{
      				fprintf(stderr, "%s\n", mysql_error(con));
      				mysql_close(con);
      				exit(1);
  			}  

			//query per visualizzare i dati inseriti
			sprintf(sql_statement, "SELECT CODICEFISCALE, VALIDITA, MESI FROM USER WHERE CODICEFISCALE = '%s'", ricezioneClient.codiceFiscale);
  			if(mysql_query(con, sql_statement) !=0)
  			{
    				printf("Query failed  with the following message:\n");
    				printf("%s\n", mysql_error(con));
    				exit(1);
  			}

  			result = mysql_store_result(con);

  			printf("Numero di occorrenze nel DB: %ld\n", (long) mysql_num_rows(result));
			if ((long) mysql_num_rows(result) != 0)
			{
				//scorrimento dei dati nel database
				while((row = mysql_fetch_row(result)) != NULL)
  				{
    					strcpy(invioServerG.codiceFiscale, row[0]);
					invioServerG.validita = atoi(row[1]);
					invioServerG.mesiGreenPass = atoi(row[2]);
					printf("verifica della validità del green pass con codice fiscale: %s\n", invioServerG.codiceFiscale);
  				}
				//invio del pacchetto applicazione di risposta al serverG
				if ((FullWrite(*filedes, (void *) &invioServerG, sizeof(struct pacchettoGreenPass))) < 0) {
                				fprintf(stderr, "FULLWRITE: error\n");
                				exit(1);
            	    			}
		    		printf("dati inviati al serverG\n");
			}
			else
			{
				printf("green pass non presente nel database\n");
			}
               
           		

			pthread_mutex_unlock(&lock);

		   //gestione della richiesta di serverG (arrivata da clientT)
		} else if(ricezioneClient.ID == 3) {
			printf("In attesa di dati dal serverG...\n");
			//dichiarazione del pacchetto applicazione da inviare al serverG
			struct pacchettoGreenPass invioServerG;

			char sql_statement[2048];
			MYSQL *con;
  			con = mysql_init(NULL);
			MYSQL_ROW row;
  			MYSQL_RES *result;
  			if (con == NULL)
  			{
      				fprintf(stderr, "%s\n", mysql_error(con));
     				 exit(1);
  			}
			//da questo punto le operazioni sono eseguite in mutua esclusione
			pthread_mutex_lock(&lock);

			//connessione al database
  			if (mysql_real_connect(con, "localhost", "root", "parthenope",
          		    "greenPassDB", 0, NULL, 0) == NULL)
  			{
      				fprintf(stderr, "%s\n", mysql_error(con));
      				mysql_close(con);
      				exit(1);
  			}  
			
			//query per visualizzare i dati inseriti 
			sprintf(sql_statement, "SELECT VALIDITA FROM USER WHERE CODICEFISCALE = '%s'", ricezioneClient.codiceFiscale);
  			if(mysql_query(con, sql_statement) !=0)
  			{
    				printf("Query failed  with the following message:\n");
    				printf("%s\n", mysql_error(con));
    				exit(1);
  			}

  			result = mysql_store_result(con);

  			printf("Numero di occorrenze nel DB: %ld\n", (long) mysql_num_rows(result));
			if ((long) mysql_num_rows(result) != 0)
			{
				//scorrimento dei dati presenti del		
				while((row = mysql_fetch_row(result)) != NULL)
  				{
    					//nella variabile del pacchetto applicazione da rispendire verrà salvata la validità "invertita" data la richiesta del clientT
					if(atoi(row[0]) == 0)
						invioServerG.validita = 1;
					else if(atoi(row[0]) == 1)
						invioServerG.validita = 0;
					printf("validita del green pass con codice fiscale %s aggiornata\n", invioServerG.codiceFiscale);
					
  				}
				
				//invio del pacchetto di risposta a serverG
				if ((FullWrite(*filedes, (void *) &invioServerG, sizeof(struct pacchettoGreenPass))) < 0) {
                				fprintf(stderr, "FULLWRITE: error\n");
                				exit(1);
            	    			}
		    		printf("dati inviati al serverG\n");

				//update effettivo della validità del green pass all'interno del database
				if(invioServerG.validita == 0)
				{
					sprintf(sql_statement, "UPDATE USER SET VALIDITA = 0 WHERE CODICEFISCALE = '%s'", ricezioneClient.codiceFiscale);
					mysql_query(con,sql_statement);
				}
				else if(invioServerG.validita == 1)
				{
					sprintf(sql_statement, "UPDATE USER SET VALIDITA = 1 WHERE CODICEFISCALE = '%s'", ricezioneClient.codiceFiscale);
					mysql_query(con,sql_statement);
				}
			}
			else
			{
				printf("green pass non presente nel database\n");
			}
               
           		

			pthread_mutex_unlock(&lock);

		}
               
           		

	close (*filedes);



}


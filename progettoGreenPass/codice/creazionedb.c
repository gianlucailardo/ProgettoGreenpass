#include <my_global.h>
#include <mysql.h>
MYSQL *con;
int main(int argc, char **argv)
{
  //puntatore a mysql
  con = mysql_init(NULL);
  if (con == NULL)
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      exit(1);
  }

  //connessione a mysql
  if (mysql_real_connect(con, "localhost", "root", "parthenope",
          NULL, 0, NULL, 0) == NULL)
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }  

  //creazione del database
  if (mysql_query(con, "CREATE DATABASE greenPassDB"))
  {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);
  }
  mysql_close(con);
  exit(0);
}


/* la creazione della tabella USER del datase che verrà usata negli altri
   file C è stata fatta da riga di comando con la seguente istruzione:

   CREATE TABLE USER(
  ID               INTEGER,
  CODICEFISCALE    VARCHAR(16) PRIMARY KEY,
  MESI             INTEGER NOT NULL,
  VALIDITA         INTEGER NOT NULL
);									*/

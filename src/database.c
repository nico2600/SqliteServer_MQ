/**
 * SqliteMQ database access functions.
 */
#include "sqlitemq.h"

//callback called - contain result of sqlite query
static int callback(void *sock, int argc, char **argv, char **azColName){
   int i;

   for(i=0; (i<argc); i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   return EXIT_SUCCESS;
}



int database_open(const char *path, sqlitemq_t *sqlitemq) {
   int rc;
   sqlite3_initialize();

   rc = sqlite3_open(path, &(sqlitemq->db));
   if( rc ){
      fprintf(stderr,"Can't open database: %s\n", sqlite3_errmsg(sqlitemq->db));
      sqlite3_close(sqlitemq->db);
      return(EXIT_FAILURE);
   }

   return(EXIT_SUCCESS);
}

// Close a database and free its structure.
void database_close(sqlite3 *env) {
	sqlite3_close(env);
   sqlite3_shutdown();
}

// Add or update a key in database.
void database_query(sqlite3 *db, const char *query) {
   int rc;
   char *zErrMsg = 0;

   printf("Executing %s\n",query);

   rc = sqlite3_exec(db, query, callback, NULL, &zErrMsg);
   if( rc!=SQLITE_OK ){
      fprintf(stderr,"[SERVER] SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }

}

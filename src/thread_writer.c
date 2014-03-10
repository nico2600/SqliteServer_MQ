/**
 *  SqliteMQ writer thread.
 */
#include "sqlitemq.h"

// Writer thread's execution loop.
void *thread_writer_loop(void *param) {
   sqlitemq_t *sqlitemq = param;
   int socket;

   // create the nanomsg socket for threads communication
   socket = nn_socket(AF_SP, NN_PULL);
   nn_bind(socket, ENDPOINT_WRITER_SOCKET);
   // loop to process new connections
   for (; ; ) {
      writer_msg_t *msg;

      // waiting for a new connection to handle
      if (nn_recv(socket, &msg, sizeof(writer_msg_t*), 0) < 0)
         continue;
      // processing
      switch (msg->type) {
         case WRITE_QUERY:
            database_query(sqlitemq->db, msg->query.data);
            break;
      }
      // free data
      if (msg->query.data)
         free(msg->query.data);
      free(msg);
   }
   return (NULL);
}


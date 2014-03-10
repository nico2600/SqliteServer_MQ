/**
 * SqliteMQ server.
 */
#include "sqlitemq.h"

/* *** Prototypes of private functions. *** */
static int _create_listening_socket(unsigned short port);
static void _main_thread_loop(sqlitemq_t *sqlitemq);

/**
 * Main function of the program.
 */
int main() {
	sqlitemq_t *sqlitemq;
	int i;

	// server init
	sqlitemq = calloc(1, sizeof(sqlitemq_t));
	sqlitemq->socket = sqlitemq->threads_socket = -1;
	sqlitemq->comm_threads = calloc(NBR_THREADS, sizeof(comm_thread_t));
	// open the database
	i =database_open("test", sqlitemq);
	// create the nanomsg socket for threads communication
	sqlitemq->threads_socket = nn_socket(AF_SP, NN_PUSH);
	nn_bind(sqlitemq->threads_socket, ENDPOINT_THREADS_SOCKET);
	// create the writer thread
	pthread_create(&sqlitemq->writer_tid, NULL, thread_writer_loop, sqlitemq);
	// create communication threads
	for (i = 0; i < NBR_THREADS; i++) {
		comm_thread_t *thread = &(sqlitemq->comm_threads[i]);

		thread->client_sock = -1;
		thread->sqlitemq = sqlitemq;
		pthread_create(&thread->tid, 0, thread_comm_loop, thread);
		pthread_detach(thread->tid);
	}
	// create listening socket
	sqlitemq->socket = _create_listening_socket(DEFAULT_PORT);
	// server loop
	_main_thread_loop(sqlitemq);
	return (0);
}

/* ************** PRIVATE FUNCTIONS **************** */
/**
 * @function	_main_thread_loop
 *		Main thread's execution loop. Wait for new incoming connections
 *		and send them to the communication threads.
 * @param	sqlitemq	Pointer to the server's structure.
 */
static void _main_thread_loop(sqlitemq_t *sqlitemq) {
	int fd;
	struct sockaddr_in addr;
	unsigned int addr_size;
	const int on = 1;

	addr_size = sizeof(addr);
	for (; ; ) {
		bzero(&addr, addr_size);
		// accept a new connection
		if ((fd = accept(sqlitemq->socket, (struct sockaddr*)&addr,
				 &addr_size)) < 0)
			continue ;
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&on, sizeof(on));
		// send the file descriptor number to communication threads
		nn_send(sqlitemq->threads_socket, &fd, sizeof(fd), 0);
	}
}

/**
 * @function	_create_listening_socket
 *		Create a socket that will listening for incoming connections.
 * @param	port	Port number.
 * @return	The created socket.
 */
static int _create_listening_socket(unsigned short port) {
	int sock;
	struct sockaddr_in addr;
	unsigned int addr_size;
	const int on = 1;

	// create the socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	// some options
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on));
	setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (void*)&on, sizeof(on));
	// binding to any interface
	addr_size = sizeof(addr);
	bzero(&addr, addr_size);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	bind(sock, (struct sockaddr*)&addr, addr_size);
	listen(sock, SOMAXCONN);
	return (sock);
}

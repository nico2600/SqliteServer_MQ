/**
 * @header	sqlitemq.h
 *
 * Definitions used in the SqliteMQ server.
 */
#ifndef __SQLITEMQ_H__
#define __SQLITEMQ_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <pthread.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "nanomsg/nn.h"
#include "nanomsg/pipeline.h"
//include sqlite3
#include <sqlite3.h>

/* ********** DEFAULT CONFIGURATION VALUES ************ */
/** @const NBR_THREADS Number of communication threads. */
#define NBR_THREADS		20
/** @const DEFAULT_PORT Default port number. */
#define DEFAULT_PORT		11139
/** @const DEFAULT_MAPSIZE Default map size (10 MB). */
#define DEFAULT_MAPSIZE		10485760

/* ********** NANOMSG ENDPOINTS CONFIGURATION *********** */
/** @const ENDPOINT_THREADS_SOCKET Communication threads' connection endpoint. */
#define ENDPOINT_THREADS_SOCKET	"inproc://threads_socket"
/** @const ENDPOINT_WRITER_SOCKET Writer thread's connection endpoint. */
#define ENDPOINT_WRITER_SOCKET	"inproc://writer_socket"

/* ********** PROTOCOL ************ */
/** @const PROTO_PUT PUT command. */
#define PROTO_QUERY	1
/** @const PROTO_OK OK response. */
#define PROTO_OK	1
/** @const PROTO_ERR Error response. */
#define PROTO_ERR	0

/* ********** STRUCTURES ************ */
/**
 * @typedef	sqlitemq_t
 * 		Main structure of the SqliteMQ server.
 * @field	db		Pointer to the database environment.
 * @field	socket		Socket descriptor for incoming connections.
 * @field	threads_socket	Nanomsg socket for threads communication.
 * @field	writer_tid	ID of the writer thread.
 * @field	comm_threads	Array of communication threads.
 */
typedef struct sqlitemq_s {
	sqlite3 *db;
	int socket;
	int threads_socket;
	pthread_t writer_tid;
	struct comm_thread_s *comm_threads;
} sqlitemq_t;

/**
 * @typedef	comm_thread_t
 *		Structure for communication threads.
 * @field	tid		Thread's identifier.
 * @field	sqlitemq	Pointer to the server's structure.
 * @field	client_sock	Socket used to communicate with the client.
 * @field	writer_sock	Nanomsg socket to send data to the writer thread.
 */
typedef struct comm_thread_s {
	pthread_t tid;
	sqlitemq_t *sqlitemq;
	int client_sock;
	int writer_sock;
} comm_thread_t;

typedef struct query_val_s {
      size_t       size;   /**< size of the data item */
      char    *data;   /**< address of the data item */
} query_val_t;

/**
 * @typedef	writer_msg_t
 *		Structure used to transfer data to the writer thread.
 * @field	type		Type of action (WRITE_PUT, WRITE_DEL).
 * @field	key		Size and content of the key.
 * @field	value		Size and content of the value.
 */
typedef struct writer_msg_s {
	enum {
		WRITE_QUERY
	} type;
	query_val_t query;
} writer_msg_t;

/* ************** FUNCTIONS *********** */
/* --- writer thread --- */
/**
 * @function	thread_writer_loop
 *		Writer thread's execution loop.
 * @param	param	Pointer to the server's structure.
 * @return	Always NULL.
 */
void *thread_writer_loop(void *param);

/* --- communication threads --- */
/**
 * @function	thread_comm_loop
 *		Communication threads' execution loop.
 * @param	param	Pointer to the thread's structure.
 * @return	Always NULL.
 */
void *thread_comm_loop(void *param);

/* --- commands --- */
/**
 * @function	command_put
 *		QUERY command execution.
 * @param	thread	Pointer to the current thread structure.
 */
void command_query(comm_thread_t *thread);

/**
 * @function	database_query
 *		Add or update a key in database.
 * @param	db	Pointer to the database environment.
 * @param	query	content of the query.
 */
void database_query(sqlite3 *db, const char *query);

/**
 * @function database_open
 *    Open a Sqlite3 database
 *
 * @param   path  Path to the database's data directory.
 * @param   pointer to the general struct
 */
int database_open(const char *path,sqlitemq_t *sqlitemq);

/**
 * Close a database and free its structure.
 * @param  env   A pointer to the database environment.
 */
void database_close(sqlite3 *env);

#endif /* __SQLITEMQ_H__ */

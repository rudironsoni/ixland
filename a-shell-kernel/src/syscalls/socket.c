/*
 * socket.c - Socket syscalls
 *
 * All socket operations pass through to native BSD sockets
 * BSD sockets are POSIX-compatible
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "../../include/linux/socket.h"

/*
 * Note: All socket functions are defined as macros in linux/socket.h
 * that directly map to native BSD socket functions.
 * 
 * No implementation needed here - the macros in the header do the work.
 * 
 * This file exists for:
 * 1. Documentation purposes
 * 2. Future iOS-specific socket handling if needed
 * 3. Link-time verification that all socket functions exist
 */

/* Verify socket functions are available at link time */
typedef int (*socket_func_t)(int, int, int);
typedef int (*bind_func_t)(int, const struct sockaddr *, socklen_t);
typedef int (*connect_func_t)(int, const struct sockaddr *, socklen_t);
typedef int (*listen_func_t)(int, int);
typedef int (*accept_func_t)(int, struct sockaddr *, socklen_t *);
typedef ssize_t (*send_func_t)(int, const void *, size_t, int);
typedef ssize_t (*recv_func_t)(int, void *, size_t, int);

/* Force linkage of socket functions */
static void __attribute__((used)) socket_linkage_verification(void) {
    socket_func_t s = socket;
    bind_func_t b = bind;
    connect_func_t c = connect;
    listen_func_t l = listen;
    accept_func_t a = accept;
    send_func_t se = send;
    recv_func_t r = recv;
    
    (void)s;
    (void)b;
    (void)c;
    (void)l;
    (void)a;
    (void)se;
    (void)r;
}

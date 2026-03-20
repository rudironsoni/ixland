/*
 * linux/socket.h - Socket interface
 *
 * Linux-compatible header for a-Shell kernel
 * Pass-through to BSD sockets
 */

#ifndef _LINUX_SOCKET_H
#define _LINUX_SOCKET_H

/* Pass-through to native BSD sockets - they're POSIX compatible */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Socket Creation
 * ============================================================================ */

#define a_shell_socket      socket
#define a_shell_socketpair  socketpair

/* ============================================================================
 * Socket Binding and Connection
 * ============================================================================ */

#define a_shell_bind        bind
#define a_shell_listen      listen
#define a_shell_accept      accept
#define a_shell_accept4     accept4
#define a_shell_connect     connect
#define a_shell_shutdown    shutdown

/* ============================================================================
 * Socket Names
 * ============================================================================ */

#define a_shell_getsockname getsockname
#define a_shell_getpeername getpeername

/* ============================================================================
 * Socket Options
 * ============================================================================ */

#define a_shell_setsockopt  setsockopt
#define a_shell_getsockopt  getsockopt

/* ============================================================================
 * Data Transmission
 * ============================================================================ */

#define a_shell_send        send
#define a_shell_sendto      sendto
#define a_shell_sendmsg     sendmsg
#define a_shell_recv        recv
#define a_shell_recvfrom    recvfrom
#define a_shell_recvmsg     recvmsg

/* ============================================================================
 * Address Resolution
 * ============================================================================ */

#define a_shell_gethostbyname   gethostbyname
#define a_shell_gethostbyaddr   gethostbyaddr
#define a_shell_getaddrinfo     getaddrinfo
#define a_shell_freeaddrinfo    freeaddrinfo
#define a_shell_getnameinfo     getnameinfo

/* ============================================================================
 * Legacy Compatibility Macros
 * ============================================================================ */

#define socket(domain, type, protocol) \
                                a_shell_socket(domain, type, protocol)
#define socketpair(domain, type, protocol, sv) \
                                a_shell_socketpair(domain, type, protocol, sv)

#define bind(sockfd, addr, addrlen) \
                                a_shell_bind(sockfd, addr, addrlen)
#define listen(sockfd, backlog) \
                                a_shell_listen(sockfd, backlog)
#define accept(sockfd, addr, addrlen) \
                                a_shell_accept(sockfd, addr, addrlen)
#define accept4(sockfd, addr, addrlen, flags) \
                                a_shell_accept4(sockfd, addr, addrlen, flags)
#define connect(sockfd, addr, addrlen) \
                                a_shell_connect(sockfd, addr, addrlen)
#define shutdown(sockfd, how) \
                                a_shell_shutdown(sockfd, how)

#define getsockname(sockfd, addr, addrlen) \
                                a_shell_getsockname(sockfd, addr, addrlen)
#define getpeername(sockfd, addr, addrlen) \
                                a_shell_getpeername(sockfd, addr, addrlen)

#define setsockopt(sockfd, level, optname, optval, optlen) \
                                a_shell_setsockopt(sockfd, level, optname, optval, optlen)
#define getsockopt(sockfd, level, optname, optval, optlen) \
                                a_shell_getsockopt(sockfd, level, optname, optval, optlen)

#define send(sockfd, buf, len, flags) \
                                a_shell_send(sockfd, buf, len, flags)
#define sendto(sockfd, buf, len, flags, dest_addr, addrlen) \
                                a_shell_sendto(sockfd, buf, len, flags, dest_addr, addrlen)
#define sendmsg(sockfd, msg, flags) \
                                a_shell_sendmsg(sockfd, msg, flags)
#define recv(sockfd, buf, len, flags) \
                                a_shell_recv(sockfd, buf, len, flags)
#define recvfrom(sockfd, buf, len, flags, src_addr, addrlen) \
                                a_shell_recvfrom(sockfd, buf, len, flags, src_addr, addrlen)
#define recvmsg(sockfd, msg, flags) \
                                a_shell_recvmsg(sockfd, msg, flags)

#define gethostbyname(name)     a_shell_gethostbyname(name)
#define gethostbyaddr(addr, len, type) \
                                a_shell_gethostbyaddr(addr, len, type)
#define getaddrinfo(node, service, hints, res) \
                                a_shell_getaddrinfo(node, service, hints, res)
#define freeaddrinfo(res)       a_shell_freeaddrinfo(res)
#define getnameinfo(sa, salen, host, hostlen, serv, servlen, flags) \
                                a_shell_getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_SOCKET_H */

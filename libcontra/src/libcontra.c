/* 
  Copyright 2018 Thorsten Alteholz <libcontra@alteholz.eu>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program (see the file COPYING included with this
  distribution); if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*!
*************************************************************************
*
* \file libcontra.c
*
* \author Thorsten Alteholz <libcontra@alteholz.eu>
*
* \todo documentation
*
* \brief library for connection tracking
*
* for more info see documents in docu directory
*
*
* \addtogroup libcontra
* The group of the library.
* @{
************************************************************************/

#ifndef RTLD_NEXT
/*! we need to define this in order to use RTLD_NEXT in dlopen() */
#  define _GNU_SOURCE
#endif
/* XXX maybe not needed
#include <netinet/tcp.h>
#include <netinet/in.h>
*/
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <pthread.h>

void _libcontra_log(int level, const char *fmt, ...)
    __attribute__((format (printf, 2, 3)));

/*! macro for logging function */
#define libcontra_log(level, fmt, ...) _libcontra_log(level, fmt"\n", ##__VA_ARGS__)

#define LOGLEVEL_NULL		0  	/*!< no log */
#define LOGLEVEL_ERROR		1	/*!< only log error messages */
#define LOGLEVEL_WARNING	2	/*!< log everything >= warning */
#define LOGLEVEL_INFO		3	/*!< log everything >= info */
#define LOGLEVEL_DEBUG		4	/*!< log everything */

static pthread_mutex_t libcontra_lock; 
static bool	libcontra_isInitialized=false;
static bool	libcontra_logConnect=false;
static bool	libcontra_logInit=false;
static bool	libcontra_logStdio=false;
static bool	libcontra_configResolve=false;
static int	libcontra_loglevel=LOGLEVEL_NULL;
static char*	libcontra_filename=NULL;
static FILE*	libcontra_file=NULL;



/*!
*************************************************************************
*
* \fn void libcontra_init()
*
* \author Thorsten Alteholz <libcontra@alteholz.eu>
*
* \todo documentation
*
* \bug up to now, there is no know bug
*
* \brief initialize libcontra
*
* This function will be called whenever libcontra is used for the
* first time within a process.
* ENV variables will be evaluated.
*
************************************************************************/
void libcontra_init()
{
  char *env;

  if (pthread_mutex_init(&libcontra_lock, NULL) != 0) {
    printf("\n mutex init failed\n");
    return;
  }

  /* check environment variables */
  env=getenv("CONTRA_CONFIG_RESOLVE");
  if (env!=NULL) {
    if (strstr(env,"yes")!=NULL) libcontra_configResolve=true;
    if (strstr(env,"true")!=NULL) libcontra_configResolve=true;
  }

  env=getenv("CONTRA_LOG_STDIO");
  if (env!=NULL) {
    if (strstr(env,"yes")!=NULL) libcontra_logStdio=true;
    if (strstr(env,"true")!=NULL) libcontra_logStdio=true;
  }

  env=getenv("CONTRA_LOG_LEVEL");
  if (env!=NULL) {
    libcontra_loglevel=atoi(env);
  }

  libcontra_filename=getenv("CONTRA_LOG_FILE");
  if (libcontra_filename != NULL) {
    libcontra_file=fopen(libcontra_filename,"a");
  }

  env=getenv("CONTRA_LOG_FUNCTIONS");
  if (env!=NULL) {
    if (strstr(env,"init")!=NULL)          libcontra_logInit=true;
    if (strstr(env,"connect")!=NULL)       libcontra_logConnect=true;
  }

  /* */
  if (libcontra_logInit==true) {
    if (libcontra_logInit)		libcontra_log(LOGLEVEL_INFO, "D: log infos in init()");
					libcontra_log(LOGLEVEL_INFO, "D: log level",libcontra_loglevel);
    if (libcontra_logStdio)		libcontra_log(LOGLEVEL_INFO, "D: log to stdio");
    if (libcontra_file!=NULL)		libcontra_log(LOGLEVEL_INFO, "D: log to FILE");
    if (libcontra_logConnect)		libcontra_log(LOGLEVEL_INFO, "D: log infos in connect()");
    if (libcontra_configResolve)	libcontra_log(LOGLEVEL_INFO, "D: resolve IP addresses");
  }
  /* */
  libcontra_isInitialized=true;
}

/*!
*************************************************************************
*
* \fn void _libcontra_log(int level, const char *fmt, ...)
*
* \author Thorsten Alteholz <libcontra@alteholz.eu>
*
* \todo documentation
*
* \bug up to now, there is no know bug
*
* \brief write stuff to logfile
*
* write stuff to log
*
************************************************************************/
void _libcontra_log(int level, const char *fmt, ...)
{
  va_list arg;
//  FILE *log_file = (level == LOG_ERROR) ? err_log : info_log;

  pthread_mutex_lock(&libcontra_lock);

  /* Check if the message should be logged */
  if (level > libcontra_loglevel)
     return;

  /* Write the error message */
  if (libcontra_logStdio==true) {
    va_start(arg, fmt);
    vfprintf(stdout, fmt, arg);
    va_end(arg);
  }
  if (libcontra_file!=NULL) {
    va_start(arg, fmt);
    vfprintf(libcontra_file, fmt, arg);
    va_end(arg);
    #ifdef DEBUG
     fflush(libcontra_file);
     fsync(fileno(libcontra_file));
    #endif
  }

  pthread_mutex_unlock(&libcontra_lock);
}

/*!
*************************************************************************
*
* \fn int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
*
* \author Thorsten Alteholz <libcontra@alteholz.eu>
*
* \todo documentation
*
* \bug up to now, there is no know bug
*
* \brief replacement of libc::connect()
*
* connect XXX
*
************************************************************************/
int connect(int sockfd, const struct sockaddr *addr,
                   socklen_t addrlen)
{
  int (*libc_connect)(int, const struct sockaddr *, socklen_t);
  int rc;
  int port=0;
  char output[NI_MAXHOST+256];

  if (libcontra_logConnect) libcontra_log(LOGLEVEL_DEBUG, "D: libcontra connect start");

  if (libcontra_isInitialized==false) {
    libcontra_init();
  } 

  memset(output,NI_MAXHOST+256,0);
  sprintf(output,"E: unknown SA_FAMILY: %i",addr->sa_family);
  if (addr->sa_family==AF_INET) {
    /* IPv4 */
    if (libcontra_logConnect) libcontra_log(LOGLEVEL_DEBUG, "D: libcontra connect IPv4");
    struct sockaddr_in *ipv4;
    char addrbuf[INET6_ADDRSTRLEN];

    ipv4=(struct sockaddr_in *)addr;
    inet_ntop(ipv4->sin_family, &ipv4->sin_addr, addrbuf,INET6_ADDRSTRLEN);
    port=htons(ipv4->sin_port);
   
    if (libcontra_configResolve==true) { 
      if (libcontra_logConnect) libcontra_log(LOGLEVEL_DEBUG, "D: libcontra connect IPv4 resolve: %i",port);
      /* DNS will start another request, so do nothin */
      if (port!=53) {
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

        memset(hbuf,0,sizeof(hbuf));
        memset(sbuf,0,sizeof(sbuf));

        if (getnameinfo(addr, addrlen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NAMEREQD)) {
          /* could not resolv hostname, deny connection */
          errno = EACCES;
          return -1;
        }
        sprintf(output,"I: IPv4 connect to %s:%s",hbuf,sbuf);
      }
      else {
        sprintf(output,"I: IPv4 DNS connect to %s:%i",addrbuf,port);
      }
    }
    else {
      if (libcontra_logConnect) libcontra_log(LOGLEVEL_DEBUG, "D: libcontra connect IPv4 without resolve: %i",port);
      if (port!=53) {
        sprintf(output,"I: IPv4 connect to %s:%i",addrbuf,port);
      }
      else {
        sprintf(output,"I: IPv4 DNS connect to %s:%i",addrbuf,port);
      }
    }
  }
  else {
    if (libcontra_logConnect) libcontra_log(LOGLEVEL_DEBUG, "D: libcontra connect NO IPv4");
  }

  *(void **)(&libc_connect) = dlsym(RTLD_NEXT, "connect");
  if(dlerror()) {
    errno = EACCES;
    return -1;
  }

  rc = (*libc_connect)(sockfd, addr, addrlen);

  if (libcontra_logConnect) libcontra_log(LOGLEVEL_NULL, "LOG libcontra connect: %s -> %x",output, rc);

  if (libcontra_logConnect) libcontra_log(LOGLEVEL_DEBUG, "D: libcontra connect end");

  return rc;
}

/*! @} */


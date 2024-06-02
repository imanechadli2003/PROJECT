#ifndef GLOBAL_H_
#define GLOBAL_H_
/*Bibliothéque nécessaires*/
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>

extern int deconnexion; /*variable pour tracker la deconnexion du client */
int port; /*le port où on se trouve*/
char * ip; /*adresse IP */
long dS; /*socket du client */


#endif
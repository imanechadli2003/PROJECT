/*FICHIER POUR les variables globaux et les bibliotéques */
#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdarg.h>


#define MAX_CLIENT 15 
#define NB_ROOMS 5 
#define SIZE_MSG sizeof(char)*500 

/*definition d'une structure client*/
typedef struct Client Client;
struct Client{
    int connected; 
    int id;
    char pseudo[15];
    char password[12];
    char descr[300];
    long dSC; 
    int idRoom; /*id du salon */
    int created; /*Permet de savoir si un client occupe la place dans le tableau de clients présents */
    int isAdmin; /*bool pour savoir si le client est un admin du serveur*/
};

typedef struct Room Room;
struct Room{
    int id;
    char * name;
    char * descr;
    int members[MAX_CLIENT]; /* a chaque indice de ce tableau,on pourrait savoir est ce que le client est present ou pas dans ce salon */ 
    int admin[MAX_CLIENT]; /* à chaque indice on peut savoir si le client numéro i est admin de ce salon*/ 
    int ban[MAX_CLIENT]; /*à chaque indice on peut  savoir si le client numéro i est ban du salon*/ 
    int created; /*il y a des salons (default) qui ne sont pas crées,du coup si 0 il n'existe pas,1 il existe ce salon*/
};


 Client tabClient[MAX_CLIENT];
 Room rooms[NB_ROOMS];
 pthread_t tabThread[MAX_CLIENT];
 sem_t semNbClient;
 pthread_t tabThreadToKill[MAX_CLIENT*2];
int nbThreadToKill;

extern pthread_mutex_t lock;

 long dSFile; 
 int dS; 
char * arg1;

#endif
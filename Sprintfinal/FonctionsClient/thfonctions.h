/*fichiers pour les threads de communication du client */
#ifndef THFUNCTIONS_H_
#define THFUNCTIONS_H_
#include "global.h"
#include "traitmessage.h"
#include "commfonctions.h"

/* envoi du fichier dans le thread (du client au serveur)
  param : void * fileNameParam : nom du fichier 
 * */
void * sendfile_th(void * fileNameParam);

/* reception du fichier dans le thread  (du serveur au client )
 * param : void * fileNameParam : nom du fichier 
 * */
void * recvfile_th(void * fileNameParam);

/* gestion des messages dans le thread
 * param : void * dSparam : numéro de la socket sur laquelle envoyer
 * */
void * sending_th(void * dSparam);

/* gestion du thread de réception
 * param: void * dSparam : numéro de la socket sur laquelle recevoir
 * */
void * receiving_th(void * dSparam);

#endif
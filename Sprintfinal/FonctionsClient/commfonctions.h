/*fichier pour gestion l'interaction entre le client et le serveur */
#ifndef COMMFONCTIONS_H_
#define COMMFONCTIONS_H_

#include "global.h"
#include "thfonctions.h"

/* fonction pour gérer CTRL+C*/
void Ctrl_C_Handler(int sign);

/*envoi d'un message */
void sending(int dS, char * msg);

/*envoie d'un entier */
void sendingInt(int dS, int number);

/*reception d'un message */
void receiving(int dS, char * rep, ssize_t size);

/*reception d'un entier (generalement pour la reception des bytes)*/
int receivingInt(long dS);


/*Création du socket client pour communiquer avec le serveur */
int createSocketClient(int port, char * ip);

/*envoi du fichier,et la création du thread pour le faire*/
void sendfile(int dS);

/*reception du fichier et création du thread pour le faire */
void recvfile(int dS);

#endif
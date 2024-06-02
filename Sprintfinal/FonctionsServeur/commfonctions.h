#ifndef DIALOGFUNCTIONS_H_
#define DIALOGFUNCTIONS_H_

#include "global.h"
#include "traitmessage.h"
#include "traitclients.h"
#include "thfonctions.h"

/* Gestion du signal CTRL+C côté serveur */
void Ctrl_C_Handler(int sign);

/* Arrête le serveur proprement */
void closeServeur();

/* Fonctions d'envoi et de réception des messages */
void receiving(int dS, char *rep, ssize_t size);

/* Reçoit un entier à partir de la socket spécifiée */
int receivingInt(long dS);

/* Envoie un message via la socket spécifiée */
void sending(int dS, char *msg);

/* Envoie un entier via la socket spécifiée */
void sendingInt(int dS, int number);
/*---------------------------------------------*/
/* Envoie un message à tous les clients connectés */
void sendingAll(int numClient, char *msg);

/* Envoie un message à tous les clients connectés dans un salon spécifique */
void sendingRoom(int numClient, char *msg);

/* Envoie un message privé à un client spécifique */
void sendingPrivate(int numClient, char *msg);

/* Fonctions pour la gestion des connexions côté serveur */
int createSocketServeur(int port);

/* Accepte une connexion client sur le serveur */
int acceptConnection(int dS);
/*---------------------------------------------*/

/* Envoie un fichier via une socket */
void sendFile(int dS, FILE *fp);

/* Upload d'un fichier du client vers le serveur */
void uploadFile(int dS);

/* Download d'un fichier du serveur vers le client */
void downloadFile(int dS, char *msgReceived);

/* Vérifie si un client est administrateur et peut arrêter le serveur */
/* Retourne 1 si le client est admin, 0 sinon */
int canShutdown(int numClient);

#endif /* DIALOGFUNCTIONS_H_ */

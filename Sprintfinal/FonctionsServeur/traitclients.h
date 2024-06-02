#ifndef TRAITCLIENT_H_
#define TRAITCLIENT_H_

/* ----- FICHIER DE GESTION DES CLIENTS ----- */

#include "global.h"
#include "commfonctions.h"

/*indice du premier emplacement disponible dans le tableau des clients 
-1 si tout est occupée*/
int giveNumClient();

/*recherche du nombre de client dans le tableau du clients qui a ce pseudo 
-1 s'il ne trouve pas */
int findClient(char * pseudo);

/*envoi des clients connectés  à un client (numclient:son numéroclient dans le tableau des clients)*/
void displayClient(int numClient);

/*envoi des liste des admins du serveur */
void displayAdmin(int numClient);


/*Envoie le profil d'un autre client au client*/

void displayOneClient(int numClient, char * msg);

/*
 Initialisation du tableau des clients à partir du fichier clients.txt qui 
 sauvegarde les comptes et informations des clients inscrits sur le serveur 
 */
void initClients();

/*
 Sauvegarde des comptes et informations des clients inscrits sur le serveur dans le fichier clients.txt
 */
void saveClients();

/**
 modifier le pseudo du  profil de client avec le numéro de client numclient
 * */
void updatePseudo(int numClient, char * msg);

/**
 modifie la description du profil de client avec le numéro du client numclient
 */
void updateDescr(int numClient, char * msg);

/*modifie le mot de passe du profil de client avec le numéro du client numclient */
void updatePassword(int numClient, char * msg);

/*création d'un compte client */
int createAccount(int dSC, char * pseudo, int numClient);

/*permet à un client de se connecter */
int connection(int dSC, int numClient);

/*retourne le nombre des admins du serveur*/
int nbAdmin();

/*supprimer le compte d'un client de numéro client numclient */
int deleteAccount(int numClient);

/*recherche d'un client par son socket*/
int findClientBySocket(int dS);

/**
 * @brief Donne les droits de modification et de suppression d'un salon à un client 
 * @param numClient numéro du client qui donne les droits
 * @param msg message reçu contenant la commande /rightServer, le nom du client concerné et le nom du salon
 * */
void giveRightServer(int numClient, char * msg);

/**
 * @brief Déconnexion propre du client : quitte le salon actuel, réinitialise les variables nécessaires et ferme le socket 
 * @param dS la socket du client 
 * */
void closingClient(int dS);

/**
 Terminaison des threads pour lesquels les clients se sont déconnectés
 */
void killThread();

#endif
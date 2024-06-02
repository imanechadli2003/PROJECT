#ifndef TRAITMESSAGE_H_
#define TRAITMESSAGE_H_

#include "global.h"

/**
 * @brief vérification du déconnexion du client 
 * @param msg message du client à checker
 * @return 1  si le client veut se déconnecter , 0  sinon
*/
int endOfCommunication(char * msg);

/**
 * @brief vérifie si le client veut envoyer un fichier au serveur 
 * @param msg message du client à checker
 * @return 1  si oui, 0 sinon
*/
int issendfile(char * msg);

/**
 * @brief vérifie si un client souhaite recevoir un fichier 
 * @param msg message du client à checker
 * @return 1 si oui, 0  sinon
*/
int isrecvfile(char * msg);

/**
 * @brief vérifie si le serveur à été fermé
 * @param msg message reçu à checker
 * @return 1 si oui, 0  sinon
*/
int isServeurShutdown(char * msg);

/**
 * @brief Vérifie si le client veut voir le manuel
 * @param msg message reçu à checker
 * @return 1 si oui, 0  sinon
*/
int isMan(char * msg);

/**
 * @brief affiche le man.txt au client
 */
void displayMan();

#endif
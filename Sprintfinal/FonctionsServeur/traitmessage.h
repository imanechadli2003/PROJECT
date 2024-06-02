/*FICHIER POUR LA GESTION DES MESSAGES */
#ifndef MANAGEMENTMESSAGE_H_
#define MANAGEMENTMESSAGE_H_


#include "global.h"
#include "commfonctions.h"

/*ajouter le pseudo de l'expediteur au msg */
void addPseudoToMsg(char * msg, char * pseudoSender);

/*Affichage du fichier contenant le manuel */
void displayMan(int numClient);

/*traite le message et retourne le numéro de commande associé à ce message */
int numCommande(char * msg);

/*retourne 0 si le nom n'est pas valide,1 sinon */
int isAvailableName(char * name);

#endif
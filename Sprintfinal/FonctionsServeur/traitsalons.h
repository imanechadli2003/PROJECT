#ifndef TRAITSALONS_H_
#define TRAITSALONS_H_

/* ----- FICHIER DE GESTION DES SALONS DE DISCUSSION ----- */

#include "global.h"
#include "commfonctions.h"


void initRoom();


void welcomeMsg(int dS);

void presentationRoom(int dS);


void createRoom(int numClient, char * msg);


void addMember(int numClient, int idRoom);


void deleteMember(int numClient, int idRoom);


void joinRoom(int numClient, char * msg);


void moveClient(int numClient, char * msg);


void kickClient(int numClient, char * msg);

void banClient(int numClient, char * msg);

void unbanClient(int numClient, char * msg);


void giveRightRoom(int numClient, char * msg);


void updateRoom(int room, int admin, int ban);


int isOccupiedRoom(int idRoom);


void removeRoom(int numClient, char * msg);


int getNonCreatedRoom();


void updateNameRoom(int numClient, char * msg);


void updateDescrRoom(int numClient, char * msg);


int getRoomByName(char * nameRoom);

#endif 

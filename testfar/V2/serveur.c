#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#define max 10 
//une structure pour définir les propriétés du client 
typedef struct Client Client;
struct Client{
    int occupe;
    char * pseudo;
    long dSC;
};
Client tableauclient[max]; // tableau pour regrouper tous les clients
pthread_t tableauthread[max]; //chaque thread a son propre thread
long total=0; // variable pour le nombre de clients présents

 // fonction pour retourner la premiere place disponible 
 //on incrémente tant que la place actuelle est occupée
 //si elle retourne -1 c'est à dire qu'on a arrivé au max
int numclient(){
    int i = 0;
    while (i<max){
        if(!tableauclient[i].occupe){
            return i;
        }
        i+=1;
    }
    
    return -1;
}
//fonction pour envoyer le message à tous les autres clients
//fonction prend en parametre le descripteur du socket de notre client
//et le message qu'on veut envoyer 
void sendmsgg(int dS, char * msg){
    int i;
    for (i = 0; i<max ; i++) {
        if(tableauclient[i].occupe && dS != tableauclient[i].dSC){
            int sendR = send(tableauclient[i].dSC, msg, strlen(msg)+1, 0);
            if (sendR == -1){ /*vérification de la valeur de retour*/
                perror("erreur au send");
                exit(-1);
            }
        }
    }
}
//fonction pour recevoir le message d'un socket
void receivingmsgg(int dS, char * rep, ssize_t size){
    int recvR = recv(dS, rep, size, 0);
    if (recvR == -1){ /*vérification de la valeur de retour*/
        perror("erreur au recv");
        exit(-1);
    }
}
int endOfCommunication(char * msg){
    if (strcmp(msg, "a quitté la conversation\n")==0){
        return 1;
    }
    return 0;
}
void * broadcast(void * clientnum){
    int fin = 0;
    int numClient = (long) clientnum;
    char * pseudoSender = tableauclient[numClient].pseudo;

    while(!fin){
        /*on recoit le message*/
        char * msg= (char *) malloc(sizeof(char)*100);
        receivingmsgg(tableauclient[numClient].dSC, msg, sizeof(char)*100);
        printf("\nMessage recu: %s \n", msg);

        /*On verifie si le client veut terminer la communication*/
        fin = endOfCommunication(msg);

        /*Ajout du pseudo de l'expéditeur devant le message à envoyer*/
        char * msgToSend = (char *) malloc(sizeof(char)*115);
        strcat(msgToSend, pseudoSender);
        strcat(msgToSend, " : ");
        strcat(msgToSend, msg);

        /*Envoi du message aux autres clients*/
        printf("Envoi du message aux %ld clients. \n", total);
        sendmsgg(tableauclient[numClient].dSC, msgToSend);
        
    }

    /*Fermeture du socket client*/
    total= total-1;
    tableauclient[numClient].occupe=0;
    close(tableauclient[numClient].dSC);

    return NULL;
}

int main(int argc, char *argv[]) {
    /*Verification des paramètres*/
    if(argc<2){
        printf("vérifiez votre commande  ");
    }

	/*Création de la socket*/
	int dS = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in ticket;
	ticket.sin_family = AF_INET;
	ticket.sin_addr.s_addr = INADDR_ANY;
	ticket.sin_port = htons(atoi(argv[1]));

	int bindR = bind(dS, (struct sockaddr*)&ticket, sizeof(ticket));
	if (bindR == -1){
		perror("erreur au bind");
		exit(-1);
	}

	int listenR = listen(dS, max);
	if (listenR == -1){
		perror("erreur au listen");
		exit(-1);
	}


    while(1){
        int dSC;

        if(total < max){

            struct sockaddr_in aC;
            socklen_t lg = sizeof(struct sockaddr_in);
            dSC = accept(dS, (struct sockaddr*) &aC,&lg);
            if (dSC == -1){
                perror("erreur au accept");
                exit(-1);
            }

            long numClient = numclient();

            if (send(dSC, &total, sizeof(int), 0) == -1){
                perror("erreur au send du numClient");
                exit(-1);
            }

            printf("Client %ld connecté\n", numClient);

            tableauclient[numClient].occupe = 1;
            tableauclient[numClient].dSC = dSC;

            char * pseudo = (char *) malloc(sizeof(char)*100);
            receivingmsgg(dSC, pseudo, sizeof(char)*12);

            pseudo = strtok(pseudo, "\n");
            tableauclient[numClient].pseudo = (char *) malloc(sizeof(char)*12);
            strcpy(tableauclient[numClient].pseudo,pseudo);

            strcat(pseudo," a rejoint la conversation\n");
            sendmsgg(dSC, pseudo);

            free(pseudo);


                //COMMUNICATION//

            int threadReturn = pthread_create(&tableauthread[numClient],NULL,broadcast,(void *)numClient);
            if(threadReturn == -1){
                perror("erreur ");
            }

            
            total += 1;
            
            printf("Clients connectés : %ld\n", total);
            
        }

    }
	close(dS);
    return 0;
}



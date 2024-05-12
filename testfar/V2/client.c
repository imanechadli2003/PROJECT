#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


int fin = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


    //Cette fonction vérifie si le client souhaite mettre fin à la communication.
   // Paramètres : char ** msg - le message du client à examiner
    //Renvoie : 1 (vrai) si le client souhaite quitter, 0 (faux) sinon
    

int endOfCommunication(char * msg){
    if (strcmp(msg, "fin\n")==0){
        strcpy(msg, "a quitté la conversation\n");
        return 1;
    }
    return 0;
}


void sendmsgg(int dS, char * msg){
    int sendR = send(dS, msg, strlen(msg)+1, 0);
    if (sendR == -1){ /*vérification de la valeur de retour*/
        perror("erreur au send");
        exit(-1);
    }
}
//fonction pour notre thread
void * sending_th(void * dSparam){
    int dS = (long)dSparam;
    while (!fin){

        /*Saisie du message au clavier*/
        char * m = (char *) malloc(sizeof(char)*100);

        printf(">");
        fgets(m, 100, stdin);

        /*On vérifie si le client veut quitter la communication*/
        fin = endOfCommunication(m);
        
        /*Envoi*/
        sendmsgg(dS, m);

        free(m);
    }
    close(dS);
    return NULL;
}

//Cette fonction reçoit un message d'une socket et vérifie si tout se déroule correctement.
void receiving(int dS, char * rep, ssize_t size){
    int recvR = recv(dS, rep, size, 0);
    if (recvR == -1){ /*vérification de la valeur de retour*/
        printf("** fin de la communication **\n");
        exit(-1);
    }
}

//Le thread du receive 
void * receiving_th(void * dSparam){
    int dS = (long)dSparam;
    while(!fin){

        char * r = (char *) malloc(sizeof(char)*100);
        receiving(dS, r, sizeof(char)*100);
        printf(">%s",r);

        free(r);
    }
    close(dS);
    return NULL;
}


//notre main
//le programme doit s'écuter en donnant l'ip et le port 
int main(int argc, char *argv[]) {

    /*Verification des paramètres*/
    if(argc<3){
        printf("Evérifiez votre commande  ");
    }

	/*Création de la socket*/
	long dS = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in aS;
	aS.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &(aS.sin_addr));
	aS.sin_port = htons(atoi(argv[2]));

	/*Demander une connexion*/
	socklen_t lgA = sizeof(struct sockaddr_in);
	int connectR = connect(dS, (struct sockaddr *) &aS, lgA);
	if (connectR == -1){
		perror("erreur au connect");
		exit(-1);
	}

    /*Reception du nombre de client*/
    int nbClient;
    if (recv(dS, &nbClient, sizeof(int), 0) == -1){ 
        perror("erreur au recv du numClient");
        exit(-1);
    }
        
    char * myPseudo = (char *) malloc(sizeof(char)*12);
    printf("Votre pseudo (maximum 12 caractères): ");
    fgets(myPseudo, 12, stdin);

   //envoie le pseudo 
    sendmsgg(dS, myPseudo);

    /*En attente d'un autre client*/
    if(nbClient==0){
        printf("En attente d'un autre client\n");

        /*Reception du premier message informant de l'arrivée d'un autre client*/
        char * msg = (char *) malloc(sizeof(char)*100);
        receiving(dS, msg, sizeof(char)*100);
        printf("%s",msg);

        free(msg);
    }
// COMMUNICATION 
//création de deux threads (un pour l'envoi,un pour la reception)
    pthread_t thread_sending;
    pthread_t thread_receiving;

    int thread1 = pthread_create(&thread_sending, NULL, sending_th, (void *)dS);
    if(thread1==-1){
        perror("error thread 1");
    }

    int thread2 = pthread_create(&thread_receiving, NULL, receiving_th, (void *)dS);
    if(thread2==-1){
        perror("error thread 2");
    }
    pthread_join(thread_sending, NULL);
    pthread_join(thread_receiving, NULL);
    
    return 0;
}
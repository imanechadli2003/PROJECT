#include "commfonctions.h"

/* Déclarations des fonctions */
void Ctrl_C_Handler(int sign);
void closeServeur();
void receiving(int dS, char *rep, ssize_t size);
int receivingInt(long dS);
void sending(int dS, char *msg);
void sendingInt(int dS, int number);
void sendingAll(int numClient, char *msg);
void sendingRoom(int numClient, char *msg);
void sendingPrivate(int numClient, char *msg);
int createSocketServeur(int port);
int acceptConnection(int dS);
void sendFile(int dS, FILE *fp);
void uploadFile(int dS);
void downloadFile(int dS, char *msgReceived);
int canShutdown(int numClient);

/* Gestion du signal CTRL+C côté serveur */
void Ctrl_C_Handler(int sign) {
    closeServeur();
    printf("\nCTRL+C détecté : Arrêt du serveur\n");
    exit(EXIT_SUCCESS);
}

/* Fermeture propre du serveur */
void closeServeur(){
    saveClients();
    updateRoom(1, 1, 1);

    int i; 
    for(i = 0; i < MAX_CLIENT; i++){
        if(tabClient[i].connected){
            sending(tabClient[i].dSC, "/shutdown");
            close(tabClient[i].dSC);
        }
    }

    shutdown(dS, 2);
    shutdown(dSFile, 2);
}

/* Réception d'un message de la socket 'dS' et stockage dans 'rep' */
void receiving(int dS, char *rep, ssize_t size){
    int recvR = recv(dS, rep, size, 0);
    if (recvR == -1){
        perror("Erreur lors de la réception");
        exit(-1);
    }
}

/* Réception d'un entier de la socket 'dS' */
int receivingInt(long dS){
    int number;
    int recvR = recv(dS, &number, sizeof(int), 0);
    if(recvR == -1){
        perror("Erreur lors de la réception d'un entier");
        exit(-1);
    } else if(recvR == 0){
        closingClient(dS);
    }
    return number;
}

/* Envoi d'un message à la socket 'dS' */
void sending(int dS, char *msg){
    int sendR = send(dS, msg, strlen(msg) + 1, 0);
    if (sendR == -1){
        perror("Erreur lors de l'envoi");
        exit(-1);
    }
}

/* Envoi d'un entier à la socket 'dS' */
void sendingInt(int dS, int number){
    int sendR = send(dS, &number, sizeof(int), 0);
    if (sendR == -1){
        perror("Erreur lors de l'envoi de l'entier");
        exit(-1);
    }
}

/* Envoi d'un message à tous les clients connectés */
void sendingAll(int numClient, char *msg){
    pthread_mutex_lock(&lock);

    int dS = tabClient[numClient].dSC;
    addPseudoToMsg(msg, tabClient[numClient].pseudo);
    
    int i;
    for (i = 0; i < MAX_CLIENT; i++) {
        if(tabClient[i].connected && dS != tabClient[i].dSC){
            sending(tabClient[i].dSC, msg);
        }
    }

    pthread_mutex_unlock(&lock);
}

/* Envoi d'un message à tous les clients dans un salon spécifique */
void sendingRoom(int numClient, char *msg){
    pthread_mutex_lock(&lock);

    int dS = tabClient[numClient].dSC;
    addPseudoToMsg(msg, tabClient[numClient].pseudo);

    int i;
    int idRoom = tabClient[numClient].idRoom;
    for (i = 0; i < MAX_CLIENT; i++) {
        if(rooms[idRoom].members[i] && dS != tabClient[i].dSC){
            sending(tabClient[i].dSC, msg);
        }
    }

    pthread_mutex_unlock(&lock);
}

/* Envoi d'un message privé à un client spécifique */
void sendingPrivate(int numClient, char *msg){
    pthread_mutex_lock(&lock);

    int mydSC = tabClient[numClient].dSC;

    pthread_mutex_unlock(&lock);

    char *pseudo = (char *) malloc(SIZE_MSG);
    strcpy(pseudo, msg);
    pseudo = strtok(pseudo, " ");
    strcpy(pseudo, pseudo + 1);

    int client = findClient(pseudo);

    if (client == -1){
        sending(mydSC, "Le pseudo saisi n'existe pas!\n");
    } else if (!tabClient[client].connected){
        sending(mydSC, "Le client n'est pas connecté!\n");
    } else {
        pthread_mutex_lock(&lock);
        addPseudoToMsg(msg, tabClient[numClient].pseudo);
        pthread_mutex_unlock(&lock);

        sending(tabClient[client].dSC, msg);
        printf("Envoi du message à %s\n", pseudo);
    }
    free(pseudo);
}

/* Création de la socket serveur */
int createSocketServeur(int port){
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(port);

    int bindRFile = bind(dS, (struct sockaddr*)&ad, sizeof(ad));
    if (bindRFile == -1){
        perror("Erreur lors du bind");
        exit(-1);
    }

    int listenRFile = listen(dS, MAX_CLIENT);
    if (listenRFile == -1){
        perror("Erreur lors de l'écoute");
        exit(-1);
    }

    return dS;
}

/* Accepte une connexion client sur le serveur */
int acceptConnection(int dS){
    int dSC;
    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);
    dSC = accept(dS, (struct sockaddr*) &aC, &lg);
    if (dSC == -1){
        perror("Erreur lors de l'acceptation");
        exit(-1);
    }
    return dSC;
}

/* Envoi d'un fichier via une socket */
void sendFile(int dS, FILE *fp){
    char data[1024];
    int nbBytes = -1;
    int fd = fileno(fp);

    while(nbBytes != 0){
        nbBytes = read(fd, data, 1023);
        data[1023] = '\0';
        sendingInt(dS, nbBytes);
        if(nbBytes != 0){
            if (send(dS, data, sizeof(data), 0) == -1) {
                perror("Erreur lors de l'envoi du fichier.");
                exit(1);
            }
        }
        bzero(data, nbBytes);
        sleep(0.1);
    }
    printf("\n** Fichier envoyé **\n");
    fclose(fp);
}

/* Réception d'un fichier du client vers le serveur */
void uploadFile(int dS){
    char *fileName = (char *) malloc(SIZE_MSG);
    receiving(dS, fileName, SIZE_MSG);

    printf("\nNom du fichier à recevoir: %s\n", fileName);

    if(strcmp(fileName, "error") == 0){
        printf("Nom de fichier incorrect\n");
    } else {
        pthread_t threadFile;
        int thread = pthread_create(&threadFile, NULL, uploadFile_th, (void *)fileName);
        if(thread == -1){
            perror("Erreur lors de la création du thread");
        }
    }
}

/* Envoi d'un fichier du serveur vers le client */
void downloadFile(int dS, char *msgReceived){
    sending(dS, msgReceived);

    int cr = system("ls ./FileServeur > listeFichier.txt");
    if(cr == -1){
        printf("Commande échouée");
    }

    FILE *fp = fopen("listeFichier.txt", "r");
    sendFile(dS, fp);

    char *fileName = (char *) malloc(SIZE_MSG);
    receiving(dS, fileName, SIZE_MSG);
    printf("\nNom du fichier à envoyer: %s\n", fileName);

    char *pathToFile = (char *) malloc(sizeof(char) * 130);
    strcpy(pathToFile, "FileServeur/");
    strcat(pathToFile, fileName);

    fp = fopen(pathToFile, "r");

    if (fp == NULL) {
        char *error = "error";
        printf("Erreur! Fichier inconnu\n");
        sending(dS, error);
    } else {
        sending(dS, fileName);

        pthread_t threadFile;
        int thread = pthread_create(&threadFile, NULL, downloadFile_th, (void *)fp);
        if(thread == -1){
            perror("Erreur lors de la création du thread");
        }
    }
    free(pathToFile);
}

/* Vérifie si un client est administrateur et peut arrêter le serveur */
int canShutdown(int numClient){
    if(tabClient[numClient].isAdmin){
        return 1;
    } else {
        sending(tabClient[numClient].dSC, "Vous n'avez pas les droits pour fermer le serveur.\n\n");
        return 0;
    }
}

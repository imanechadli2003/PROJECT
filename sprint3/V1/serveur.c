#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#define FILE_DIR "./received_files"  // Répertoire pour les fichiers reçus

/* Prototypes des fonctions */
void handle_exit_cmd();

typedef struct Client {
    int occupied;
    char *pseudo;
    long dSC;
} Client;

bool s_r = true;
#define MAX_CLIENT 10
Client c[MAX_CLIENT];
pthread_t t[MAX_CLIENT];
sem_t semNbClient;
int tabThreadToKill[MAX_CLIENT];

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void handle_sigint(int sig) {
    s_r = false;
    handle_exit_cmd();
    exit(0);
}

void send_msg(int dS, char *msg) {
    int sendR = send(dS, msg, strlen(msg) + 1, 0);
    if (sendR == -1) {
        perror("erreur au send");
        exit(-1);
    }
}

void handle_exit_cmd() {
    s_r = false;
    char exitMessage[] = "** Fin de la communication. **\n";
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (c[i].occupied) {
            send_msg(c[i].dSC, exitMessage);
            close(c[i].dSC);
            c[i].occupied = 0;
        }
    }
}

void *cmd_handler(void *arg) {
    char command[100];
    while (fgets(command, sizeof(command), stdin) != NULL) {
        if (strcmp(command, "exit\n") == 0) {
            handle_exit_cmd();
            break;
        }
    }
    exit(0);
}

int give_client_num() {
    int i = 0;
    int index = -1;
    pthread_mutex_lock(&lock);
    while (i < MAX_CLIENT && index == -1) {
        if (!c[i].occupied) {
            index = i;
        }
        i += 1;
    }
    pthread_mutex_unlock(&lock);
    return index;
}

int find_client(char *pseudo) {
    int i = 0;
    int client = -1;
    pthread_mutex_lock(&lock);
    while (i < MAX_CLIENT && client == -1) {
        if (c[i].occupied) {
            if (strcmp(pseudo, c[i].pseudo) == 0) {
                client = c[i].dSC;
            }
        }
        i += 1;
    }
    pthread_mutex_unlock(&lock);
    return client;
}

int is_avail_pseudo(char *pseudo) {
    int i = 0;
    int available = 1;
    pthread_mutex_lock(&lock);
    while (i < MAX_CLIENT && available) {
        if (c[i].occupied) {
            if (strcmp(c[i].pseudo, pseudo) == 0) {
                available = 0;
            }
        }
        i += 1;
    }
    pthread_mutex_unlock(&lock);
    return available;
}

void add_pseudo_to_msg(char *msg, char *pseudoSender) {
    char *msgToSend = (char *)malloc(sizeof(char) * 150);
    strcpy(msgToSend, pseudoSender);
    strcat(msgToSend, " : ");
    strcat(msgToSend, msg);
    strcpy(msg, msgToSend);
    free(msgToSend);
    return;
}

void receive(int dS, char *rep, ssize_t size) {
    int recvR = recv(dS, rep, size, 0);
    if (recvR == -1) {
        perror("erreur au recv");
        exit(-1);
    }
}

void send_help_msg(int dS) {
    FILE *file = fopen("manuel.txt", "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de fonctionnalites");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        send_msg(dS, line);
    }
    fclose(file);
}

void send_int(int dS, int number) {
    int sendR = send(dS, &number, sizeof(int), 0);
    if (sendR == -1) {
        perror("erreur au send");
        exit(-1);
    }
}

void send_all(int numClient, char *msg) {
    pthread_mutex_lock(&lock);
    int dS = c[numClient].dSC;
    add_pseudo_to_msg(msg, c[numClient].pseudo);
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (c[i].occupied && dS != c[i].dSC) {
            send_msg(c[i].dSC, msg);
        }
    }
    pthread_mutex_unlock(&lock);
}

void send_private(int numClient, char *msg) {
    pthread_mutex_lock(&lock);
    int mydSC = c[numClient].dSC;
    char *copyMsg = (char *)malloc(sizeof(char) * 100);
    strcpy(copyMsg, msg);
    char *pseudo = (char *)malloc(sizeof(char) * 13);
    pseudo = strtok(copyMsg, " ");
    strcpy(pseudo, pseudo + 1);
    pthread_mutex_unlock(&lock);
    int dSC = find_client(pseudo);
    if (dSC == -1) {
        char *error = (char *)malloc(sizeof(char) * 100);
        error = "Le pseudo saisit n'existe pas!\n";
        send_msg(mydSC, error);
        printf("%s", error);
        free(error);
    } else {
        pthread_mutex_lock(&lock);
        add_pseudo_to_msg(msg, c[numClient].pseudo);
        pthread_mutex_unlock(&lock);
        send_msg(dSC, msg);
        printf("Envoi du message à %s\n", pseudo);
    }
    free(pseudo);
}

int end_communication(char *msg) {
    if (strcmp(msg, "** a quitté la communication **\n") == 0) {
        return 1;
    }
    return 0;
}

void receive_file(int dS, char *filename, long filesize) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", FILE_DIR, filename);

    FILE *file = fopen(filepath, "wb");
    if (file == NULL) {
        perror("Erreur lors de la création du fichier");
        return;
    }

    char buffer[1024];
    long bytesReceived = 0;
    while (bytesReceived < filesize) {
        int bytes = recv(dS, buffer, sizeof(buffer), 0);
        if (bytes == -1) {
            perror("Erreur lors de la réception du fichier");
            break;
        }
        fwrite(buffer, 1, bytes, file);
        bytesReceived += bytes;
    }

    fclose(file);
    printf("Fichier %s reçu.\n", filename);
}

void *broadcast(void *clientParam) {
    int isEnd = 0;
    int numClient = (long)clientParam;
    while (!isEnd) {
        char *msgReceived = (char *)malloc(sizeof(char) * 100);
        receive(c[numClient].dSC, msgReceived, sizeof(char) * 100);
        printf("\nMessage recu: %s \n", msgReceived);
        isEnd = end_communication(msgReceived);
        if (!isEnd) {
            if (strcmp(msgReceived, "help\n") == 0) {
                send_help_msg(c[numClient].dSC);
            } else if (strncmp(msgReceived, "file ", 5) == 0) {
                char filename[256];
                long filesize;
                sscanf(msgReceived + 5, "%s %ld", filename, &filesize);
                receive_file(c[numClient].dSC, filename, filesize);
            } else if (strcmp(msgReceived, "exit\n") == 0) {
                handle_exit_cmd();
                break;
            } else {
                char first = msgReceived[0];
                if (strcmp(&first, "@") == 0) {
                    send_private(numClient, msgReceived);
                } else {
                    printf("Envoi du message aux autres clients. \n");
                    send_all(numClient, msgReceived);
                }
            }
        }
        free(msgReceived);
    }
    sem_post(&semNbClient);
    pthread_mutex_lock(&lock);
    c[numClient].occupied = 0;
    tabThreadToKill[numClient] = 1;
    close(c[numClient].dSC);
    pthread_mutex_unlock(&lock);
    return NULL;
}

void kill_thread() {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENT; i++) {
        if (tabThreadToKill[i]) {
            pthread_cancel(t[i]);
            tabThreadToKill[i] = 0;
        }
    }
    pthread_mutex_unlock(&lock);
}

int main(int argc, char *argv[]) {
    pthread_t command_thread;
    sem_init(&semNbClient, 0, MAX_CLIENT);
    if (argc < 2) {
        printf("Erreur : Lancez avec ./serveur <votre_port> ");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_CLIENT; i++) {
        tabThreadToKill[i] = 0;
    }
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(atoi(argv[1]));
    int bindR = bind(dS, (struct sockaddr*)&ad, sizeof(ad));
    if (bindR == -1) {
        perror("erreur au bind");
        exit(-1);
    }
    int listenR = listen(dS, MAX_CLIENT);
    if (listenR == -1) {
        perror("erreur au listen");
        exit(-1);
    }
    if (pthread_create(&command_thread, NULL, cmd_handler, NULL) != 0) {
        perror("Erreur lors de la création du thread de gestion des commandes");
        exit(EXIT_FAILURE);
    }
    while (s_r) {
        int dSC;
        sem_wait(&semNbClient);
        kill_thread();
        struct sockaddr_in aC;
        socklen_t lg = sizeof(struct sockaddr_in);
        dSC = accept(dS, (struct sockaddr*)&aC, &lg);
        if (dSC == -1) {
            perror("erreur au accept");
            exit(-1);
        }
        int valueSem;
        sem_getvalue(&semNbClient, &valueSem);
        int nbClient = MAX_CLIENT - valueSem;
        long numClient = give_client_num();
        send_int(dSC, nbClient);
        printf("Client %ld connecté\n", numClient);
        char *pseudo = (char *)malloc(sizeof(char) * 100);
        int availablePseudo = 0;
        do {
            send_int(dSC, availablePseudo);
            receive(dSC, pseudo, sizeof(char) * 12);
            pseudo = strtok(pseudo, "\n");
            availablePseudo = is_avail_pseudo(pseudo);
        } while (!availablePseudo);
        send_int(dSC, availablePseudo);
        pthread_mutex_lock(&lock);
        c[numClient].pseudo = (char *)malloc(sizeof(char) * 12);
        strcpy(c[numClient].pseudo, pseudo);
        c[numClient].occupied = 1;
        c[numClient].dSC = dSC;
        pthread_mutex_unlock(&lock);
        strcpy(pseudo, "** a rejoint la communication **\n");
        send_all(numClient, pseudo);
        printf("%s\n", pseudo);
        free(pseudo);
        int threadReturn = pthread_create(&t[numClient], NULL, broadcast, (void *)numClient);
        if (threadReturn == -1) {
            perror("erreur thread create");
        }
        printf("Clients connectés : %d\n", nbClient);
    }
    close(dS);
    sem_destroy(&semNbClient);
    return 0;
}
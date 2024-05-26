#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_MSG_SIZE 100

void sending(int socket, char * msg) {
    ssize_t msg_len = strlen(msg) + 1;
    ssize_t sent = send(socket, &msg_len, sizeof(msg_len), 0);
    if (sent != sizeof(msg_len)) {
        perror("Erreur lors de l'envoi de la taille du message");
        exit(EXIT_FAILURE);
    }
    ssize_t total_sent = 0;
    while (total_sent < msg_len) {
        ssize_t sent_now = send(socket, msg + total_sent, msg_len - total_sent, 0);
        if (sent_now == -1) {
            perror("Erreur lors de l'envoi du message");
            exit(EXIT_FAILURE);
        }
        total_sent += sent_now;
    }
}

void receiving(int socket, char * rep) {
    ssize_t msg_len;
    ssize_t received = recv(socket, &msg_len, sizeof(msg_len), 0);
    if (received != sizeof(msg_len)) {
        perror("Erreur lors de la réception de la taille du message");
        exit(EXIT_FAILURE);
    }
    ssize_t total_received = 0;
    while (total_received < msg_len) {
        ssize_t received_now = recv(socket, rep + total_received, MAX_MSG_SIZE - total_received, 0);
        if (received_now == -1) {
            perror("Erreur lors de la réception du message");
            exit(EXIT_FAILURE);
        }
        total_received += received_now;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Erreur : Lancez avec ./client <votre_ip> <votre_port> ");
        exit(EXIT_FAILURE);
    }

    int dS = socket(PF_INET, SOCK_STREAM, 0);
    if (dS == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(aS.sin_addr));
    aS.sin_port = htons(atoi(argv[2]));

    socklen_t lgA = sizeof(struct sockaddr_in);
    if (connect(dS, (struct sockaddr *) &aS, lgA) == -1) {
        perror("Erreur lors de la connexion");
        exit(EXIT_FAILURE);
    }

    int numClient = 0;
    if (recv(dS, &numClient, sizeof(int), 0) == -1) {
        perror("Erreur lors de la réception du numéro de client");
        exit(EXIT_FAILURE);
    }

    printf("Vous êtes le client numéro %d. \n", numClient);

    char * myPseudo = (char *) malloc(sizeof(char) * 12);
    printf("Votre pseudo (maximum 12 caractères): ");
    fgets(myPseudo, 12, stdin);
    sending(dS, myPseudo);

    if (numClient == 1) {
        printf("En attente d'un autre client\n");
    }

    char * hisPseudo = (char *) malloc(sizeof(char) * 12);
    receiving(dS, hisPseudo);
    hisPseudo = strtok(hisPseudo, "\n");
    printf("\n--------- Vous communiquez avec %s ---------\n", hisPseudo);
    printf("\nVos messages peuvent faire jusqu'à 100 caractères.\n"
           "Pour quitter la conversation envoyez 'fin'.\n"
           "Bonne communication !\n\n");

    if (numClient == 1) {
        char * m = (char *) malloc(sizeof(char) * MAX_MSG_SIZE);
        printf("Vous : ");
        fgets(m, MAX_MSG_SIZE, stdin);
        sending(dS, m);
        free(m);
    }

    int communication = 1;

    while (communication) {
        char * r = (char *) malloc(sizeof(char) * MAX_MSG_SIZE);
        receiving(dS, r);
        printf("%s : %s", hisPseudo, r);

        if (strcmp(r, "** a quitté la communication **\n") == 0) {
            communication = 1;
            free(r);
            break;
        }

        free(r);

        char * m = (char *) malloc(sizeof(char) * MAX_MSG_SIZE);
        printf("Vous : ");
        fgets(m, MAX_MSG_SIZE, stdin);
        sending(dS, m);

        if (strcmp(m, "fin\n") == 0) {
            communication = 1;
            free(m);
            break;
        }

        free(m);
    }

    close(dS);
    return 0;
}


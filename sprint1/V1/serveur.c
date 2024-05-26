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

int endOfCommunication(char ** msg) {
    if (strcmp(*msg, "fin\n") == 0) {
        *msg = "** a fini la conversation **\n";
        printf("%s", *msg);
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Erreur : Lancez avec ./serveur <votre_port> ");
        exit(EXIT_FAILURE);
    }

    int dS = socket(PF_INET, SOCK_STREAM, 0);
    if (dS == -1) {
        perror("Erreur lors de la création de la socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(atoi(argv[1]));

    if (bind(dS, (struct sockaddr*)&ad, sizeof(ad)) == -1) {
        perror("Erreur lors du nommage de la socket");
        exit(EXIT_FAILURE);
    }

    if (listen(dS, 7) == -1) {
        perror("Erreur lors de la mise en écoute de la socket");
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct sockaddr_in aC;
        socklen_t lg = sizeof(struct sockaddr_in);
        int dSC1 = accept(dS, (struct sockaddr*) &aC, &lg);
        if (dSC1 == -1) {
            perror("Erreur lors de l'acceptation de la connexion");
            exit(EXIT_FAILURE);
        }

        int numClient = 1;
        if (send(dSC1, &numClient, sizeof(int), 0) == -1) {
            perror("Erreur lors de l'envoi du numéro de client");
            exit(EXIT_FAILURE);
        }

        printf("Client 1 connecté\n");

        int dSC2 = accept(dS, (struct sockaddr*) &aC, &lg);
        if (dSC2 == -1) {
            perror("Erreur lors de l'acceptation de la connexion");
            exit(EXIT_FAILURE);
        }

        numClient = 2;
        if (send(dSC2, &numClient, sizeof(int), 0) == -1) {
            perror("Erreur lors de l'envoi du numéro de client");
            exit(EXIT_FAILURE);
        }

        printf("Client 2 connecté\n");

        int communication = 1;

        while (communication) {
            char * msg = (char *) malloc(sizeof(char) * MAX_MSG_SIZE);
            receiving(dSC1, msg);
            printf("\nMessage recu du client1: %s \n", msg);
            communication = endOfCommunication(&msg);
            sending(dSC2, msg);
            printf(" -- Message envoye\n");

            if (communication) {
                char * rep = (char *) malloc(sizeof(char) * MAX_MSG_SIZE);
                receiving(dSC2, rep);
                printf("\nReponse recu du client2 : %s \n", rep);
                communication = endOfCommunication(&rep);
                sending(dSC1, rep);
                printf(" -- Reponse envoye\n");
            }
        }

        close(dSC1);
        close(dSC2);
    }

    close(dS);
    return 0;
}


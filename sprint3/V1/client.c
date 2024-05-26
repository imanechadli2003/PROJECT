#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#define DIR_PATH "./files_to_send"  // Répertoire dédié aux fichiers à envoyer

/* Prototypes de fonctions */
void displayHelp(int dS);
int endOfCommunication(char *msg);
void *receive_from_server(void *arg);
void sending(int dS, char *msg);
void *sending_th(void *dSparam);
void receiving(int dS, char *rep, ssize_t size);
void *receiving_th(void *dSparam);
void send_file(int dS);
void list_files();

/* Indicateur de fin de communication */
int isEnd = 0;

/* Affiche l'aide pour le client */
void displayHelp(int dS) {
    char *msg = "help\n";
    sending(dS, msg);
}

/* Vérifie si le message indique la fin de la communication */
int endOfCommunication(char *msg) {
    if (strcmp(msg, "fin\n") == 0) {
        strcpy(msg, "** a quitté la communication **\n");
        return 1;
    }
    return 0;
}

/* Liste les fichiers dans le répertoire dédié */
void list_files() {
    DIR *d;
    struct dirent *dir;
    d = opendir(DIR_PATH);
    if (d) {
        printf("Fichiers disponibles dans le répertoire %s:\n", DIR_PATH);
        while ((dir = readdir(d)) != NULL) {
            char filepath[512];
            struct stat filestat;
            snprintf(filepath, sizeof(filepath), "%s/%s", DIR_PATH, dir->d_name);
            if (stat(filepath, &filestat) == 0 && S_ISREG(filestat.st_mode)) {
                printf("%s\n", dir->d_name);
            }
        }
        closedir(d);
    } else {
        perror("Erreur lors de l'ouverture du répertoire");
    }
}

/* Envoie un fichier au serveur */
void send_file(int dS) {
    list_files();

    char filename[256];
    printf("Entrez le nom du fichier à envoyer: ");
    scanf("%s", filename);

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", DIR_PATH, filename);

    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char file_info[512];
    snprintf(file_info, sizeof(file_info), "file %s %ld\n", filename, filesize);
    sending(dS, file_info);

    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(dS, buffer, bytesRead, 0);
    }

    fclose(file);
    printf("Fichier %s envoyé.\n", filename);
}

/* Thread pour recevoir les messages du serveur */
void *receive_from_server(void *arg) {
    int dSC = *(int *)arg;
    char message[100];
    while (1) {
        receiving(dSC, message, sizeof(message));
        printf("%s", message);
        if (strcmp(message, "** Fin de la communication. **\n") == 0) {
            printf("Le serveur a terminé la communication.\n");
            break;
        }
    }
    close(dSC); /* Fermer la connexion avec le serveur */
    exit(EXIT_SUCCESS); /* Terminer le client */
}

/* Envoie un message au serveur */
void sending(int dS, char *msg) {
    int sendR = send(dS, msg, strlen(msg) + 1, 0);
    if (sendR == -1) {
        perror("erreur au send");
        exit(-1);
    }
}

/* Thread pour envoyer des messages au serveur */
void *sending_th(void *dSparam) {
    int dS = (long)dSparam;
    while (!isEnd) {
        char *m = (char *)malloc(sizeof(char) * 100);
        printf(">");
        fgets(m, 100, stdin);
        if (strcmp(m, "help\n") == 0) {
            displayHelp(dS);
        } else if (strncmp(m, "file", 4) == 0) {
            send_file(dS);
        } else if (strcmp(m, "exit\n") == 0) {
            sending(dS, m);
            isEnd = 1;
        } else {
            isEnd = endOfCommunication(m);
            sending(dS, m);
        }
        free(m);
    }
    close(dS);
    return NULL;
}

/* Réceptionne un message d'une socket et teste que tout se passe bien */
void receiving(int dS, char *rep, ssize_t size) {
    int recvR = recv(dS, rep, size, 0);
    if (recvR == -1) {
        printf("** fin de la communication **\n");
        exit(-1);
    }
}

/* Thread pour recevoir les messages du serveur */
void *receiving_th(void *dSparam) {
    int dS = (long)dSparam;
    while (!isEnd) {
        char *r = (char *)malloc(sizeof(char) * 100);
        receiving(dS, r, sizeof(char) * 100);
        printf(">%s", r);
        if (endOfCommunication(r) || strcmp(r, "** Fin de la communication. **\n") == 0) {
            isEnd = 1;
        }
        free(r);
    }
    close(dS);
    return NULL;
}

/* Fonction principale */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("vérifiez votre commande ");
        exit(EXIT_FAILURE);
    }
    long dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(aS.sin_addr));
    aS.sin_port = htons(atoi(argv[2]));
    socklen_t lgA = sizeof(struct sockaddr_in);
    int connectR = connect(dS, (struct sockaddr *)&aS, lgA);
    if (connectR == -1) {
        perror("erreur ");
        exit(-1);
    }
    int nbClient;
    if (recv(dS, &nbClient, sizeof(int), 0) == -1) {
        perror("erreur ");
        exit(-1);
    }
    int availablePseudo;
    char *myPseudo = (char *)malloc(sizeof(char) * 12);
    recv(dS, &availablePseudo, sizeof(int), 0);
    printf("Votre pseudo : ");
    fgets(myPseudo, 12, stdin);
    sending(dS, myPseudo);
    recv(dS, &availablePseudo, sizeof(int), 0);
    while (!availablePseudo) {
        printf("Pseudo déjà utilisé!\nVotre pseudo : ");
        fgets(myPseudo, 12, stdin);
        sending(dS, myPseudo);
        recv(dS, &availablePseudo, sizeof(int), 0);
    }
    if (nbClient == 1) {
        printf("vous etes seul,attendez une autre personne\n");
        char *msg = (char *)malloc(sizeof(char) * 100);
        receiving(dS, msg, sizeof(char) * 100);
        printf("%s", msg);
        free(msg);
    }
    pthread_t thread_sendind;
    pthread_t thread_receiving;
    int thread1 = pthread_create(&thread_sendind, NULL, sending_th, (void *)dS);
    if (thread1 == -1) {
        perror("error ");
    }
    int thread2 = pthread_create(&thread_receiving, NULL, receiving_th, (void *)dS);
    if (thread2 == -1) {
        perror("error ");
    }
    pthread_join(thread_sendind, NULL);
    pthread_join(thread_receiving, NULL);
    return 0;
}
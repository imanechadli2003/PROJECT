/*fonctions threads pour l'envoi et la réception des messages /fichiers */

#include "thfonctions.h"
#include <fcntl.h>

void * sendfile_th(void * fpParam){

	long dSFile = createSocketClient(port+1, ip);

    FILE * fp = (FILE *)fpParam;

    char data[1024] = "";
    int nbBytes = -1;

    int fd = fileno(fp);


    while(nbBytes != 0){/*retourne 0 si on est arrivé à la fin du fichier */
        nbBytes = read(fd, data, 1023);
        data[1023]='\0';
        sleep(0.3);
        sendingInt(dSFile, nbBytes);
        if(nbBytes != 0){
            if (send(dSFile, data, sizeof(data), 0) == -1) {
                perror("[-]Error in sending file.");
                exit(1);
            }
        }
        bzero(data, nbBytes);
    } 

    printf("\n** Fichier envoyé **\n");
    fclose(fp);
    close(dSFile);
    return NULL;
}


void * recvfile_th(void * fileNameParam){

    long dSFile = createSocketClient(port+1, ip);

    char * fileName = (char *)fileNameParam;

    char * pathToFile = (char *) malloc(sizeof(char)*130);
    strcpy(pathToFile,"FileReceived/");
    strcat(pathToFile,fileName);

    char buffer[1024];
    int fp = open(pathToFile, O_WRONLY |  O_CREAT, 0666);
    if(fp == -1){
        printf("erreur au open");
        exit(1);
    }

    int nbBytes; /*Nombre de bytes à recevoir*/
    nbBytes = receivingInt(dSFile);

    
    while(nbBytes>0){
        recv(dSFile, buffer, 1024, 0);
        write(fp, buffer,nbBytes);
        nbBytes = receivingInt(dSFile);
        bzero(buffer, 1024);
    }
    printf("\n** Fichier reçu **\n");
    
    close(fp);
    close(dSFile);
    return NULL;

}


void * sending_th(void * dSparam){
    int dS = (long)dSparam;

    while (!deconnexion){

        char * m = (char *) malloc(sizeof(char)*100);
        fgets(m, 100, stdin);
        strtok(m,"\n");
        strcat(m,"\0");

        deconnexion = endOfCommunication(m);

        sending(dS, m);

        if (issendfile(m)){ 
            sendfile(dS);
        }else if(isMan(m)){
            displayMan();
        }

        free(m);
    }

    close(dS);
    return NULL;
}

void * receiving_th(void * dSparam){
    int dS = (long)dSparam;
    int serveurShutdown;

    while(!deconnexion && !serveurShutdown){
        
        char * r = (char *) malloc(sizeof(char)*500);
        receiving(dS, r, sizeof(char)*500);

       
        deconnexion = endOfCommunication(r);
        serveurShutdown = isServeurShutdown(r);

        if(isrecvfile(r)){ 
            recvfile(dS);
        }else if(!deconnexion && !serveurShutdown){
            printf("%s",r);
        }

        free(r);
    }

    close(dS);
    return NULL;
}
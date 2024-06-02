/*IMPLEMENTATION DES FONCTIONS DE COMMUNICATION */
#include "commfonctions.h"

void Ctrl_C_Handler(int sign) {
    sending(dS, "/end");
    shutdown(dS, 2);
    printf("\n CTRL+C détecté : Déconnexion... \n");
    exit(EXIT_SUCCESS);
}

void sending(int dS, char * msg){
    int sendR = send(dS, msg, strlen(msg)+1, 0);
    if (sendR == -1){ 
        perror("erreur au send");
        exit(-1);
    }
}

void sendingInt(int dS, int number){
    int sendR = send(dS, &number, sizeof(int), 0);
    if (sendR == -1){ 
        perror("erreur au send");
        exit(-1);
    }
}

void receiving(int dS, char * rep, ssize_t size){
    int recvR = recv(dS, rep, size, 0);
    if (recvR == -1){ 
        perror("erreur receiving\n");
        printf("** fin de la communication **\n");
        exit(-1);
    }
}

int receivingInt(long dS){
    int number;
    if(recv(dS, &number, sizeof(int), 0) == -1){ 
        perror("erreur au recv d'un int");
        exit(-1);
    } 
    return number;
}

int createSocketClient(int port, char * ip){

    /*création du socket client */
	long dS = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in aS;
	aS.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &(aS.sin_addr));
	aS.sin_port = htons(port);

	/*se connecter */
	socklen_t lgA = sizeof(struct sockaddr_in);
	int connectR = connect(dS, (struct sockaddr *) &aS, lgA);
	if (connectR == -1){
		perror("erreur au connect");
		exit(-1);
	}

    return dS;
}

/*préconditions:le fichier doit etre obligatoirement dans le dossier "FILETOSEND*/
void sendfile(int dS){
    printf("\n - les fichier disponibles que vous pouve envoyer -- \n");

    /*afficher les fichiers disponibles à l'envoi*/
    int cr = system("ls --format=single-column ./FileToSend");
    if(cr == -1){
        printf("error");
    }

    char * fileName = (char *) malloc(sizeof(char)*100);
    printf("\n  quel fichier vous voulez envoyer ?: \n");
    fgets(fileName, 100, stdin); /*recevoir le nom de fichier*/ 
    strtok(fileName,"\n");
    strcat(fileName,"\0");
    
    char * pathToFile = (char *) malloc(sizeof(char)*130);
    strcpy(pathToFile,"FileToSend/");
    strcat(pathToFile,fileName); /*ajouter le nom du fichier tappé*/

    FILE * fp = NULL;
    fp = fopen(pathToFile,"r"); /*ouverture de fichier selon le path qu'on a obtenu */

    if (fp== NULL) { 
        char * error = "error";
        sending(dS, error);  /*envoyer un message contenant "error"*/
        printf("Erreur! Fichier inconnu\n"); 

    }else { /*succés de l'ouverture du fichier  */
        /*Envoi du nom du fichier au serveur*/
        sending(dS,fileName);
        /*Ccréation du thread pour envoyer le fichier */
        pthread_t threadFile;
        int thread = pthread_create(&threadFile, NULL, sendfile_th, (void *)fp);
        if(thread==-1){
            perror("error thread");
        }
    }
    free(pathToFile);
    free(fileName);
}

void recvfile(int dS){
    printf("\n --Liste des fichiers que vous pouvez télécharger  ----- \n");
    
    int nbBytes = receivingInt(dS); 
    char buffer[1024];

    /*Reception et affichage*/
    while(nbBytes>0){
        recv(dS, buffer, 1024, 0);
        printf("%s\n",buffer);
        nbBytes = receivingInt(dS);
        bzero(buffer, 1024);
    }

    printf("\n -Vous voulez téléchargez quoi ? : \n");

    char * fileName = (char *) malloc(sizeof(char)*100);
    receiving(dS,fileName,sizeof(char)*100);

    if(strcmp(fileName,"error")==0){ /*si on reçoit error */
        printf("fichier inexistant :)  !\n");
    }else { /*succés*/
        
        pthread_t threadFile;
        int thread = pthread_create(&threadFile, NULL, recvfile_th, (void *)fileName);
        if(thread==-1){
            perror("error thread");
        } 
    }
    return;
}
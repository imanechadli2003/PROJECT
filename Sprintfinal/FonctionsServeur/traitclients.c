#include "traitclients.h"

int giveNumClient(){
    int i = 0;
    int indice = -1;

    pthread_mutex_lock(&lock); 

    while (i<MAX_CLIENT && indice==-1){
        if(!tabClient[i].created){ 
            indice = i;
        }
        i+=1;
    }
    pthread_mutex_unlock(&lock); 

    return indice;
}

int findClient(char * pseudo){
    int i = 0;
    int client = -1;

    pthread_mutex_lock(&lock); 

    while (i<MAX_CLIENT && client==-1){
        if (tabClient[i].created){
            if (strcmp(pseudo, tabClient[i].pseudo)==0){ 
                client = i;
            }
        }
        i+=1;
    }
    pthread_mutex_unlock(&lock); 

    return client;
}

void displayClient(int numClient){

	char * msg = (char *)malloc(sizeof(char)*15*MAX_CLIENT);
	strcpy(msg, "-----------------Utilisateurs connectés :------------------------ \n");
    int i;

    for(i = 0; i < MAX_CLIENT; i++){
    	
    	if(tabClient[i].connected){
    		strcat(msg, "-- ");
    		strcat(msg, tabClient[i].pseudo);
    		strcat(msg, "\n");
    	}
    }
    strcat(msg, "--------------------------------------------------------\n\n");
    sending(tabClient[numClient].dSC, msg);

    free(msg);

	return;
}

void displayAdmin(int numClient){

	char * msg = (char *)malloc(100+sizeof(char)*15*MAX_CLIENT);
	strcpy(msg, "--------------Les admins de ce serveur -----------------\n");
    int i;

    for(i = 0; i < MAX_CLIENT; i++){
    	
    	if(tabClient[i].isAdmin){
    		strcat(msg, "-- ");
    		strcat(msg, tabClient[i].pseudo);
    		strcat(msg, "\n");
    	}
    }
    strcat(msg, "---------------------------------------------------------------- \n\n");
    sending(tabClient[numClient].dSC, msg);
    free(msg);

	return;
}

void displayOneClient(int numClient, char * msg){

    strtok(msg," "); 
    char * hisPseudo = strtok(NULL,"\0"); 

    if(hisPseudo==NULL){ 
        sending(tabClient[numClient].dSC, "Saisissez le pseudo d'un client.\n");  

    }else{
        int client = findClient(hisPseudo);

        if(client==-1){ 
            sending(tabClient[numClient].dSC, "Aucun client ne correspond à ce pseudo.\n");
        
        }else{ 

            char *  profil =  (char *) malloc(sizeof(char)*300);
            strcpy(profil,"\n** ");

            pthread_mutex_lock(&lock); 
            
            strcat(profil,tabClient[client].pseudo);
            strcat(profil," **\n-- ");
            strcat(profil,tabClient[client].descr);
            strcat(profil," --\n");

            if(tabClient[client].isAdmin){
                strcat(profil,"~~ Administrateur du serveur ~~\n");
            }

            int i;
            for (i=1;i<NB_ROOMS;i++){
                if(rooms[i].admin[client]){
                    strcat(profil,"~~ Administrateur du salon : ");
                    strcat(profil,rooms[i].name);
                    strcat(profil," ~~\n");
                }
            }
            strcat(profil,"\n");

            pthread_mutex_unlock(&lock); 

            sending(tabClient[numClient].dSC, profil); 
            free(profil);
        }
    }
}

void initClients(){
    int i = 0;
    char buffer[100] = "";

    FILE *fp = fopen("FunctionsServeur/clients.txt","r");
    if(fp==NULL){
        perror("error\n");
    }

    pthread_mutex_lock(&lock); 
    
    while(fgets(buffer,100, (FILE *) fp)!=NULL && i<MAX_CLIENT){
        
        /*ID*/
        tabClient[i].id = atoi(strtok(buffer,","));

        /*PSEUDO*/
        strcpy(tabClient[i].pseudo,strtok(NULL,","));

        /*PASSWORD*/
        strcpy(tabClient[i].password,strtok(NULL,","));

        /*DESCRIPTION*/
        strcpy(tabClient[i].descr,strtok(NULL,","));

        /*CONNECTED*/
        tabClient[i].connected = 0;

        /*CREATED*/
        tabClient[i].created = atoi(strtok(NULL,","));

        /*ISADMIN*/
        tabClient[i].isAdmin = atoi(strtok(NULL,","));

        i++;
    }
    while(i<MAX_CLIENT){ 

        /*ID*/
        tabClient[i].id = i;

        /*PSEUDO*/
        strcpy(tabClient[i].pseudo,"Default");

        /*PASSWORD*/
        strcpy(tabClient[i].password,"Default");

        /*DESCRIPTION*/
        strcpy(tabClient[i].descr,"Default");

        /*CONNECTED*/
        tabClient[i].connected = 0;

        /*CREATED*/
        tabClient[i].created = 0;

        /*ISADMIN*/
        tabClient[i].isAdmin = 0;

        i++;

    }

    pthread_mutex_unlock(&lock); /*Fin d'une section critique*/

    fclose(fp);

    printf("\n-- Clients initialisés --\n");
}

void saveClients(){
    char * line = (char *)malloc(sizeof(char)*200);
    
    int ret = remove("./FunctionsServeur/clients.txt");
    if (ret == -1){
        perror("erreur suppression \n");
        exit(1);
    }

    int fd = open("./FunctionsServeur/clients.txt", O_CREAT | O_WRONLY, 0666);
    if (fd == -1){
        perror("erreur création saveClients: \n");
        exit(1);
    }


    int i;
    for (i = 0; i < MAX_CLIENT; i++){
        char id[2];
        char created [2];
        char isAdmin [2];

        /*ID*/
        sprintf(id,"%d",tabClient[i].id);
        strcpy(line,id);
        strcat(line,",");

        /*PSEUDO*/
        strcat(line,tabClient[i].pseudo);
        strcat(line,",");

        /*PASSWORD*/
        strcat(line,tabClient[i].password);
        strcat(line,",");

        /*DESCRIPTION*/
        strcat(line,tabClient[i].descr);
        strcat(line,",");

        /*CREATED*/
        sprintf(created,"%d",tabClient[i].created);
        strcat(line, created);
        strcat(line, ",");

        /*ISADMIN*/
        sprintf(isAdmin,"%d",tabClient[i].isAdmin);
        strcat(line, isAdmin);
        strcat(line, "\n\0");
        
        int w = write(fd,line,strlen(line));
        if(w == -1){
            perror("erreur au write\n");
            exit(1);
        }
    }

    printf("\n-- Fichier clients mis à jour --\n");
    free(line);
    return;
}

void updatePseudo(int numClient, char * msg){

    strtok(msg," "); 
    char * newPseudo = strtok(NULL,"\0"); 

    if(newPseudo==NULL){ 
        sending(tabClient[numClient].dSC, "donnez le nouveau pseudo.\n\n");       
    
    }else if(!isAvailableName(newPseudo)){ 
        sending(tabClient[numClient].dSC, "Un pseudo ne peut pas contenir d'espace.\n\n");  
    
    }else{ /*succés*/

        int client = findClient(newPseudo);
        if(client!=-1){ 
            sending(tabClient[numClient].dSC, "déjà utilisé .\n\n"); 
        
        }else{
            
            char * info = (char *)malloc(sizeof(char)*60);
            strcpy(info,"** est désormais ");
            strcat(info,newPseudo);
            strcat(info," **");
            
            sendingRoom(numClient,info);

            pthread_mutex_lock(&lock); 

            strcpy(tabClient[numClient].pseudo,newPseudo);
        
            
            saveClients();

            pthread_mutex_unlock(&lock); 

            sending(tabClient[numClient].dSC, "Votre pseudo a été mis à jour.\n\n");  
        }
    }
}

void updateDescr(int numClient, char * msg){

    strtok(msg," "); 
    char * newDescr = strtok(NULL,"\0"); 

    if(newDescr==NULL){ 
        sending(tabClient[numClient].dSC, "donnez la nouvelle description.\n\n");       
    
    }else{ 

        pthread_mutex_lock(&lock); 

        strcpy(tabClient[numClient].descr,newDescr);
    
        
        saveClients();

        pthread_mutex_unlock(&lock); 

        sending(tabClient[numClient].dSC, "Votre description a été mise à jour.\n\n"); 
    }
}

void updatePassword(int numClient, char * msg){

    
    strtok(msg," "); 
    char * oldPassword = strtok(NULL," "); 

    if(oldPassword==NULL){ 
        sending(tabClient[numClient].dSC, "donnez votre ancien mot de passe.\n\n");       
    
    }else{ 

        char * newPassword = strtok(NULL,"\0"); 

        if(newPassword==NULL){ 
            sending(tabClient[numClient].dSC, "donnez le nouveau mot de passe .\n\n");       
        
        }if(strcmp(oldPassword,tabClient[numClient].password)!=0){ 
            sending(tabClient[numClient].dSC, "Mot de passe incorrect\n\n");              
        
        }else{ 

            pthread_mutex_lock(&lock); 

            strcpy(tabClient[numClient].password,newPassword);
    
            saveClients();

            pthread_mutex_unlock(&lock); 

            sending(tabClient[numClient].dSC, "Votre mot de passe a été mis à jour.\n\n");
        }

    }
}

int createAccount(int dSC, char * pseudo, int numClient){

    strcpy(tabClient[numClient].pseudo,pseudo);

    sending(dSC,"vous n'etes pas inscrit, inscrivez vous!\n donnez un mot de passe : \n");

    char * password = (char *) malloc(sizeof(char)*20);
    receiving(dSC, password, sizeof(char)*20);

    if(strcmp(password,"cancel")==0){ 
        free(password);
        return 0;
    }


    strcpy(tabClient[numClient].password,password);

    printf("-- Password : %s\n",tabClient[numClient].password);

    strcpy(tabClient[numClient].descr,"Default");
    tabClient[numClient].created = 1; 
    tabClient[numClient].dSC = dSC;

    tabClient[numClient].connected = 1;
    saveClients();
    addMember(numClient,0); 
    welcomeMsg(dSC);

    free(password);
    return 1;

}

int connection(int dSC, int numClient){

    sending(dSC,"\n AH HAPPY TO SEE YOU AGAIN,JOIN US ! \ndonne ton  mot de passe : \n");

    char * password = (char *) malloc(sizeof(char)*20);
    receiving(dSC, password, sizeof(char)*20);

    int availablePassword = strcmp(password,tabClient[numClient].password)==0;
    int cancel = strcmp(password,"cancel")==0;

    while(!availablePassword && !cancel){
        printf("Mot de passe incorrect!\n");
        sending(dSC,"Mot de passe incorrect!\nSaissisez votre mot de passe : \n");
        receiving(dSC, password, sizeof(char)*15);
        availablePassword = strcmp(password,tabClient[numClient].password)==0;
        cancel = strcmp(password,"cancel")==0;
    }

    if(cancel){ 
        free(password);
        return 0;
    }

    printf("Mot de passe correct!\n");

    tabClient[numClient].connected = 1;
    tabClient[numClient].dSC = dSC;

    addMember(numClient,0);
    welcomeMsg(dSC);

    free(password);
    return 1;
}

int nbAdmin(){
    int i = 0;
    int nb = 0;

    pthread_mutex_lock(&lock); 

    for(i=0;i<MAX_CLIENT;i++){
        if (tabClient[i].isAdmin){ 
            nb+=1;
        }
    }

    pthread_mutex_unlock(&lock); 

    return nb;
}

int deleteAccount(int numClient){

    if(tabClient[numClient].isAdmin && nbAdmin()==1){ 

        sending(tabClient[numClient].dSC, "vous ne pouvez pas supprimer votre compte,il y a que toi qui est admin,donnez le role à quelqu'un avant\n\n");
        return 0;

    }else{ 

       
        sending(tabClient[numClient].dSC, "Etes vous surs de continuer la suppression du compte ? y/n \n");
        char * rep = (char *)malloc(SIZE_MSG);
        receiving(tabClient[numClient].dSC,rep,sizeof(char)*60);

        if(strcmp(rep,"y")==0){ /*on entame la suppression :) */

            pthread_mutex_lock(&lock); 

            sending(tabClient[numClient].dSC,"/end");

            tabClient[numClient].created=0;
            tabClient[numClient].isAdmin=0;
            strcpy(tabClient[numClient].descr,"Default");
            int i;
            
            for(i=0;i<NB_ROOMS;i++){
                rooms[i].admin[numClient]=0;
                rooms[i].ban[numClient]=0;
            }

            /*on fait la mise à jour du fichier clients */
            saveClients();

            pthread_mutex_unlock(&lock); 
            return 1;

        }else{ /*LE client ne veut pas supprimer son compte, annulation*/

            sending(tabClient[numClient].dSC, "annultion du suppression.\n\n");
            return 0;
        }

        free(rep);

    }
}

int findClientBySocket(int dS){
    int i = 0;
    int client = -1;

    pthread_mutex_lock(&lock); 

    while (i<MAX_CLIENT && client==-1){
        if (tabClient[i].connected){
            if (dS == tabClient[i].dSC){
                client = i;
            }
        }
        i+=1;
    }
    pthread_mutex_unlock(&lock); 

    return client;
}

void giveRightServer(int numClient, char * msg){

    strtok(msg," "); 
    char *  pseudo = strtok(NULL," "); 
    if(pseudo==NULL){ 
        sending(tabClient[numClient].dSC, "Sdonnez le pseudo du client que vous voulez nommer comme admin.\n\n");       
    
    }else{

        int client = findClient(pseudo); 
        if(client==-1){ 
            sending(tabClient[numClient].dSC, "inexistant !.\n\n"); 
        
        }else if(!tabClient[numClient].isAdmin){ 
            sending(tabClient[numClient].dSC, "VOus devez etre admin du serveur pour pouvoir faire cette opération\n.\n"); 
        
        }else{ 
            
            tabClient[client].isAdmin=1;

            /*mise à jour du fichier clients*/
            saveClients();
            sending(tabClient[numClient].dSC, "Les droits du serveur ont été transmis au client.\n\n"); 

            if(tabClient[client].connected){
                sending(tabClient[client].dSC, "Vous avez été déclaré administrateur du serveur.\n\n"); 
            }
        }
    }

    return;
}

void closingClient(int dS){ 
    int numClient = findClientBySocket(dS);

    deleteMember(numClient,tabClient[numClient].idRoom);

    pthread_mutex_lock(&lock);
    tabClient[numClient].connected=0;

    tabThreadToKill[nbThreadToKill]=tabThread[numClient]; 
    nbThreadToKill+=1;
    tabThread[numClient] = ((pthread_t)0);

    close(tabClient[numClient].dSC);

    pthread_mutex_unlock(&lock);
    sem_post(&semNbClient); 
}

void killThread(){
    int i;

    pthread_mutex_lock(&lock); 
    
    for (i=0;i<MAX_CLIENT*2;i++){
        if(tabThreadToKill[i]!=((pthread_t)0)){
            pthread_join(tabThreadToKill[i],NULL);
        }
    }
    nbThreadToKill=0;

    pthread_mutex_unlock(&lock); 
}
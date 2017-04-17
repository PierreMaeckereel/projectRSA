#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<errno.h>

void afficheErreur(char* message){
  perror(message);
  exit(1);
}

int pubsFiltre(char* tampon){
  FILE* fichier = fopen("easylist.txt","r+");
  char ligne[1000] = "";

  while(fgets(ligne,1000,fichier)){
    if(strncmp(tampon,ligne,strlen(ligne)) == 0){
      return 1;
    }
  }
  return 0;
}

int main(int argc, char** argv){
  pid_t pid;
  struct sockaddr_in client_addr, server_addr;
  int socketfd, newsocketfd;
  struct hostent* hote;

  if(argc < 2){
    afficheErreur("Le proxy doit etre lancé avec un numero de port en parametre");
  }

  printf("\n*** MyAdsBlocker port n° %s ***\n",argv[1]);

  bzero((char*)&server_addr,sizeof(server_addr));
  bzero((char*)&client_addr,sizeof(client_addr));

  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(atoi(argv[1]));
  server_addr.sin_family = AF_INET;

  socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(socketfd < 0){ afficheErreur("Problème d'initialisation de la socket serveur");}
  if(bind(socketfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){ afficheErreur("Erreur de liaison de la socket serveur");}
  listen(socketfd,50);
  int clientlenght = sizeof(client_addr);

  bouclepoint:

  newsocketfd = accept(socketfd, (struct sockaddr*)&client_addr, &clientlenght);
  if(newsocketfd < 0){ afficheErreur("Problème accès à la connexion");}
  pid = fork();
  if(pid == 0){
    struct sockaddr_in hote_addr;
    int boolPort = 0, newsocketfdtrans, port, i, socketfdtrans, retoursend;
    char split1[300], split2[300], split3[10], tampon[510];
    char* splittok = NULL;
    int hey, pubsPresence;
    bzero((char*)tampon,500);
    hey = recv(newsocketfd,tampon,500,0);
    if(hey < 0){ afficheErreur("erreur recv l.54");}
    sscanf(tampon,"%s %s %s",split1,split2,split3);

    if(((strncmp(split1,"GET",3)==0))&&((strncmp(split3,"HTTP/1.1",8)==0)||(strncmp(split3,"HTTP/1.0",8)==0))&&(strncmp(split2,"http://",7)==0)){
      strcpy(split1,split2);
      for(i=7; i<strlen(split2); i++){
        if(split2[i] == ':'){
          boolPort = 1;
          break;
        }
      }

      splittok = strtok(split2,"//");
      if(boolPort != 1){
        splittok = strtok(NULL,"/");
        port = 80;
      }else{
        splittok = strtok(NULL,":");
      }

      sprintf(split2,"%s",splittok);
      printf("Hote : %s\n", split2);
      hote = gethostbyname(split2);

      if(boolPort != 0){
        splittok = strtok(NULL,"/");
        port = atoi(splittok);
      }

      strcat(split1,"^]");
      splittok = strtok(split1,"//");
      splittok = strtok(NULL,"/");
      if(splittok != NULL){
        splittok = strtok(NULL,"^]");
      }
      printf("Chemin : %s\n",splittok);
      printf("Port : %d\n",port);

      bzero((char*)&hote_addr,sizeof(hote_addr));
      bcopy((char*)hote->h_addr, (char*)&hote_addr.sin_addr.s_addr, hote->h_length);
      hote_addr.sin_port = htons(port);
      hote_addr.sin_family = AF_INET;

      socketfdtrans = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      newsocketfdtrans = connect(socketfdtrans, (struct sockaddr*)&hote_addr, sizeof(struct sockaddr));
      sprintf(tampon,"\nConnected to %s  IP - %s\n",split2,inet_ntoa(hote_addr.sin_addr));
      if(newsocketfdtrans < 0){ afficheErreur("Erreur de connexion au serveur distant");}
      printf("\n%s\n",tampon);
      bzero((char*)tampon,sizeof(tampon));

      pubsPresence = 0;

      if(splittok != NULL){
        sprintf(tampon,"GET /%s %s\r\nHost: %s\r\nConnection: close\r\n\r\n",splittok,split3,split2);
        //pubsPresence = pubsFiltre(splittok);
      }else{
        sprintf(tampon,"GET / %s\r\nHost: %s\r\nConnection: close\r\n\r\n",split3,split2);
      }

      if(pubsPresence == 0){
        retoursend = send(socketfdtrans,tampon,strlen(tampon),0);
        printf("\n%s\n",tampon);
        if(retoursend < 0){
          afficheErreur("Erreur d'écriture dans la socket");
        }else{
          do{
            bzero((char*)tampon,500);
            retoursend = recv(socketfdtrans,tampon,500,0);
            if(retoursend > 0){
              send(newsocketfd, tampon, retoursend, 0);
            }
          }while(retoursend > 0);
        }
      }
    }else{
      send(newsocketfd,"400 : BAD REQUEST\nONLY HTTP REQUESTS ALLOWED",18,0);
    }
    close(socketfdtrans);
    close(newsocketfd);
    close(socketfd);
    _exit(0);
  }else{
    close(newsocketfd);
    goto bouclepoint;
  }
  return 0;
}

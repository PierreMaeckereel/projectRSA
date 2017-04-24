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

int TAILLE_MAX = 6000;

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
  struct sockaddr_in client_addr;
  int socketfd, newsocketfd;

  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s;

  if(argc < 2){
    afficheErreur("Le proxy doit etre lancé avec un numero de port en parametre");
  }

  printf("\n*** MyAdsBlocker port n° %s ***\n",argv[1]);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  s = getaddrinfo(NULL, argv[1], &hints, &result);
  if(s != 0){ afficheErreur("Erreur GetAddrInfo");}

  for(rp = result; rp != NULL; rp->ai_next){
    socketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if(socketfd == -1){
      continue;
    }
    if(bind(socketfd, rp->ai_addr, rp->ai_addrlen) == 0){
      break;
    }
  }

  if(rp == NULL){
    afficheErreur("Erreur de binding");
  }
  freeaddrinfo(result);

  listen(socketfd,50);
  int clientlenght = sizeof(client_addr);

  bouclepoint:

  newsocketfd = accept(socketfd, (struct sockaddr*)&client_addr, &clientlenght);
  if(newsocketfd < 0){ afficheErreur("Problème accès à la connexion");}
  pid = fork();
  if(pid == 0){
    struct sockaddr_in hote_addr;
    int boolPort = 0, i, socketfdtrans, retoursend;
    char split1[300], split2[300], split3[10], tampon[TAILLE_MAX];
    char* splittok = NULL;
    char* port = NULL;
    int hey, pubsPresence;
    bzero((char*)tampon,TAILLE_MAX);

    struct addrinfo hintscli;
    struct addrinfo *rescli, *rpcli;
    int scli;

    hey = recv(newsocketfd,tampon,TAILLE_MAX,0);
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
        port = "80";
      }else{
        splittok = strtok(NULL,":");
      }

      sprintf(split2,"%s",splittok);
      printf("Hote : %s\n", split2);

      if(boolPort != 0){
        splittok = strtok(NULL,"/");
        port = splittok;
      }

      strcat(split1,"^]");
      splittok = strtok(split1,"//");
      splittok = strtok(NULL,"/");
      if(splittok != NULL){
        splittok = strtok(NULL,"^]");
      }
      printf("Chemin : %s\n",splittok);
      printf("Port : %s\n",port);

      //Utilisation de getaddrinfo
      memset(&hintscli, 0, sizeof(struct addrinfo));
      hintscli.ai_family = AF_UNSPEC;
      hintscli.ai_socktype = SOCK_STREAM;
      hintscli.ai_flags = 0;
      hintscli.ai_protocol = IPPROTO_TCP;

      scli = getaddrinfo(split2, port, &hintscli, &rescli);
      if(scli != 0){ afficheErreur("Erreur getaddrinfo client");}

      for(rpcli = rescli; rpcli != NULL; rpcli = rpcli->ai_next){
        socketfdtrans = socket(rpcli->ai_family, rpcli->ai_socktype, rpcli->ai_protocol);
        if(socketfdtrans == -1){ continue;}
        if(connect(socketfdtrans, rpcli->ai_addr, rpcli->ai_addrlen) != -1){ break;}
      }

      if(rpcli == NULL){ afficheErreur("Erreur de connexion côté client");}
      freeaddrinfo(rescli);

      sprintf(tampon,"\nConnected to %s  IP - %s\n",split2,inet_ntoa(hote_addr.sin_addr));
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
            bzero((char*)tampon,TAILLE_MAX);
            retoursend = recv(socketfdtrans,tampon,TAILLE_MAX,0);
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

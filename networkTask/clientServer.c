#include "clientServer.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include <netinet/in.h>
#include <arpa/inet.h>

unsigned char mode = 0;

void *serverThread(void *arg); /*Main task for server part (receive data from client)*/
void *clientThread(void *arg); /*Main task for client part (send data to server)*/
void *clientReaderThread(void *arg); /*Reader for client*/
void *serverWriterThread(void *arg); /*Writer for server*/

int main (int argc, char *argv[])
{
   /*Check input arg*/
   if(2!=argc)
   {
      printf("Invalid input params\n");return 0;
   }
   else
   {
     if (0 == strcmp(argv[1], "Server"))mode = SERVER_MODE;
     else if (0 == strcmp(argv[1], "Client")) mode = CLIENT_MODE;
     else return 0;
   }

   /*Creating server and client thread*/
   pthread_t serverThread_id;
   pthread_t clientThread_id;

   if( mode == SERVER_MODE)
   {
      pthread_create(&serverThread_id, NULL, serverThread, NULL);
      pthread_join(serverThread_id, NULL);
   }
   else
   {
      pthread_create(&clientThread_id, NULL, clientThread, NULL);
      pthread_join(clientThread_id, NULL);
   }


}

void *clientThread(void *arg)
{
    pthread_t clientReaderThread_id;

    int socket_fd = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    /*Creating socket file descriptor*/
    if (-1 == (socket_fd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("Socket create error!\n");
        pthread_exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    /*Convert  addresses*/
    if(-1 == inet_pton(AF_INET, IP, &serv_addr.sin_addr))
    {
        printf("Invalid address!\n");
        pthread_exit(0);
    }
    /*Connecting to server*/
    if ( -1 == connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        printf("Connection error!\n");
        pthread_exit(0);
    }

    pthread_create(&clientReaderThread_id, NULL, clientReaderThread, &socket_fd);

    do
    {
       sleep(1);
       printf(">");
       scanf("%s", &buffer);
    }while(send(socket_fd, buffer, strlen(buffer),0)>0);
    pthread_exit(0);
}

void *clientReaderThread(void *arg)
{
   if(NULL != arg)
   {
      int socket = *(int*)arg;
      char buffer[1024] = {0};

      while(recv(socket, buffer, 1024,0)>0)
      {
         printf("\n> %s \n", buffer);
         sleep(1);
      }
   }
   pthread_exit(0);
}


void *serverThread(void *arg)
{
    pthread_t serverWriterThread_id;

    int server_fd;
    int new_socket;
    int valread;
    struct sockaddr_in address;
    char buffer[1024] = {0};
    int addrlen = sizeof(address);
    /*Creating socket file descriptor*/
    if (-1 == (server_fd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("Socket create error!\n");
        pthread_exit(0);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    /* Attaching socket to the port*/
    if (0 != bind(server_fd, (struct sockaddr *)&address,sizeof(address)))
    {
        printf("Socket attaching error!\n");
        pthread_exit(0);
    }

    /*Listening the port*/
    if (0 != listen(server_fd, 10))
    {
        printf("Listening!\n");
        pthread_exit(0);
    }

    while(1)
    {
       if (-1 == (new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)))
       {
            printf("Accept error!\n");
            pthread_exit(0);
       }

       pthread_create(&serverWriterThread_id, NULL, serverWriterThread, &new_socket);

       while(valread = recv(new_socket, buffer, 1024,0)>0)
       {
          printf("\n> %s \n", buffer);
          sleep(1);
       }
       pthread_cancel(serverWriterThread_id);
       close(server_fd);
   }
}

void *serverWriterThread(void *arg)
{
   if(NULL != arg)
   {
      int socket = *(int*)arg;
      char buffer[1024] = {0};

      while(1)
      {
        printf(">");
        scanf("%s", &buffer);
        send(socket, buffer, strlen(buffer),0);
        sleep(1);
      }
   }
   pthread_exit(0);
}

/******************************************************************************
    �����ļ�����TCPServer.c
    ����˵����  TCP/IP�������˳���

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SERVER_PORT 5676 /*�����û����Ӷ˿�*/
#define MAXSOCK 10 /*���ٵȴ����ӿ���*/
#define MAXDATASIZE 100 /* ÿ�ο��Խ��յ�����ֽ� */
int main()
   {
    int sockfd; /* ������socket������ */ 
    
    /* ����TCP Socket ,����socket������,�罨��ʧ�ܣ������˳�*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <0) {
      perror("socket");
      exit(1);
    }
   
    struct sockaddr_in server_addr; /* ��������ַ��Ϣ */
    
    /* ��ʼ����������ַ��Ϣ */
    bzero(&server_addr,sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;        
    server_addr.sin_port = htons(SERVER_PORT);    
    server_addr.sin_addr.s_addr = INADDR_ANY; 

    /* �󶨷�����socket������ */
    if (bind(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0) {
      perror("bind");
      exit(1);
    }
   
    /* ����������socket������ */ 
    if (listen(sockfd, MAXSOCK) <0) {
      perror("listen");
      exit(1);
    }


    int clientfd; /* �ͻ���socket������ */
    struct sockaddr_in client_addr; /* �ͻ��˵�ַ��Ϣ */
    int sin_size;
    
    while(1) {   
        
    	 sin_size = sizeof(struct sockaddr_in);
         if ((clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size)) <0) {
             perror("accept");
             continue;
         }

         printf("server: got connection from %s\n",inet_ntoa(client_addr.sin_addr)); 
     
         char buf[MAXDATASIZE];
         int  numbytes;
         if ((numbytes=recv(clientfd, buf, MAXDATASIZE, 0)) == -1) {
             perror("recv");
             exit(1);
         }
         buf[numbytes] = '\0';
         printf("Received: %s",buf);  
        

         if (send(clientfd, "Hello, world!\n", 14, 0) <0){
            perror("send");
         }
         close(clientfd);
    }
    close(sockfd); 
    return 1;
}


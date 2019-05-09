/******************************************************************************
    程序文件名：TCPServer.c
    程序说明：  TCP/IP服务器端程序

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
#define SERVER_PORT 5676 /*定义用户连接端口*/
#define MAXSOCK 10 /*多少等待连接控制*/
#define MAXDATASIZE 100 /* 每次可以接收的最大字节 */
int main()
   {
    int sockfd; /* 服务器socket描述符 */ 
    
    /* 建立TCP Socket ,返回socket描述符,如建立失败，程序退出*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <0) {
      perror("socket");
      exit(1);
    }
   
    struct sockaddr_in server_addr; /* 服务器地址信息 */
    
    /* 初始化服务器地址信息 */
    bzero(&server_addr,sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;        
    server_addr.sin_port = htons(SERVER_PORT);    
    server_addr.sin_addr.s_addr = INADDR_ANY; 

    /* 绑定服务器socket描述符 */
    if (bind(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr))<0) {
      perror("bind");
      exit(1);
    }
   
    /* 监听服务器socket描述符 */ 
    if (listen(sockfd, MAXSOCK) <0) {
      perror("listen");
      exit(1);
    }


    int clientfd; /* 客户端socket描述符 */
    struct sockaddr_in client_addr; /* 客户端地址信息 */
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


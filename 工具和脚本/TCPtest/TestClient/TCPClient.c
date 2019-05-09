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
#define PORT 5676 /* 客户机连接远程主机的端口 */
#define MAXDATASIZE 100 /* 每次可以接收的最大字节 */
#define SERVER_IP "192.168.200.137"
int main() {
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct sockaddr_in their_addr; /* connector's address information */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	     perror("socket");
	     exit(1);
    }

    bzero(&their_addr,sizeof(their_addr)); /* zero the rest of the struct */    
    their_addr.sin_family = AF_INET; /* host byte order */
    their_addr.sin_port = htons(PORT); /* short, network byte order */
    their_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct   sockaddr)) == -1) {
	     perror("connect");
		 exit(1);
	}

    if (send(sockfd, "Hello, world!\n", 14, 0) <0){
            perror("send");
    } 

	if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
		 perror("recv");
		 exit(1);
	}
	buf[numbytes] = '\0';
	printf("Received: %s",buf);
	close(sockfd);
	return 0;
}


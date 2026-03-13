#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>

int main(){
	int listen_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(listen_sockfd == -1){
		perror("creating socket error!");
		exit(-1);
	}
	
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8888);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	int res = bind(listen_sockfd,(sockaddr*)&addr,sizeof(addr));
	if(res == -1){
		perror("binding error!");
		exit(-1);
	}

	res = listen(listen_sockfd,128);
	if(res == -1){
		perror("listening error!");
		exit(-1);
	}

	sockaddr_in clientAddress;
	char temp[1024] = {0};
	socklen_t client_len = sizeof(sockaddr_in);
	
	int connect_sockfd = accept(listen_sockfd,(sockaddr*)&clientAddress,&client_len);
	if(connect_sockfd == -1){
		perror("accept error!");
		exit(-1);
		}

	while(1){
	res = read(connect_sockfd,temp,1024);
	if(res == -1){
		perror("read error!");
		exit(-1);
		}
	

	for(int i = 0;i < 1024;i++){
		temp[i] = toupper(temp[i]);
	}

	write(connect_sockfd,temp,res);
	}
	
	close(listen_sockfd);
	close(connect_sockfd);
}

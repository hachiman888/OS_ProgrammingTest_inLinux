#include <unistd.h>
#include <iostream>
#include <fcntl.h>

int main(){
	int fd = open("targetText.txt",O_RDWR|O_CREAT|O_TRUNC,0664);
	if(fd < 0){
		perror("open error!");
	}
	int ret = dup2(fd,STDOUT_FILENO);
	if(ret < 0){
		perror("dup2 error!");
	}
	pid_t pid = fork();
	if(pid == 0){
		execlp("ps","ps","aux",NULL);
	}else{
		sleep(3);
		close(fd);
	}
}

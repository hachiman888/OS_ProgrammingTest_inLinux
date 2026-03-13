#include <unistd.h>
#include <iostream>
#include <fcntl.h>
using namespace std;

int main(){
	pid_t pid;
	int fd[2];
	int ret = pipe(fd);

	if(ret == -1) {
		perror("pipe error!");
		exit(-1);
	}
	
	pid = fork();
	if(pid == 0){
		close(fd[0]);
		dup2(fd[1],STDOUT_FILENO);
		execlp("ls","ls",NULL);
	}else{
		close(fd[1]);
		dup2(fd[0],STDIN_FILENO);
		execlp("wc","wc","-l",NULL);
	}	
}

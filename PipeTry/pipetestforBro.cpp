#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>

int main()
{
	int fd[2];
	int ret =  pipe(fd);
	if(ret == -1) perror("pipe error!");
	pid_t pid,wpid;
	int i = 0;

	for(;i < 2;i++){
		pid = fork();
		if(pid == -1) perror("fork error!");
		if(pid == 0) break;
	}
	
	if(i == 2){
		close(fd[0]);
		close(fd[1]);
		while((wpid = waitpid(-1,NULL,0)) != -1){
		}

	}else if(i == 0){//older brother
		close(fd[0]);
		dup2(fd[1],STDOUT_FILENO);
		execlp("ls","ls",NULL);
	}else if(i == 1){//younger brother
		close(fd[1]);
		dup2(fd[0],STDIN_FILENO);
		execlp("wc","wc","-l",NULL);
	}

} 

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

int main(){
	pid_t pid,wpid;
	int status;
	int i;
	for(i = 0; i < 3;i++){
		if((pid = fork()) == 0) break;
	}
	if(i == 3){
		sleep(3);
		while((wpid = waitpid(-1,&status,WNOHANG)) != -1){
			if(WIFEXITED(status)){
				std::cout << "child exiting status: " << WEXITSTATUS(status) << std::endl;
			}else if(WIFSIGNALED(status)){
				std::cout << "child killed by signal: " << WTERMSIG(status) << std::endl;
			}
		}
		std::cout << "this is parent" << std::endl;
	}
	else{
		if(i == 0) execlp("ps","ps","aux",NULL);
		if(i == 1) execl("./test1","./test1",NULL);
		if(i == 2) execl("test2","test2",NULL);
	}
}

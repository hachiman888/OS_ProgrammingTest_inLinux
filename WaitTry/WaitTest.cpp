#include <unistd.h>
#include <iostream>
#include <sys/wait.h>

int main(){
	pid_t pid,wpid;
	int status;
	pid = fork();

	if(pid == 0){
		std::cout << "i am child,my pid is: " << getpid() << std::endl;
		sleep(10);
		std::cout << "i am going to die" << std::endl;
		return 99;		
	}else if(pid > 0) {
		wpid = wait(&status);
		if(wpid == -1){
			perror("wait error!");
			exit(1);
		}
		if(WIFEXITED(status)){
			std::cout << "exited with: " << WEXITSTATUS(status) << std::endl;
		}
		if(WIFSIGNALED(status)){
			std::cout << "killed by signal:" << WTERMSIG(status) << std::endl;
		}

		std::cout << "parent is going to die" << std::endl; 
	}else{
		perror("fork error");
		exit(1);
	}
}

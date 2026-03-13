#include <iostream>
#include <unistd.h>

int main(){
	std::cout << "before forking------" << std::endl;
	std::cout << "before forking------" << std::endl;
	std::cout << "before forking------" << std::endl;
	std::cout << "before forking------" << std::endl;
	std::cout << "before forking------" << std::endl;

	pid_t pid = fork();
	if(pid == -1) {
		perror("fork error!");
		exit(1);
	}else if(pid == 0){
		std::cout << "this is the child process\t" << getpid() << std::endl;
	}else{
		std::cout << "this is the parent process\t" <<"child process id: " << pid << std::endl;
	}

	std::cout << "process stopping" << std::endl;
}

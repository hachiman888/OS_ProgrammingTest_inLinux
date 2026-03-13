#include <iostream>
#include <signal.h>

void sys_err(const char* msg){
	perror(msg);
	exit(-1);
}

void signalHandler(int signum){
	if(signum == SIGINT){
		std::cout << "cought it\t" << signum << std::endl; 
	}else if(signum == SIGQUIT){
		std::cout << "!!!!!!cought it\t" << signum << std::endl;
	}
}

int main(){
	struct sigaction act,oldact;
	act.sa_handler = signalHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT,&act,&oldact);
	sigaction(SIGQUIT,&act,&oldact);

	while(1);
}

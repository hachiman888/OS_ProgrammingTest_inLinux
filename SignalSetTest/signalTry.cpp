#include <iostream>
#include <signal.h>

void checkMember(sigset_t* set){
	for(int i = 1;i < 32;i++){
		if(sigismember(set,i))
			std::cout.put('1');
		else std::cout.put('0');
	}
	std::cout << std::endl;
}

int main(){
	sigset_t set,old_set,myset;

	sigemptyset(&set);
	sigaddset(&set,SIGINT);
	
	int ret = sigprocmask(SIG_BLOCK,&set,&old_set);
	if(ret == -1){
		perror("sigprocmask error!");
		exit(-1);
	}
	while(1){
		ret = sigpending(&myset);
		
		if(ret == -1){
		perror("sigpending error!");
		exit(-1);
		}
		checkMember(&myset);
		sleep(2);	
	}
}

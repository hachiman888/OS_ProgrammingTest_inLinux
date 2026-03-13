#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void sys_err(const char* msg){
	perror(msg);
	exit(-1);
}

int main(){
	pid_t pid = fork();
	if(pid != 0){
		return 0;
	}else if(pid == 0){
		int ret = setsid();
		if(ret == -1){
			sys_err("setsid error!");
		}

		int ret2 = chdir("..");
		if(ret2 == -1)
			sys_err("chdir error!");

		 umask(022);
		 close(STDIN_FILENO);
		 int fd = open("/dev/null",O_RDWR);
		 if(fd == -1) 
			 sys_err("open error!");

		 dup2(fd,STDOUT_FILENO);
		 dup2(fd,STDERR_FILENO);

		 while(1);
	}else sys_err("fork error!");
}

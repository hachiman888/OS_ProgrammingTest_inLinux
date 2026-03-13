#include<iostream>
#include<unistd.h>
#include<fcntl.h>

int main(int argc,char* argv[]){
	int fd1 = open(argv[1],O_RDONLY);
	int fd2 = open(argv[2],O_RDWR|O_CREAT|O_TRUNC,0664);
	char buff[1024];
	int n = 0;

	if(fd1 == -1) {
		perror("open agrv1 error");
		exit(-1);
	}
	if(fd2 == -1){
		perror("open argv2 error");
		exit(-1);
	}
	while((n = read(fd1,buff,1024)) != 0){
		if(n < 0){ perror("read error");break;}
		write(fd2,buff,n);//write as much  i read
	}
	close(fd1);
	close(fd2);
}


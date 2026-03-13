#include <unistd.h>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/mman.h>

void sys_err(const char* msg){
    perror(msg);
    exit(-1);
}

int main(){
    int fd = open("ForTest.txt",O_RDONLY);

    if(fd == -1) {
        sys_err("open error!");
    }

    int len = lseek64(fd,0,SEEK_END);
    char* p = static_cast<char*>(mmap(NULL,len,PROT_READ,MAP_SHARED,fd,0));
    close(fd);

    if(p == MAP_FAILED){
        sys_err("mmap error!");
    }

    std::cout << p << std::endl;
    int ret = munmap(p,len);
    if(ret == -1){
        sys_err("munmap error!");
    }
}
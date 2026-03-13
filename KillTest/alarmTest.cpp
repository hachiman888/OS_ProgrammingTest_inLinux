#include <iostream>
#include <sys/time.h>
#include <unistd.h>

int main(){
    int temp = alarm(1);
    for(int i = 0;;i++){
        std::cout << i << '\t';
    }
}
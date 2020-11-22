#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char* argv[]){
    for(int i = 1;i <= 10;i++){
        int bid_list[21] = {
		    20, 18, 5, 21, 8, 7, 2, 19, 14, 13,
		    9, 1, 6, 10, 16, 11, 4, 12, 15, 17, 3
        };
        int bid = bid_list[atoi(argv[1]) + i - 2] * 100;
        printf("%d %d\n",atoi(argv[1]),bid);
        fflush(stdout);
    }
    exit(0);
}
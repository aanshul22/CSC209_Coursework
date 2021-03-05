#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int ret;

    printf("A\n");

    ret = fork();

    printf("B\n");

    if (ret < 0) {
        perror("fork");
        exit(1);
    } else if (ret == 0) {
        printf("C\n");
    } else {
        printf("D\n");
    }

    printf("E\n");
    return 0;
}

// Question 1: Which lines of output are printed more than once?
//
// Ans. B and E
//
// Question 2: Write down all the different possible orders for the output.
// Note that this includes output orders that you may not be able to reproduce.
//
// Ans. 10 diff possibilities
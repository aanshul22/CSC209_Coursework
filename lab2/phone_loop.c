#include<stdio.h>


int main() {
    char phone[11];
    int num;
    int error = 0;

    scanf("%s", phone);

    while (scanf("%d", &num) == 1) {

        if (num == -1) {
            printf("%s\n", phone);
        }
        else if (0 <= num && num <= 9) {
            printf("%c\n", phone[num]);
        }
        else {
            printf("ERROR\n");
            error = 1;
        }
    }

    return error;
}
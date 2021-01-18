#include<stdio.h>


int main() {
    char phone[11];
    int num;

    scanf("%s %d", phone, &num);

    if (num == -1) {
        printf("%s\n", phone);
    }
    else if (0 <= num && num <= 9) {
        printf("%c\n", phone[num]);
    }
    else {
        printf("ERROR\n");
        return 1;
    }

    return 0;
}
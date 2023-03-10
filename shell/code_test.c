#include <stdlib.h>
#include <stdio.h>

int main(void) {
    char *str = "1234%%%";
    printf("%d\n", atoi(str));
    return 0;
}
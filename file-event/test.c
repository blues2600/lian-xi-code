#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    char buf[] = "/user/bin/sw/hom/fd/f54/target";
    char limt[]="/";
    char result[100]={'\0'};

    char *token = strtok(buf, limt);
    if (token == NULL) {
        printf("strtok() failed. \n");
        return 0;
    }

    while (token != NULL) {
        strcpy(result, token);
        printf("%s\n", token);
        token = strtok(NULL, limt);
    }

    printf("resoult=%s\n", result);

    return 0;
}

#include <stdio.h>

// #define DISABLE_PRINTF

#ifdef DISABLE_PRINTF
    #define printf(fmt, ...) (0)
#endif

int main(){
    printf("hello world!\n");
    return 0;
}
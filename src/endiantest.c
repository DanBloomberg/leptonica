/*  endiantest.c  */

#include <stdio.h>

int main()
{
/* fprintf(stderr, "doing the test\n"); */
    long v = 0x04030201;
    if (*((unsigned char *)(&v)) == 0x04)
        printf("L_BIG_ENDIAN\n");
    else
        printf("L_LITTLE_ENDIAN\n");
    return 0;
}



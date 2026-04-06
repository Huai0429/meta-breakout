#include"utils.h"
int str_to_int(char *str)
{
    int len = strlen(str) - 1;
    int ret = 0, multiply = 1;
    for (int i = len; i >= 0; i--) {
        ret += ((str[i] - 48) * multiply);
        multiply *= 10;
        // printf("ascii = %d %c\n",(str[i] - 48),str[i]);
    }
    // printf("ret = %d\n",ret);
    return ret;
}
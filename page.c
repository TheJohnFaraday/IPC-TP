#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    printf("%d", getpagesize());
    return 0;
}

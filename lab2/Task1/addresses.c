#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int addr5;
int addr6;

int foo();
void point_at(void *p);
void foo1();
void foo2();
void increment();

int main(int argc, char **argv)
{
    int addr2;
    int addr3;
    char *yos = "ree";
    int *addr4 = (int *)(malloc(50));
    int iarray[] = {1, 2, 3};
    float farray[3];
    double darray[3];
    char carray[] = {'a', 'b', 'c'};
    int *iarrayPtr = &iarray[0];
    char *carrayPtr = &carray[0];

    printf("- &addr2: %p\n", &addr2);
    printf("- &addr3: %p\n", &addr3);
    printf("- foo: %p\n", foo);
    printf("- &addr5: %p\n", &addr5);

    point_at(&addr5);

    printf("- &addr6: %p\n", &addr6);
    printf("- yos: %p\n", yos);
    printf("- addr4: %p\n", addr4);
    printf("- &addr4: %p\n", &addr4);

    printf("- &foo1: %p\n", &foo1);
    printf("- &foo2: %p\n", &foo2);
    printf("- &foo2 - &foo1: %ld\n", &foo2 - &foo1);
    return 0;
}

int foo()
{
    return -1;
}

void point_at(void *p)
{
    int local;
    static int addr0 = 2;
    static int addr1;
    static int addr10 = 70;

    long dist1 = (size_t)&addr6 - (size_t)p;
    long dist2 = (size_t)&local - (size_t)p;
    long dist3 = (size_t)&foo - (size_t)p;

    printf("long size on this machine: %u bytes\n", sizeof(long));
    printf("dist1: (size_t)&addr6 - (size_t)p: %ld\n", dist1);
    printf("dist2: (size_t)&local - (size_t)p: %ld\n", dist2);
    printf("dist3: (size_t)&foo - (size_t)p:  %ld\n", dist3);

    printf("- addr0: %p\n", &addr0);
    printf("- addr1: %p\n", &addr1);
    printf("- addr10: %p\n", &addr10);
}

void foo1()
{
    printf("foo1\n");
}

void foo2()
{
    printf("foo2\n");
}

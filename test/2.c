#include <sys/mman.h>
#include <stdio.h>

typedef struct
{
    int a;
    int b;
} myStruct;

int main()
{
    myStruct * entry = (myStruct *)mmap(0, sizeof(myStruct),
            PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (entry == MAP_FAILED) {
        printf("Map failed.\n");
    }
    else {
        entry->a = 4;
        printf("Success: entry=%p, entry->a = %d\n", entry, entry->a);
    }
    return 0;
}
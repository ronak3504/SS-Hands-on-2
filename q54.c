#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

void show_limit(const char *name, int resource)
{
    struct rlimit limit;

    printf("\n%s\n", name);

    if (getrlimit(resource, &limit) == -1) {
        perror("getrlimit");
        return;
    }

    printf("Soft limit: ");

    if (limit.rlim_cur == RLIM_INFINITY)
        printf("Unlimited\n");
    else
        printf("%llu\n", (unsigned long long)limit.rlim_cur);

    printf("Hard limit: ");

    if (limit.rlim_max == RLIM_INFINITY)
        printf("Unlimited\n");
    else
        printf("%llu\n", (unsigned long long)limit.rlim_max);
}

int main(void)
{
    printf("System Resource Limits using getrlimit()\n");

    show_limit("Maximum open files:", RLIMIT_NOFILE);
    show_limit("Maximum stack size:", RLIMIT_STACK);
    show_limit("Maximum core file size:", RLIMIT_CORE);
    show_limit("Maximum CPU time:", RLIMIT_CPU);
    show_limit("Maximum process file size:", RLIMIT_FSIZE);

    return 0;
}

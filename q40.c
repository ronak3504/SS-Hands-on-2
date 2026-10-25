#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/resource.h>
#include <fcntl.h>

int main(void)
{
    struct rlimit limit;
    int fd[2];

    printf("=== Q40: Process File Limit and Pipe Size ===\n\n");

    /* Maximum number of open file descriptors */
    if (getrlimit(RLIMIT_NOFILE, &limit) == -1)
    {
        perror("getrlimit");
        return 1;
    }

    printf("1. Maximum Open File Descriptors\n");
    printf("---------------------------------\n");

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

    long open_max = sysconf(_SC_OPEN_MAX);

    if (open_max != -1)
        printf("sysconf(_SC_OPEN_MAX): %ld\n", open_max);

#ifdef OPEN_MAX
    printf("OPEN_MAX macro: %d\n", OPEN_MAX);
#endif

    /* Pipe limits and actual capacity */
    printf("\n2. Pipe Size and Limits\n");
    printf("-----------------------\n");

#ifdef PIPE_BUF
    printf("PIPE_BUF macro: %d bytes\n", PIPE_BUF);
#endif

    if (pipe(fd) == -1)
    {
        perror("pipe");
        return 1;
    }

#ifdef F_GETPIPE_SZ
    int pipe_size = fcntl(fd[0], F_GETPIPE_SZ);

    if (pipe_size == -1)
        perror("fcntl F_GETPIPE_SZ");
    else
        printf("Actual pipe capacity: %d bytes\n", pipe_size);
#else
    printf("F_GETPIPE_SZ is not available.\n");
#endif

    close(fd[0]);
    close(fd[1]);

    printf("\nProgram completed successfully.\n");

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

void display_limits(const struct rlimit *limit)
{
    printf("Soft limit: ");

    if (limit->rlim_cur == RLIM_INFINITY)
        printf("Unlimited\n");
    else
        printf("%llu\n", (unsigned long long)limit->rlim_cur);

    printf("Hard limit: ");

    if (limit->rlim_max == RLIM_INFINITY)
        printf("Unlimited\n");
    else
        printf("%llu\n", (unsigned long long)limit->rlim_max);
}

int main(void)
{
    struct rlimit limit;
    unsigned long long input;
    char path[PATH_MAX];
    int *fds = NULL;
    size_t opened = 0, capacity = 0;

    printf("=== setrlimit() Demonstration ===\n");
    printf("Process ID: %ld\n\n", (long)getpid());

    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        perror("getrlimit");
        return 1;
    }

    printf("Original open-file limits:\n");
    display_limits(&limit);

    printf("\nEnter the new soft limit: ");

    if (scanf("%llu", &input) != 1 ||
        input == 0 ||
        (rlim_t)input != input ||
        (limit.rlim_max != RLIM_INFINITY &&
         (rlim_t)input > limit.rlim_max)) {
        printf("Invalid soft limit.\n");
        return 1;
    }

    limit.rlim_cur = (rlim_t)input;

    if (setrlimit(RLIMIT_NOFILE, &limit) == -1) {
        perror("setrlimit");
        return 1;
    }

    printf("\nUpdated limits:\n");

    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        perror("getrlimit");
        return 1;
    }

    display_limits(&limit);

    printf("\nEnter the file path to open repeatedly: ");

    if (scanf("%4095s", path) != 1) {
        printf("Invalid file path.\n");
        return 1;
    }

    printf("\nOpening '%s' repeatedly...\n", path);

    while (1) {
        int fd = open(path, O_RDONLY);

        if (fd == -1) {
            if (errno == EMFILE) {
                printf("\nReached the process open-file limit (EMFILE).\n");
            } else {
                perror("open");
            }
            break;
        }

        if (opened == capacity) {
            size_t new_capacity = capacity == 0 ? 8 : capacity * 2;
            int *new_fds = realloc(fds, new_capacity * sizeof(*fds));

            if (new_fds == NULL) {
                perror("realloc");
                close(fd);
                break;
            }

            fds = new_fds;
            capacity = new_capacity;
        }

        fds[opened++] = fd;
    }

    printf("Successfully opened file descriptors: %zu\n", opened);

    for (size_t i = 0; i < opened; i++)
        close(fds[i]);

    free(fds);

    printf("All opened descriptors closed.\n");
    printf("Demonstration complete.\n");

    return 0;
}

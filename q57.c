#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <errno.h>
#include <string.h>

static void show_sysconf(const char *label, int name, const char *unit)
{
    errno = 0;
    long value = sysconf(name);

    printf("%-48s", label);

    if (value == -1) {
        if (errno != 0)
            printf("Error: %s\n", strerror(errno));
        else
            printf("Indeterminate or unsupported\n");
        return;
    }

    printf("%ld %s\n", value, unit);
}

static void show_open_file_limit(void)
{
    struct rlimit limit;

    printf("%-48s", "d) Maximum number of open files:");

    if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
        printf("Error: %s\n", strerror(errno));
        return;
    }

    if (limit.rlim_cur == RLIM_INFINITY)
        printf("Unlimited (soft limit)\n");
    else
        printf("%llu file descriptors (soft limit)\n",
               (unsigned long long)limit.rlim_cur);
}

int main(void)
{
    printf("====== SYSTEM LIMITATIONS ======\n\n");

    show_sysconf(
        "a) Maximum exec argument/environment size:",
        _SC_ARG_MAX, "bytes"
    );

    show_sysconf(
        "b) Maximum simultaneous processes per user ID:",
        _SC_CHILD_MAX, "processes"
    );

    show_sysconf(
        "c) Number of clock ticks per second:",
        _SC_CLK_TCK, "ticks/second"
    );

    show_open_file_limit();

    show_sysconf(
        "e) Size of a memory page:",
        _SC_PAGESIZE, "bytes"
    );

    show_sysconf(
        "f) Total number of physical memory pages:",
        _SC_PHYS_PAGES, "pages"
    );

    show_sysconf(
        "g) Currently available physical memory pages:",
        _SC_AVPHYS_PAGES, "pages"
    );

    printf("\n================================\n");

    return 0;
}

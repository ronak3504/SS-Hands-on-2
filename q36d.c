#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int main(void)
{
    const char *fifo_name = "myfifo";

    if (mknod(fifo_name, S_IFIFO | 0666, 0) == -1)
    {
        perror("mknod");
        return 1;
    }

    printf("FIFO '%s' created successfully using mknod().\n",
           fifo_name);

    struct stat st;

    if (lstat(fifo_name, &st) == -1)
    {
        perror("lstat");
        return 1;
    }

    if (S_ISFIFO(st.st_mode))
        printf("Verification successful: the entry is a FIFO.\n");
    else
        printf("Verification failed: the entry is not a FIFO.\n");

    return 0;
}

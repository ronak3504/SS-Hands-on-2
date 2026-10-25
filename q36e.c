#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(void)
{
    const char *fifo_name = "myfifo";

    if (mkfifo(fifo_name, 0666) == -1)
    {
        perror("mkfifo");
        return 1;
    }

    printf("FIFO '%s' created successfully using mkfifo().\n",
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
    {
        printf("Verification failed: the entry is not a FIFO.\n");
        return 1;
    }

    return 0;
}

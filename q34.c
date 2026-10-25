#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define USE_DUP   1
#define USE_DUP2  2
#define USE_FCNTL 3

#define MAX_METHODS 3

struct Result
{
    int method;
    int success;
    int lines;
    int words;
    int bytes;
};

/*
   Redirect source_fd to target_fd using the selected method.
*/
int redirect_fd(int source_fd, int target_fd, int method)
{
    int result;

    if (method == USE_DUP)
    {
        if (close(target_fd) == -1 && errno != EBADF)
            return -1;

        result = dup(source_fd);

        if (result == -1)
            return -1;

        if (result != target_fd)
        {
            close(result);
            errno = EBADF;
            return -1;
        }
    }
    else if (method == USE_DUP2)
    {
        result = dup2(source_fd, target_fd);

        if (result == -1)
            return -1;
    }
    else if (method == USE_FCNTL)
    {
        if (close(target_fd) == -1 && errno != EBADF)
            return -1;

        result = fcntl(source_fd, F_DUPFD, target_fd);

        if (result == -1)
            return -1;

        if (result != target_fd)
        {
            close(result);
            errno = EBADF;
            return -1;
        }
    }
    else
    {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

/*
   Execute the real pipeline:
   ls -l | wc

   Capture wc's output so the comparison option can display
   the actual line, word and byte counts.
*/
int run_pipeline(int method, struct Result *result)
{
    int pipefd[2];
    int output_pipe[2];
    pid_t ls_pid, wc_pid;
    int ls_status, wc_status;
    char output[256];
    size_t used = 0;

    result->method = method;
    result->success = 0;
    result->lines = 0;
    result->words = 0;
    result->bytes = 0;

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return -1;
    }

    if (pipe(output_pipe) == -1)
    {
        perror("output pipe");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    /* First child: ls -l */
    ls_pid = fork();

    if (ls_pid == -1)
    {
        perror("fork ls");
        close(pipefd[0]);
        close(pipefd[1]);
        close(output_pipe[0]);
        close(output_pipe[1]);
        return -1;
    }

    if (ls_pid == 0)
    {
        close(pipefd[0]);
        close(output_pipe[0]);
        close(output_pipe[1]);

        if (redirect_fd(pipefd[1], STDOUT_FILENO, method) == -1)
        {
            perror("redirect ls stdout");
            close(pipefd[1]);
            _exit(1);
        }

        close(pipefd[1]);

        execlp("ls", "ls", "-l", (char *)NULL);

        perror("execlp ls");
        _exit(127);
    }

    /* Second child: wc */
    wc_pid = fork();

    if (wc_pid == -1)
    {
        perror("fork wc");

        close(pipefd[0]);
        close(pipefd[1]);
        close(output_pipe[0]);
        close(output_pipe[1]);

        while (waitpid(ls_pid, NULL, 0) == -1 && errno == EINTR)
            ;

        return -1;
    }

    if (wc_pid == 0)
    {
        close(pipefd[1]);
        close(output_pipe[0]);

        if (redirect_fd(pipefd[0], STDIN_FILENO, method) == -1)
        {
            perror("redirect wc stdin");
            close(pipefd[0]);
            close(output_pipe[1]);
            _exit(1);
        }

        close(pipefd[0]);

        if (redirect_fd(output_pipe[1], STDOUT_FILENO, USE_DUP2) == -1)
        {
            perror("redirect wc stdout");
            close(output_pipe[1]);
            _exit(1);
        }

        close(output_pipe[1]);

        execlp("wc", "wc", (char *)NULL);

        perror("execlp wc");
        _exit(127);
    }

    /* Parent */
    close(pipefd[0]);
    close(pipefd[1]);
    close(output_pipe[1]);

    /* Read wc's output. */
    while (used < sizeof(output) - 1)
    {
        ssize_t n = read(output_pipe[0], output + used,
                         sizeof(output) - 1 - used);

        if (n == -1)
        {
            if (errno == EINTR)
                continue;

            perror("read output");
            close(output_pipe[0]);

            while (waitpid(ls_pid, NULL, 0) == -1 && errno == EINTR)
                ;
            while (waitpid(wc_pid, NULL, 0) == -1 && errno == EINTR)
                ;

            return -1;
        }

        if (n == 0)
            break;

        used += (size_t)n;
    }

    close(output_pipe[0]);
    output[used] = '\0';

    while (waitpid(ls_pid, &ls_status, 0) == -1)
    {
        if (errno == EINTR)
            continue;

        perror("waitpid ls");
        return -1;
    }

    while (waitpid(wc_pid, &wc_status, 0) == -1)
    {
        if (errno == EINTR)
            continue;

        perror("waitpid wc");
        return -1;
    }

    if (!WIFEXITED(ls_status) || WEXITSTATUS(ls_status) != 0 ||
        !WIFEXITED(wc_status) || WEXITSTATUS(wc_status) != 0)
    {
        fprintf(stderr, "A pipeline command failed.\n");
        return -1;
    }

    if (sscanf(output, "%d %d %d",
               &result->lines, &result->words, &result->bytes) != 3)
    {
        fprintf(stderr, "Could not parse wc output.\n");
        return -1;
    }

    result->success = 1;
    return 0;
}

const char *method_name(int method)
{
    if (method == USE_DUP)
        return "dup()";

    if (method == USE_DUP2)
        return "dup2()";

    return "fcntl(F_DUPFD)";
}

void execute_one(int method)
{
    struct Result result;

    printf("\nExecuting ls -l | wc using %s...\n\n",
           method_name(method));

    fflush(stdout);

    if (run_pipeline(method, &result) == 0)
    {
        printf("Lines: %d\n", result.lines);
        printf("Words: %d\n", result.words);
        printf("Bytes: %d\n", result.bytes);
        printf("\nPipeline executed successfully.\n");
    }
    else
    {
        printf("Pipeline execution failed.\n");
    }
}

void compare_all(void)
{
    struct Result results[MAX_METHODS];

    printf("\n========== COMPARISON ==========\n");

    for (int i = 0; i < MAX_METHODS; i++)
    {
        if (run_pipeline(i + 1, &results[i]) == -1)
        {
            printf("%s failed.\n", method_name(i + 1));
            results[i].success = 0;
        }
    }

    printf("\n%-20s %8s %8s %8s\n",
           "Method", "Lines", "Words", "Bytes");

    printf("------------------------------------------------\n");

    for (int i = 0; i < MAX_METHODS; i++)
    {
        if (results[i].success)
        {
            printf("%-20s %8d %8d %8d\n",
                   method_name(results[i].method),
                   results[i].lines,
                   results[i].words,
                   results[i].bytes);
        }
        else
        {
            printf("%-20s FAILED\n",
                   method_name(results[i].method));
        }
    }

    printf("\nAll successful results should match if the directory "
           "does not change during the comparison.\n");
}

void explain_redirection(void)
{
    printf("\n========== FILE DESCRIPTOR REDIRECTION ==========\n");

    printf("\nFile descriptor numbers:\n");
    printf("0 = Standard input  (stdin)\n");
    printf("1 = Standard output (stdout)\n");
    printf("2 = Standard error  (stderr)\n");

    printf("\nPipeline data flow:\n");
    printf("ls -l --stdout--> PIPE --stdin--> wc\n");

    printf("\nThe ls process redirects descriptor 1 to the pipe's "
           "write end.\n");

    printf("The wc process redirects descriptor 0 to the pipe's "
           "read end.\n");

    printf("\nMethods:\n");

    printf("1. dup(): closes the target descriptor and duplicates "
           "the source to the lowest available descriptor.\n");

    printf("2. dup2(): duplicates the source directly onto the "
           "specified target descriptor.\n");

    printf("3. fcntl(F_DUPFD): duplicates the descriptor using "
           "the specified minimum descriptor number.\n");

    printf("\nThe parent closes its unused pipe ends and waits "
           "for both child processes.\n");
}

int main(void)
{
    int choice;

    while (1)
    {
        printf("\n========== Q34: PIPELINE EXPLORER ==========\n");
        printf("1. Execute using dup()\n");
        printf("2. Execute using dup2()\n");
        printf("3. Execute using fcntl()\n");
        printf("4. Compare all three methods\n");
        printf("5. Explain file descriptor redirection\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");

        fflush(stdout);

        if (scanf("%d", &choice) != 1)
        {
            int ch;

            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            if (feof(stdin))
            {
                printf("\nExiting.\n");
                break;
            }

            printf("Invalid input. Enter a menu number.\n");
            continue;
        }

        switch (choice)
        {
            case 1:
                execute_one(USE_DUP);
                break;

            case 2:
                execute_one(USE_DUP2);
                break;

            case 3:
                execute_one(USE_FCNTL);
                break;

            case 4:
                compare_all();
                break;

            case 5:
                explain_redirection();
                break;

            case 0:
                printf("Exiting Q34 Pipeline Explorer. Goodbye!\n");
                return 0;

            default:
                printf("Invalid choice. Select 0 to 5.\n");
        }
    }

    return 0;
}

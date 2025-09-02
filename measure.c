/* measure.c – timing wrapper that leaves your original frpc.c untouched */
#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* ------------------------------------------------------------------ helpers */
static long nsdiff(struct timespec a, struct timespec b)
{
    return (b.tv_sec - a.tv_sec) * 1000000000L + (b.tv_nsec - a.tv_nsec);
}
static int tokcount(const char* s)
{
    int n = 0, in = 0;
    for (; *s; ++s)
    {
        if (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
            in = 0;
        else if (!in)
        {
            in = 1;
            ++n;
        }
    }
    return n;
}
static char* trimnl(char* s)
{
    char* p = strchr(s, '\n');
    if (p)
        *p = 0;
    return s;
}

/* ------------------------------------------------------------------ main */
int main(int argc, char* argv[])
{
    const char* exprFile = (argc == 3 && strcmp(argv[1], "-i") == 0) ? argv[2] : "expressions.txt";
    FILE* fin = fopen(exprFile, "r");
    if (!fin)
    {
        perror("expressions");
        return 1;
    }

    FILE* fr = fopen("results", "w");
    FILE* fe = fopen("errors", "w");
    if (!fr || !fe)
    {
        perror("results/errors");
        return 1;
    }

    char line[4096], out[512];

    while (fgets(line, sizeof(line), fin))
    {
        if (line[0] == '\n' || line[0] == '#')
            continue;
        int size = tokcount(line);

        /* temp input with this expression */
        char inTmp[] = "/tmp/inXXXXXX";
        int fdIn = mkstemp(inTmp);
        write(fdIn, line, strlen(line));
        close(fdIn);

        for (int mode = 1; mode <= 3; ++mode)
        {

            char outTmp[] = "/tmp/outXXXXXX";
            int fdOut = mkstemp(outTmp);
            close(fdOut);

            struct timespec t0, t1;
            clock_gettime(CLOCK_MONOTONIC, &t0);

            /* ------- run frpc -------- */
            if (mode == 1)
            {
                /* capture stdout with pipe */
                int pfd[2];
                pipe(pfd);
                pid_t pid = fork();
                if (pid == 0)
                { /* child */
                    dup2(open(inTmp, O_RDONLY), STDIN_FILENO);
                    dup2(pfd[1], STDOUT_FILENO);
                    close(pfd[0]);
                    close(pfd[1]);
                    execl("./frpc", "frpc", (char*)0);
                    _exit(127);
                }
                close(pfd[1]);
                FILE* fp = fdopen(pfd[0], "r");
                while (fgets(out, sizeof(out), fp)) /* skip echo */
                    if (!strncmp(out, "ANSWER", 6))
                        break;
                fclose(fp);
                waitpid(pid, NULL, 0);
            }
            else
            {
                pid_t pid = fork();
                if (pid == 0)
                {
                    execl("./frpc", "frpc", "-i", inTmp, "-o", outTmp, (char*)0);
                    _exit(127);
                }
                waitpid(pid, NULL, 0);
                FILE* fo = fopen(outTmp, "r");
                while (fo && fgets(out, sizeof(out), fo))
                    if (!strncmp(out, "ANSWER", 6))
                        break;
                if (fo)
                    fclose(fo);
            }

            clock_gettime(CLOCK_MONOTONIC, &t1);

            if (!strncmp(out, "ANSWER", 6))
                fprintf(fr, "%d|%d|%ld\n", mode, size, nsdiff(t0, t1));
            else
                fprintf(fe, "%d|%s|NO-ANSWER\n", mode, trimnl(line));

            unlink(outTmp);
        }
        unlink(inTmp);
    }
    fclose(fin);
    fclose(fr);
    fclose(fe);
    return 0;
}

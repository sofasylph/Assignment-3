#define _GNU_SOURCE             /* get dprintf(), M_PI, M_E, …            */
#define _POSIX_C_SOURCE 200809L /* get usleep(), kill() prototypes       */

#include <ctype.h>
#include <fcntl.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "commlib.h" /* instructor-supplied */

/* ───────────────────────  RPN stack & evaluator  ─────────────────────── */

#define MAX_STACK_CAPACITY 1000
static double stack[MAX_STACK_CAPACITY];
static int stack_index = 0;

static int g_errorFlag = 0;
static char g_errorMessage[256] = "";

/* simple helpers */
static void push(double v)
{
    if (stack_index < MAX_STACK_CAPACITY)
        stack[stack_index++] = v;
    else
    {
        g_errorFlag = 1;
        strcpy(g_errorMessage, "Stack overflow");
    }
}
static double pop(void)
{
    if (stack_index > 0)
        return stack[--stack_index];

    g_errorFlag = 1;
    strcpy(g_errorMessage, "Stack underflow");
    return 0.0;
}

/* factorial – for non-negative integer n ≤ 20 (fits in 64-bit double) */
static double factorial(double x)
{
    if (x < 0 || floor(x) != x)
    {
        g_errorFlag = 1;
        strcpy(g_errorMessage, "Bad argument for '!'");
        return 1;
    }
    double r = 1;
    for (int i = 2; i <= (int)x && i <= 20; ++i)
        r *= i;
    return r;
}

static void process_token(const char* tok)
{
    if (g_errorFlag)
        return;

    if (isdigit(tok[0]) || (tok[0] == '-' && isdigit(tok[1])))
    {
        push(strtod(tok, NULL));
    }
    else if (!strcmp(tok, "+"))
    {
        if (stack_index >= 2)
        {
            double b = pop(), a = pop();
            push(a + b);
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "+ needs 2");
        }
    }
    else if (!strcmp(tok, "-"))
    {
        if (stack_index >= 2)
        {
            double b = pop(), a = pop();
            push(a - b);
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "- needs 2");
        }
    }
    else if (!strcmp(tok, "*"))
    {
        if (stack_index >= 2)
        {
            double b = pop(), a = pop();
            push(a * b);
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "* needs 2");
        }
    }
    else if (!strcmp(tok, "/"))
    {
        if (stack_index >= 2)
        {
            double b = pop(), a = pop();
            if (b == 0)
            {
                g_errorFlag = 1;
                strcpy(g_errorMessage, "Division by zero");
            }
            else
                push(a / b);
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "/ needs 2");
        }
    }
    else if (!strcmp(tok, "sin"))
    {
        if (stack_index >= 1)
            push(sin(pop()));
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "sin needs 1");
        }
    }
    else if (!strcmp(tok, "cos"))
    {
        if (stack_index >= 1)
            push(cos(pop()));
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "cos needs 1");
        }
    }
    else if (!strcmp(tok, "tan"))
    {
        if (stack_index >= 1)
            push(tan(pop()));
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "tan needs 1");
        }
    }
    else if (!strcmp(tok, "exp"))
    {
        if (stack_index >= 1)
            push(exp(pop()));
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "exp needs 1");
        }
    }
    else if (!strcmp(tok, "ln"))
    {
        if (stack_index >= 1)
        {
            double a = pop();
            if (a <= 0)
            {
                g_errorFlag = 1;
                strcpy(g_errorMessage, "ln of non-positive");
            }
            else
                push(log(a));
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "ln needs 1");
        }
    }
    else if (!strcmp(tok, "log"))
    {
        if (stack_index >= 1)
        {
            double a = pop();
            if (a <= 0)
            {
                g_errorFlag = 1;
                strcpy(g_errorMessage, "log of non-positive");
            }
            else
                push(log10(a));
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "log needs 1");
        }
    }
    else if (!strcmp(tok, "pow"))
    { /* legacy token */
        if (stack_index >= 2)
        {
            double b = pop(), a = pop();
            push(pow(a, b));
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "pow needs 2");
        }

        /* ─────────── NEW ASSIGNMENT-3 TOKENS ─────────── */
    }
    else if (!strcmp(tok, "^"))
    { /* caret alias for pow */
        if (stack_index >= 2)
        {
            double b = pop(), a = pop();
            push(pow(a, b));
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "^ needs 2");
        }
    }
    else if (!strcmp(tok, "sqrt"))
    { /* square-root */
        if (stack_index >= 1)
        {
            double a = pop();
            if (a < 0)
            {
                g_errorFlag = 1;
                strcpy(g_errorMessage, "sqrt of negative");
            }
            else
                push(sqrt(a));
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "sqrt needs 1");
        }
    }
    else if (!strcmp(tok, "%"))
    { /* floating-point modulo */
        if (stack_index >= 2)
        {
            double b = pop(), a = pop();
            push(fmod(a, b));
        }
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "% needs 2");
        }
    }
    else if (!strcmp(tok, "!"))
    { /* factorial */
        if (stack_index >= 1)
            push(factorial(pop()));
        else
        {
            g_errorFlag = 1;
            strcpy(g_errorMessage, "! needs 1");
        }
    }
    else if (!strcmp(tok, "pi"))
    { /* constants */
        push(M_PI);
    }
    else if (!strcmp(tok, "e"))
    {
        push(M_E);
    }
    else
    { /* unknown token */
        g_errorFlag = 1;
        snprintf(g_errorMessage, sizeof(g_errorMessage), "Unknown command '%s'", tok);
    }
}

static void evaluate_line(const char* line, char* out, size_t outSz)
{
    stack_index = 0;
    g_errorFlag = 0;
    g_errorMessage[0] = '\0';

    char buf[1024];
    strncpy(buf, line, sizeof(buf));
    buf[sizeof(buf) - 1] = '\0';

    for (char* tok = strtok(buf, " \t\r\n"); tok; tok = strtok(NULL, " \t\r\n"))
        process_token(tok);

    if (g_errorFlag)
    {
        snprintf(out, outSz, "ERROR: %s\n", g_errorMessage);
        return;
    }
    if (stack_index == 0)
    {
        snprintf(out, outSz, "ERROR: No result\n");
        return;
    }

    snprintf(out, outSz, "ANSWER: %.3f\n", stack[stack_index - 1]);
}

/* ───────────────────────────  BACKEND & FRONTEND  ─────────────────────────── */

#define SENTINEL "__quit__"

static void backend_loop(void)
{
    if (backend_setup() < 0)
    {
        perror("backend_setup");
        exit(1);
    }

    char buf[2048], res[256];
    for (;;)
    {
        int n = backend_receive_from_frontend(buf, sizeof(buf) - 1);
        if (n <= 0)
            break;
        buf[n] = '\0';
        if (!strcmp(buf, SENTINEL))
            break;

        evaluate_line(buf, res, sizeof(res));
        backend_send_to_frontend(res, strlen(res));
    }
    backend_close();
    exit(0);
}

static void frontend_loop(int fd_in, int fd_out, pid_t back_pid)
{
    if (frontend_setup() < 0)
    {
        perror("frontend_setup");
        kill(back_pid, SIGKILL);
        exit(1);
    }

    const size_t B = 4096;
    char rBuf[B], line[2048], res[256];
    int lpos = 0;
    ssize_t nRead;

    while ((nRead = read(fd_in, rBuf, B)) > 0)
    {
        for (ssize_t i = 0; i < nRead; ++i)
        {
            char c = rBuf[i];
            if (c == '\n')
            {
                line[lpos] = '\0';

                dprintf(fd_out, "%s\n", line);                /* echo */
                frontend_send_to_backend(line, strlen(line)); /* ask */
                int n = frontend_receive_from_backend(res, sizeof(res) - 1);
                res[n] = '\0';
                write(fd_out, res, strlen(res)); /* reply */

                lpos = 0;
            }
            else if (lpos < (int)sizeof(line) - 1)
            {
                line[lpos++] = c;
            }
        }
    }
    if (lpos > 0)
    { /* last line (no NL) */
        line[lpos] = '\0';
        dprintf(fd_out, "%s\n", line);
        frontend_send_to_backend(line, strlen(line));
        int n = frontend_receive_from_backend(res, sizeof(res) - 1);
        res[n] = '\0';
        write(fd_out, res, strlen(res));
    }

    frontend_send_to_backend(SENTINEL, strlen(SENTINEL));
    frontend_close();
    waitpid(back_pid, NULL, 0); /* reap */
}

/* ──────────────────────────────────  MAIN  ────────────────────────────────── */

int main(int argc, char* argv[])
{
    int fd_in = STDIN_FILENO;
    int fd_out = STDOUT_FILENO;

    /* simple -i / -o parsing */
    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], "-i") && i + 1 < argc)
        {
            fd_in = open(argv[++i], O_RDONLY);
            if (fd_in < 0)
            {
                perror("open input");
                return 1;
            }
        }
        else if (!strcmp(argv[i], "-o") && i + 1 < argc)
        {
            fd_out = open(argv[++i], O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd_out < 0)
            {
                perror("open output");
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "Usage: %s [-i file] [-o file]\n", argv[0]);
            return 1;
        }
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    if (pid == 0)
        backend_loop(); /* child */

    usleep(100000); /* give backend a moment */
    frontend_loop(fd_in, fd_out, pid);

    if (fd_in != STDIN_FILENO)
        close(fd_in);
    if (fd_out != STDOUT_FILENO)
        close(fd_out);
    return 0;
}

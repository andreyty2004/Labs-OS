#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void at_exit()
{
    printf("[AT_EXIT] at_exit called for %d process\n", getpid());
}

void signal_handler(int sig)
{
    printf("[SIGNAL HANDLER] Singal %d received\n", sig);
}

void sigaction_handler(int sig, siginfo_t* sig_info, void* temp)
{
    printf("[SIGACTION HANDLER] INFO:\n"
        "Signal number: %d\n Signal code: %d\n"
        "Sending pid: %d\n Sending uid: %d\n"
        "Exit value: %d\n User time: %ld\n",
        sig_info->si_signo, sig_info->si_code, 
        sig_info->si_pid, sig_info->si_uid,
        sig_info->si_status, sig_info->si_utime);
}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    at_exit(at_exit);
    signal(SIGINT, signal_handler);

    struct sigaction act;
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = sigaction_handler;
    sigaction(SIGTERM, &act, NULL);

    int res = 0;
    switch(res = fork()) 
    {
        case -1:
                fprintf(stderr, "Fork error: %s (%d)\n", strerror(errno), errno); /* произошла ошибка */
                break;
        case 0:
                printf("[CHILD] I'm child of %d, my pid is %d\n", getppid(), getpid());
                break;
        default:
                int _res;
                wait(&_res);
                while(1){};
                printf("[PARENT] I'm parent of %d, my pid is %d, my parent pid is %d\n", res, getpid(), getppid());
                printf("[PARENT] Child exit code %d\n", res);
                break;
    }
    return 0;
}
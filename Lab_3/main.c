#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void at_exit_handler()
{
    printf("[AT_EXIT] atexit called for %d process\n", getpid());
}

void signal_handler(int sig)
{
    printf("[SIGNAL HANDLER] Singal %d received\n", sig);
}

void sigaction_handler(int sig, siginfo_t* sig_info, void* temp)
{
    printf("[SIGACTION HANDLER] INFO:\n"
        "Signal number: %d\nSignal code: %d\n"
        "Sending pid: %d\nSending uid: %d\n"
        "Exit value: %d\n",
        sig_info->si_signo, sig_info->si_code, 
        sig_info->si_pid, sig_info->si_uid,
        sig_info->si_status);
}

int main()
{
    atexit(at_exit_handler); 
    signal(SIGINT, signal_handler); 

    struct sigaction act; 
    act.sa_flags = SA_SIGINFO; 
    act.sa_sigaction = sigaction_handler; 
    sigaction(SIGTERM, &act, NULL);

    pid_t res = 0;
    switch(res = fork()) 
    {
        case -1:
                fprintf(stderr, "Fork error: %s (%d)\n", strerror(errno), errno); 
                break;
        case 0:
                printf("[CHILD] I'm child of %d, my pid is %d\n", getppid(), getpid());
                break;
        default:
                pid_t _res;
                wait(&_res);
                while(1){};
                printf("[PARENT] I'm parent of %d, my pid is %d, my parent pid is %d\n", res, getpid(), getppid());
                printf("[PARENT] Child exit code %d\n", res);
                break;
    }
    return 0;
}
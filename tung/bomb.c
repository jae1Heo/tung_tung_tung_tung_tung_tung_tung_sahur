#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define NUM_PROCESS 256
int main(int argc, char* argv[]){
    
    if(argc != 2) {
        perror("Usage: bomb <name of process>\n"); 
        exit(1);
    }

    pid_t pids[NUM_PROCESS]; // array which holds process ids 
    for(int i = 0; i < NUM_PROCESS; i++) { 
        pid_t pid = fork(); // create child process
        if(pid < 0) {  // if fork() failed
            perror("fork() failed\n");
            exit(-1); // terminate program
        }
        else if(pid == 0) { // if it is child process
            execl("./out", "./out", NULL); // execute tung generator
            perror("execl() failed"); // if failed, terminate program
            exit(1);  
        }
        else { // if it is parent process
            pids[i] = pid; // store child process's pid into array
        }
        sleep(3); // wait for 3 sec
    }


    for(int i = 0; i < NUM_PROCESS; i++) {
        waitpid(pids[i], NULL, 0); // blocks the parent process until child processes sends signal
    }
    return 0;
}

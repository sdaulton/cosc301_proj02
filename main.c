/*
    Sam Daulton, Bria Vicenti
    16 Oct 2014
    COSC 301 Project 2

    Stage 1:
    Bria wrote the code to tokenize commands, store them in a linked list data structure
    and free the appropriate memory when finished.  
    Sam wrote the code to run those commands in both parallel and sequential modes 
    and create and clean up after the appropriate child processes.

    Stage 2:
    Bria wrote the code to support PATH variable.
    Sam wrote the code to support background processes.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include "list.h" // Linked list of strings functionality from Bria Vicenti's project 1, and linked list of pids (Sam Daulton)

char** tokenify(const char *s, int indicator) {
    // Taken from lab 2, code by Bria Vicenti
    // Will return an array of char pointers (strings) or an array with one char pointer pointing to NULL if no tokens are present. 
    char *temp = strdup(s);
    char* delim = NULL;

    // Two delim cases to break it up into command sets / individual tokens.
    if (indicator == 0) {
    	delim = "\n \t";
    }
    else if (indicator == 1) {
    	delim = ";";
    }

    if (NULL != strtok(temp, delim)) {
    	char *temp2 = strdup(s);
    	int count = 1;

        while (NULL != strtok(NULL, delim)) {
            count++;   
        }

        char** finished = malloc((count+1) * sizeof(char*)); // Account for final null slot
        char *next = strtok(temp2, delim);
        int i = 0;
        while (next != NULL) {
            finished[i] = strdup(next);
            next = strtok(NULL, delim);
            i++;
        }
        
        finished[count] = NULL;
        free(temp);
        free(temp2);
        return finished;
    }
    else {
        char** finished = malloc(sizeof(char *));
        char *next = NULL;
        finished[0] = next;
        free(temp);
        return finished;
    }
}

void free_tokens(char **tokens, int len) {
    // Taken from Lab 2, code by Professor Sommers.
    // Modified to suit our needs. If len is passed in as -1, it means
    // we don't know the len. Len is used for when the cl_creator function
    // has an array where some slots are filled with NULL.
    int i = 0;

    if (len > 0) {
    	while (i < len) {
        	free(tokens[i]); // free each string
       		i++;
    	}
    	free(tokens); // then free the array
    }
    else {
    	while (tokens[i] != NULL) {
    		free(tokens[i]);
    		i++;
    	}
    	free(tokens);
    }
}

// To free the CL.
void free_cl(char ***cl) {
	int i = 0;
	while (cl[i] != NULL) {
		free_tokens(cl[i], -1);
		i++;
	}
	free(cl);
	return;
}

// Code by Professor Sommers, only here now for testing purposes.
void print_tokens(char *tokens[]) {
    int i = 0;
    while (tokens[i] != NULL) {
        printf("Token #%d:%s \n", i, tokens[i]);
        i++;
    }
    return;
}

void comment_handler(char* buffer) {
	// Checks for a comment in the command line input and handles it in place.
	for (int i = 0; i < strlen(buffer); i++) {
		if (buffer[i] == '#') {
			buffer[i] = '\0';
			return;
		}
	}
	return;
}

void print_prompt() {
    // prints the command line prompt
    printf("$: ");
    fflush(stdout);
}

char*** cl_creator(char* buffer) {
	// "Command list creator"
	// Creates an array of arrays of strings to handle multiple commands.
	// Feel free to clean! Not the neatest job handling extraneous issues
	// but it works.
	comment_handler(buffer);
	char **untoked_cl = tokenify(buffer, 1);
	int i = 0, orig_len = 0, command_count = 2; // Accounts for sandwiching the semicolon & NULL term
	while (untoked_cl[i] != NULL) {
		orig_len++;
        bool allspace = true;
        // check if command i is just whitespace
		for (int n = 0; n < strlen(untoked_cl[i]); n++) {
			if (!isspace(untoked_cl[i][n])) {
				allspace = false;
				break;
			}
		}
                
                // command i is not just whitespace
		if (!allspace) {
			command_count++;
		}
                //command i is just whitespace
		else { // Necessary because free() on a NULL pointer does nothing
			   // so this step allows this block to be freed properly.
			char *p = NULL; 
			char *x = untoked_cl[i];
			untoked_cl[i] = p;
			free(x);
		}
		i++;
	}

	char ***cl = malloc(command_count * sizeof(char**));

	int j = 0, k = 0; // j = tracker for untoked, k = write index
	while (j < orig_len) {
		if (untoked_cl[j] == NULL) {
			j++;
			continue;
		}
		cl[k] = tokenify(untoked_cl[j], 0);
		j++;
		k++;
	}
	
	cl[k] = NULL;
	free_tokens(untoked_cl, orig_len);
	return cl;

}

struct node* path_list_creater(FILE *input) {
    // Some code borrowed from Bria's proj01 for processing data.
    struct node *list = NULL;
    char line[128] = {'\0'};
    while (fgets(line, 128, input) != NULL) {
        line[strlen(line) - 1] = '\0'; // Clears the newline char.
        int check = list_append(line, &list);
        if (check < 0) {
            return NULL;
        }
    }
    return list;
}

// Finds, completes, and returns the full path or copies & returns the command
// if it either already works or doesn't exist. Returns a pointer to a new string regardless.
char* path_finder(char *cmd, struct node *list) {
    struct stat statresult;
    int rv = stat(cmd, &statresult);
    if (rv < 0) {
        // command does not include valid path in itself (e.g. cmd != /bin/ls)
        while (list != NULL) {
            char* test = malloc(strlen(cmd) + strlen(list -> path) + 2);
            strcpy(test, list-> path);
            strcat(test, "/");
            strcat(test, cmd);
            rv = stat(test, &statresult);
            if (rv >= 0) {
                // found a valid path for the command
                return test;
            }
            list = list -> next;
            free(test);
        }
        // no path found, return a copy of the command
    }
    // either no path found or command includes a valid path itself
    char* test = strdup(cmd);
    return test;
}

int mode_cmd(char **cmd, int *p_mode) {
    //Executes the mode command in the current process
    //Either returns the mode for the next input line or prints out the current mode, depending on the arguments passed
    if (cmd[1] != NULL) {
        if ((strcasecmp(cmd[1], "s") == 0) || (strcasecmp(cmd[1], "sequential") == 0)) {
            // set mode to sequential
            return 0;
        } else if ((strcasecmp(cmd[1], "p") == 0) || (strcasecmp(cmd[1], "parallel") == 0)) {
            //set mode to parallel
            return 1;
        }
    }
    // no valid arguments --> print current mode
    if (*p_mode == 0) {
        printf("Current Execution Mode: Sequential\n");
    } else {
        printf("Current Execution Mode: Parallel\n");
    }
    return *p_mode;
}

int run_cmds(char ***cl, int *p_mode, struct node *path_list, struct pid_node **process_list) {
    // Runs the commands from the input line
    // For system commands, this method forks the current process and runs the specified command in the child process
    // It then exits the child process
    // For built-in shell commands, this method runs the command in the main shell process
    // If the shell is in sequential mode, the parent waits for the child to finish
    // If the shell is in parallel, no waits are used (i.e. as soon as the parent is run, another fork can be called if the
    // next command is a system call)
    // Adds new forked processes to the process_list
    // returns 1 if the shell should exit after executing the commands (i.e. a valid exit command was encountered)
    // returns 0 otherwise
    // Loosely uses some code from Lab 3 (Sam Daulton)
    char **cmd = NULL;
    int status = 0;
    int i = 0;
    int new_mode = *p_mode;
    int ret_val = 0;
    int num_children = 0;
    while (cl[i] != NULL) {
        cmd = cl[i];
        if (strcasecmp(cmd[0],"mode") == 0) {
            // MODE
            new_mode = mode_cmd(cmd, p_mode);
            if (*process_list != NULL) {
                printf("Other processes are currently running.  Cannot change the mode.\n");
                new_mode = *p_mode;
            }
        } else if (strcasecmp(cmd[0], "exit") == 0) {
            if (cmd[1] == NULL) {
                // EXIT
                if (*process_list != NULL) {
                    printf("Other processes are currently running.  Cannot exit terminal.\n");
                } else {
                    ret_val = 1;
                }
            } else {
                fprintf(stderr, "Invalid argument for exit command\n");
            }
        } else if (strcasecmp(cmd[0], "jobs") == 0) {
            // JOBS
            if (cmd[1] == NULL) {
                // jobs
                if (*process_list == NULL) {
                    printf("There are no processes currently running.\n");
                } else {
                    pid_list_print(process_list);
                }
            } else {
                fprintf(stderr, "Invalid argument for jobs command\n");
            }
        } else if (strcasecmp(cmd[0], "pause") == 0) {
            // PAUSE
            if (cmd[1] == NULL) {
                fprintf(stderr, "Need PID as argument for pause command\n");
            } else if (*process_list == NULL) {
                printf("There are no processes currently.  Cannot pause\n");
            } else if (cmd[2] != NULL) {
                fprintf(stderr, "Only one argument (PID) accepted by pause command\n");
            } else {
                //try to pause
                if (pid_list_update(atoi(cmd[1]), process_list, 1) == -1) {
                    printf("Process %d does not exist\n", atoi(cmd[1]));
                } else {
                    if ((kill(atoi(cmd[1]), SIGSTOP)) == -1) {
                        fprintf(stderr, "kill failed on process: %s Reason %s\n", cmd[1], strerror(errno));
                    }
                }
            }
        } else if (strcasecmp(cmd[0], "resume") == 0) {
            // RESUME
            if (cmd[1] == NULL) {
                fprintf(stderr, "Need PID as argument for resume command\n");
            } else if (*process_list == NULL) {
                printf("There are no processes currently.  Cannot resume\n");
            } else if (cmd[2] != NULL) {
                fprintf(stderr, "Only one argument (PID) accepted by the resume command\n");
            } else {
                //try to resume
                if (pid_list_update(atoi(cmd[1]), process_list, 2) == -1) {
                    printf("Process %d does not exist\n", atoi(cmd[1]));
                } else {
                    if ((kill(atoi(cmd[1]), SIGCONT)) == -1) {
                        fprintf(stderr, "kill failed on process: %s Reason %s\n", cmd[1], strerror(errno));
                    }
                }
            }
        } else {
            // System command
            char* temp = path_finder(cmd[0], path_list);
            char* x = cmd[0];
            cmd[0] = temp;
            free(x);

            num_children++;
            pid_t pid = fork();
            if (pid == 0) {
                // child process running
                if (execv(cmd[0], cmd) < 0) {
                    //execv failed, print error and exit
                    if(*p_mode == 1) {
                        printf("\n");
                    }
                    fprintf(stderr, "execv failed on command: %s\nReason: %s\n", cmd[0], strerror(errno));
                    if(*p_mode == 1) {
                        print_prompt();
                    }
                    exit(1);
                }
            } else if (pid == -1) {
                // fork failed
                if(*p_mode == 1) {
                    printf("\n");
                }
                fprintf(stderr, "fork failed. command: %s not executed.\nReason: %s\n", cmd[0], strerror(errno));
                num_children--;
            } else if (*p_mode == 0) {
                // sequential mode: wait for child to finish
                pid_t wait_rv = wait(&status);
                if (wait_rv == -1) {
                    fprintf(stderr, "wait failed on command: %s\nReason: %s\n", cmd[0], strerror(errno));
                }
            } else {
                // parallel mode:
                pid_list_append(pid, cmd[0], process_list);
            }
        }
        i++;
    }
    /*
    // Used in Stage 1
    if (*p_mode == 1){
        // parallel mode
        // wait for all children to finish executing before printing shell prompt
        int j = 0;
        while (j < num_children) {
            wait(&status);
            j++;
        }
    }
    */

    // update mode for next input line
    // Note: new_mode is only different from *p_mode if valid mode command was entered
    // here the command is valid only if the process list is empty
    *p_mode = new_mode;
    return ret_val;
}

int main(int argc, char **argv) {
	
    int mode  = 0; // int indicating the current mode: 0 means sequential, 1 means parallel
    int will_exit = 0;  // int indicating if the shell should exit 0 means don't exit, 1 means exit
    printf("Welcome to the terminal! \n");
    char ***cl = NULL;
    char buffer[1024];

    // create list of paths to check for each system command
    struct node *path_list = NULL;
    FILE *datafile = fopen("shell-config", "r");
    if (datafile == NULL) {
        printf("Error: Cannot find shell-config. Please use full path names only!\n");
    } else {
        path_list = path_list_creater(datafile);
        if(fclose(datafile) != 0) {
            fprintf(stderr, "Could not close shell-config file.  Reason: %s\n", strerror(errno));
        }
    }
    struct pid_node *process_list = NULL; // linked list storing info about current processes

    // print shell prompt
    print_prompt();

    // setup for checking process status
    int status = 0;
    pid_t pid = 0;
    
    // setup for polling stdin
    struct pollfd pfd[1];
    pfd[0].fd = 0;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
    int poll_rv = 0;

    
    // while not EOF, continue running shell
    while (fgets(buffer, 1024, stdin) != NULL) {
        cl = cl_creator(buffer);
        will_exit = run_cmds(cl, &mode, path_list, &process_list);
        free_cl(cl);
        if (will_exit == 1) {
            break;
        }

        print_prompt();
        // Note: process_list is NULL unless mode = parallel
        // and there is a current child process (i.e. system command)
        while (process_list != NULL) {
            for (int i = 0; i < 10; i++) {
                pid = waitpid(-1, &status, WNOHANG);
                if (pid == -1) {
                    //waitpid failed
                    fprintf(stderr, "\nwaitpid failed\nReason: %s\n", strerror(errno));
                } else if (pid > 0) {
                    // waitpid returned a pid
                    // process pid's state has changed
                    printf("\n");
                    if (WIFCONTINUED(status)) {
                        // process resumed
                        pid_list_update(pid, &process_list, 2);
                    } else if (WIFSTOPPED(status)) {
                        // process paused
                        pid_list_update(pid, &process_list, 1);
                    }
                    // process completed
                    pid_list_update(pid, &process_list, 0);
                    print_prompt();
                    if (process_list == NULL) {    
                        break;
                    }
                }
            }
            // poll for input on stdin
            poll_rv = poll(&pfd[0], 1, 1000);
            if (poll_rv < 0) {
                fprintf(stderr, "\npoll failed\nReason: %s\n", strerror(errno));
            } else if (poll_rv > 0) {
                // user typed something on stdin
                if (fgets(buffer, 1024, stdin) != NULL) {
                    // not EOF
                    cl = cl_creator(buffer);
                    run_cmds(cl, &mode, path_list, &process_list);
                    free_cl(cl);
                } else {
                    // user typed EOF
                    printf("\nOther processes are currently running.  Cannot exit terminal.\n");
                }
                print_prompt(); 
            }

        }
        
    }
    if (will_exit != 1) {
        printf("\n"); // just to make smooth transition out of our shell if EOF
    }
    if (path_list != NULL) {
        list_clear(path_list);
    }
    return 0;
}


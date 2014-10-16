#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include <string.h>
#include <stdbool.h>

/* your list function definitions */

int list_append(const char *path, struct node **head) {
    // Returns -1 on failure.
    struct node *temp = malloc(sizeof(struct node));
    if (temp == NULL) {
    	printf("Error: Malloc failed!");
    	return -1;
    }
    strcpy(temp -> path, path);
    temp -> next = NULL;
    if (*head != NULL) {
        struct node *list = *head;
        while (list -> next != NULL) {
            list = list -> next;
        }
        list -> next = temp;
    }
    else {
        *head = temp;
    }
    return 0;
}

void list_print(struct node *head) {
	// Original code by Professor Sommers, modified by Bria.
	printf("***List contents begin***\n");
	while (head != NULL) {
		printf("%s\n", head -> path);
		head = head -> next;
	}
	printf("***List contents end***\n");
}

void list_clear(struct node *list) {
	// Code by Professor Sommers from HW3
    while (list != NULL) {
        struct node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

int pid_list_append(const pid_t cpid, const char *cmd, struct pid_node **head) {
    // Returns -1 on failure.
    struct pid_node *temp = malloc(sizeof(struct pid_node));
    if (temp == NULL) {
    	printf("Error: Malloc failed!");
    	return -1;
    }
    temp -> pid = cpid;
    strncpy(temp->state, "Running", 8);
    strncpy(temp->cmd, cmd, 128);
    temp -> next = NULL;
    if (*head != NULL) {
        struct pid_node *list = *head;
        while (list -> next != NULL) {
            list = list -> next;
        }
        list -> next = temp;
    }
    else {
        *head = temp;
    }
    return 0;
}

void pid_list_print(struct pid_node **head) {
    	/*
        int status = 0;
        bool terminated = false;
        pid_t wait_rv = 0;
        */
        struct pid_node *next_node = *head;
        /*
        del_list[num_pids] = 5 // number of pids that can be stored in initial array
	pid_t *del_list = malloc((num_pids + 1) * sizeof(pid_t)); // an array with the pids of 
	pid_t *new_del_list = NULL; // just in case array needs resizing
	del_list[0] = 0; // storing actual number of pids in array in del_pids[0]
	int i = 1;
	if (i >= num_pids + 1) {
    	// need to resize array
       	// create new array that can store twice as many tokens
        new_del_list = malloc((2 * num_pids + 1) * sizeof(pid_t));
        for (int j = 0; j < num_tokens+1; j++) {
            new_del_list[j] = del_list[j];
        }
        free(del_list);
        del_list = new_del_list;
        num_pids *= 2;
	}
        */
	
	printf("***Current Processes start***\n");
	while (next_node != NULL) {
		/*
                wait_rv = waitpid(next_node->pid, &status, WUNTRACED | WCONTINUED | WNOHANG);
		if (wait_rv == -1) {
		    // error in waitpid
		    fprintf(stderr, "Error calling waitpid on process with pid: %d\n", next_node->pid);
		} else if (wait_rv > 0){
                    //process was terminated
                    //should we worry about processes not terminating cleanly?
                    terminated = true;
		} else {
                    if (WIFCONTINUED(status)) {
                        strncpy(next_node->state, "Running", 8);
                    } else if (WIFSTOPPED(status)) {
                        strncpy(next_node->state, "Paused", 8);
                    }
                    printf("Process: %d\tCommand: %s\tStatus: %s\n", next_node->pid, next_node->cmd, next_node->state);
                }
                */
                printf("Process: %d\tCommand: %s\tStatus: %s\n", next_node->pid, next_node->cmd, next_node->state);
		next_node = next_node -> next;
                /*
                if (terminated) {
                    //delete pid from list
                    pid_list_update(wait_rv, head, 0);
                }
                terminated = false;
                */
	}
	printf("***Current Processes end***\n");
}

void pid_list_clear(struct pid_node *list) {
	// Code by Professor Sommers from HW3, modified by Sam
    while (list != NULL) {
        struct pid_node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

int pid_list_update(pid_t search_pid, struct pid_node **head, int update_code) {
    struct pid_node *next_node = *head;
    struct pid_node *current = *head;
    int is_head = 1;  //indicates if next_node the head of the list
    if (current != NULL) {
        // make sure the list is not empty
        next_node = current;
        is_head = 1;
        while (next_node != NULL) {
            if (next_node->pid == search_pid) {
                // next_node pid is the pid we want to delete
                if (update_code == 0) {
                    // DELETE this pid
                    if (is_head) {
                	*head = next_node->next;
                    } else {
		        current->next = next_node->next;
                    }
		    printf("Process %d (%s) completed\n", next_node->pid, next_node->cmd);
                    free(next_node);
		    return 0;
                } else if (update_code == 1) {
                    // PAUSE
                    strncpy(next_node->state, "Paused", 8);
                    return 0;
                } else if (update_code == 2) {
                    strncpy(next_node->state, "Running", 8);
                    return 0;
                }


            }
           
            next_node = next_node->next;
            if (is_head) {
                is_head = 0;
            } else {
                current = current->next;
            }
        }
    }
    return -1;
}

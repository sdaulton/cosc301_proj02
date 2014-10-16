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
    // takes a pid_t pid, string command name, and pointer to a pointer to a
    // linked list of struct pid_nodes
    // appends a new struct pid_node with pid set to cpid
    // and command set to cmd to the end of the list
    // Returns -1 on failure, 0 on success
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
    // takes a pointer to a ponter to the head of a linked list of struct pid_nodes
    // and prints the pid, command name, and status of that process
    struct pid_node *next_node = *head;
    printf("***Current Processes start***\n");
    while (next_node != NULL) {
        printf("Process: %d\tCommand: %s\tStatus: %s\n", next_node->pid, next_node->cmd, next_node->state);
        next_node = next_node -> next;
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
    // Takes the pid of the process to update, a pointer to a pointer to the head of linked list
    // of struct pid_nodes, and an update code as parameters
    // and performs the update specified by the update code.
    // Update codes: 0 --> pid terminated (delete from list)
    //               1 --> pid paused (change status to paused)
    //               2 --> pid resumed (change status to running)
    // returns 0 if the process with pid was in the list and was updated
    // returns -1 if the process with pid was not in the list
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

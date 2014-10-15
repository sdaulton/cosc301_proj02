#include <stdio.h>
#include <stdlib.h>
#include "list.h"

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

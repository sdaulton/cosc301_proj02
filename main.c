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

// SAM - If you have a better / more efficient / less ugly
// tokenify please feel free to replace this one!
char** tokenify(const char *s, int indicator) {
    // Taken from lab 2, code by Bria Vicenti
    // Will return a char array with one NULL element if no tokens are present. 
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
        char *p = strtok(temp2, delim);
        char *p2 = strdup(p);
        finished[0] = p2;

        for (int i = 1; i < count; i++) {
            p = strtok(NULL, delim);
            p2 = strdup(p);
            finished[i] = p2;
        }

        finished[count] = NULL;
        free(temp);
        free(temp2);
        return finished;
    }
    else {
        char** finished = malloc(sizeof(char*));
        char *p = NULL;
        finished[0] = p;
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
	free(cl[i]);
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
		for (int n = 0; n < strlen(untoked_cl[i]); n++) {
			if (!isspace(untoked_cl[i][n])) {
				allspace = false;
				break;
			}
		}
		if (!allspace) {
			command_count++;
		}
		else { // Is this really necessary?
			char *p = NULL; 
			char *x = NULL;
			x = untoked_cl[i];
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
		char **temp = tokenify(untoked_cl[j], 0);
		cl[k] = temp;
		j++;
		k++;
	}
	
	cl[k] = NULL;
	free_tokens(untoked_cl, orig_len);
	return cl;

}

int main(int argc, char **argv) {
	
	/*
	bool over = false; // Checks whether the shell is ready to finish running.
    printf("Welcome to the terminal! \n");
    while (!over) {
    	printf("$: ");
    	fflush(stdout);
    	char buffer[1024];
    	while (fgets(buffer, 1024, stdin) != NULL) {
    		char ***cl = NULL;
    		cl = cl_creator(buffer);
    		free_cl(cl);	
    	}
    }

    return 0;
    */

    
    char test[] = " /path; /path -o -o; ; ; /path2 -l -r; ;/path";
    char ***cl = NULL;
    cl = cl_creator(test);
    int i = 0;
    while (cl[i] != NULL) {
    	printf("This is argument %d! \n", i);
    	print_tokens(cl[i]);
    	i++;
    }

    free_cl(cl);
}


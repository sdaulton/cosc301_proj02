#ifndef __LIST_H__
#define __LIST_H__


struct node {
	char path[128];
	struct node *next;
};

int list_append(const char *path, struct node **head);
void list_print(struct node *head);
void list_clear(struct node *list);

#endif // __LIST_H__

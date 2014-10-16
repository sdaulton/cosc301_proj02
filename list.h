#ifndef __LIST_H__
#define __LIST_H__


struct node {
    char path[128];
    struct node *next;
};

struct pid_node {
    int pid;
    char state[8];
    char cmd[128];
    struct pid_node *next;
};

int list_append(const char *path, struct node **head);
void list_print(struct node *head);
void list_clear(struct node *list);

int pid_list_append(const pid_t cpid, const char *cmd, struct pid_node **head);
void pid_list_print(struct pid_node **head);
void pid_list_clear(struct pid_node *list);
int pid_list_update(const pid_t del_pid, struct pid_node **head, int update_code);


#endif // __LIST_H__

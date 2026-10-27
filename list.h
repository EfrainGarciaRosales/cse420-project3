#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct Node {
    long long val;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int size;
    int max_k;
} TopKList;

/* Initialize a top-K list */
static inline void list_init(TopKList *list, int k) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    list->max_k = k;
}

/* Insert value maintaining strictly decreasing order and capacity <= K */
static inline void list_insert(TopKList *list, long long val) {
    if (list->max_k <= 0) return;

    /* Fast discard: if full and val <= minimum in list */
    if (list->size == list->max_k && list->tail != NULL && val <= list->tail->val) {
        return;
    }

    Node *prev = NULL;
    Node *curr = list->head;

    while (curr != NULL && curr->val > val) {
        prev = curr;
        curr = curr->next;
    }

    /* Duplicate check */
    if (curr != NULL && curr->val == val) {
        return;
    }

    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_node->val = val;
    new_node->next = curr;

    if (prev == NULL) {
        list->head = new_node;
    } else {
        prev->next = new_node;
    }

    /* Update tail pointer if inserted at end */
    if (curr == NULL) {
        list->tail = new_node;
    }

    list->size++;

    /* If size exceeds K, trim the tail */
    if (list->size > list->max_k) {
        Node *p = list->head;
        while (p->next != list->tail) {
            p = p->next;
        }
        free(list->tail);
        p->next = NULL;
        list->tail = p;
        list->size--;
    } else if (list->size == 1) {
        list->tail = new_node;
    }
}

/* Free all nodes in the list */
static inline void list_free(TopKList *list) {
    Node *curr = list->head;
    while (curr != NULL) {
        Node *tmp = curr;
        curr = curr->next;
        free(tmp);
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/* Write list contents to output file */
static inline void list_write(const TopKList *list, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        perror("fopen output");
        exit(EXIT_FAILURE);
    }
    Node *curr = list->head;
    while (curr != NULL) {
        fprintf(fp, "%lld\n", curr->val);
        curr = curr->next;
    }
    fclose(fp);
}

/* Scan directory for eligible regular files */
static inline char **get_eligible_files(const char *dir_path, int *file_count) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int capacity = 16;
    int count = 0;
    char **files = (char **)malloc(capacity * sizeof(char *));

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        /* Ignore files starting with '.' */
        if (entry->d_name[0] == '.') continue;

        /* Ignore files ending with '~' */
        size_t len = strlen(entry->d_name);
        if (len > 0 && entry->d_name[len - 1] == '~') continue;

        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {
            if (count >= capacity) {
                capacity *= 2;
                files = (char **)realloc(files, capacity * sizeof(char *));
            }
            files[count] = strdup(fullpath);
            count++;
        }
    }
    closedir(dir);
    *file_count = count;
    return files;
}

static inline void free_file_list(char **files, int count) {
    for (int i = 0; i < count; i++) {
        free(files[i]);
    }
    free(files);
}

#endif

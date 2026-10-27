#include <pthread.h>
#include "list.h"

typedef struct {
    char *filepath;
    int k;
    TopKList *global_list;
    pthread_mutex_t *mutex;
} ThreadArg;

void *worker_func(void *arg) {
    ThreadArg *targ = (ThreadArg *)arg;

    /* Local list to minimize lock contention */
    TopKList local_list;
    list_init(&local_list, targ->k);

    FILE *fp = fopen(targ->filepath, "r");
    if (fp) {
        long long val;
        while (fscanf(fp, "%lld", &val) == 1) {
            list_insert(&local_list, val);
        }
        fclose(fp);
    }

    /* Merge local top-K into global list under mutex protection */
    pthread_mutex_lock(targ->mutex);
    Node *curr = local_list.head;
    while (curr != NULL) {
        list_insert(targ->global_list, curr->val);
        curr = curr->next;
    }
    pthread_mutex_unlock(targ->mutex);

    list_free(&local_list);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <K> <directoryPath> <outputFile>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int K = atoi(argv[1]);
    const char *dir_path = argv[2];
    const char *out_path = argv[3];

    int file_count = 0;
    char **files = get_eligible_files(dir_path, &file_count);

    TopKList global_list;
    list_init(&global_list, K);

    pthread_mutex_t mutex;
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("pthread_mutex_init");
        return EXIT_FAILURE;
    }

    pthread_t *threads = (pthread_t *)malloc(file_count * sizeof(pthread_t));
    ThreadArg *args = (ThreadArg *)malloc(file_count * sizeof(ThreadArg));

    for (int i = 0; i < file_count; i++) {
        args[i].filepath = files[i];
        args[i].k = K;
        args[i].global_list = &global_list;
        args[i].mutex = &mutex;

        if (pthread_create(&threads[i], NULL, worker_func, &args[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < file_count; i++) {
        pthread_join(threads[i], NULL);
    }

    list_write(&global_list, out_path);

    pthread_mutex_destroy(&mutex);
    free(threads);
    free(args);
    list_free(&global_list);
    free_file_list(files, file_count);

    return EXIT_SUCCESS;
}

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include "list.h"

#define BATCH_SIZE 64

typedef struct {
    long mtype;
    int count;
    int is_eof;
    long long data[BATCH_SIZE];
} MsgBuffer;

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

    /* System V Message Queue */
    int msqid = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (msqid < 0) {
        perror("msgget");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < file_count; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            msgctl(msqid, IPC_RMID, NULL);
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            /* Child process: build local top-K list */
            TopKList local_list;
            list_init(&local_list, K);

            FILE *fp = fopen(files[i], "r");
            if (fp) {
                long long val;
                while (fscanf(fp, "%lld", &val) == 1) {
                    list_insert(&local_list, val);
                }
                fclose(fp);
            }

            /* Send batched results to parent */
            MsgBuffer msg;
            msg.mtype = 1;
            msg.count = 0;
            msg.is_eof = 0;

            Node *curr = local_list.head;
            while (curr != NULL) {
                msg.data[msg.count++] = curr->val;
                if (msg.count == BATCH_SIZE) {
                    if (msgsnd(msqid, &msg, sizeof(MsgBuffer) - sizeof(long), 0) < 0) {
                        perror("msgsnd");
                        exit(EXIT_FAILURE);
                    }
                    msg.count = 0;
                }
                curr = curr->next;
            }

            /* Send remaining items + EOF marker */
            msg.is_eof = 1;
            if (msgsnd(msqid, &msg, sizeof(MsgBuffer) - sizeof(long), 0) < 0) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }

            list_free(&local_list);
            free_file_list(files, file_count);
            exit(EXIT_SUCCESS);
        }
    }

    /* Parent process: gather results into global top-K list */
    TopKList global_list;
    list_init(&global_list, K);

    int finished_children = 0;
    MsgBuffer recv_msg;

    while (finished_children < file_count) {
        if (msgrcv(msqid, &recv_msg, sizeof(MsgBuffer) - sizeof(long), 1, 0) < 0) {
            perror("msgrcv");
            break;
        }

        for (int i = 0; i < recv_msg.count; i++) {
            list_insert(&global_list, recv_msg.data[i]);
        }

        if (recv_msg.is_eof) {
            finished_children++;
        }
    }

    /* Wait for all child processes to terminate */
    for (int i = 0; i < file_count; i++) {
        wait(NULL);
    }

    /* Remove System V Message Queue */
    msgctl(msqid, IPC_RMID, NULL);

    list_write(&global_list, out_path);

    list_free(&global_list);
    free_file_list(files, file_count);

    return EXIT_SUCCESS;
}

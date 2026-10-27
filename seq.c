#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <K> <directoryPath> <outputFile>\n", argv[0]);
        return 1;
    }

    int K = atoi(argv[1]);     char *dirPath = argv[2];
    char *outputPath = argv[3];

    printf("Starting seq with K=%d, dir=%s, out=%s\n", K, dirPath, outputPath);

    return 0;
}
//Test// this is to see if it saves. Then Uploads to github.

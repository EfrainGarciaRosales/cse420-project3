#include "list.h"

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

    for (int i = 0; i < file_count; i++) {
        FILE *fp = fopen(files[i], "r");
        if (!fp) {
            perror("fopen input file");
            continue;
        }

        long long val;
        while (fscanf(fp, "%lld", &val) == 1) {
            list_insert(&global_list, val);
        }
        fclose(fp);
    }

    list_write(&global_list, out_path);

    list_free(&global_list);
    free_file_list(files, file_count);

    return EXIT_SUCCESS;
}

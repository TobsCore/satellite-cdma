#include <stdio.h>
#include <stdlib.h>

char *readFile(char *filename) {
    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen(filename, "rb");
    if (!fp) perror(filename), exit(1);

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    buffer = calloc(1, lSize + 1);
    if (!buffer) fclose(fp), fputs("memory alloc fails", stderr), exit(1);

    /* copy the file into the buffer */
    if (1 != fread(buffer, lSize, 1, fp))
        fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

    fclose(fp);
    free(buffer);
    return buffer;
}

int main(int argn, char *argv[]) {

    if (argn <= 1) {
        printf("Programm parameter missing: Please add a file, that should be read");
        return -1;
    }
    char *gpsSequence = readFile(argv[1]);
    printf(gpsSequence);

}
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

static const int BLKSIZE = 1024;

int open_ronly(const char* path)
{
    int fd = open(path, O_RDONLY);
    return fd;
}

char* file_to_str(const char* path)
{
    int fd = open_ronly(path);
    char* buf = (char*) malloc(BLKSIZE * sizeof(char));

    size_t total_read = 0;
    size_t counter = 0;
    int nbytes_read;
    int i = 1;

    while ( (nbytes_read = read(fd, buf + total_read, BLKSIZE)) > 0 ) {
        total_read += nbytes_read;
        counter += nbytes_read;
        if (counter == BLKSIZE) {
            ++i;
            buf = (char*) realloc(buf, i * BLKSIZE);
            counter = 0;
        }
    }
    buf[total_read] = 0;
    return buf;
}

/*
int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    char* filename = strdup(argv[1]);
    if (filename == NULL) exit(1);
    char* buf = file_to_str(filename);

    printf("%s", buf);
    free(buf);
}
*/

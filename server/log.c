#include <stdio.h>

#include "log.h"

void write_log(const char *file_name, char *msg) {
    FILE *file;

    file = fopen(file_name, "a");
    fprintf(file, "%s\n", msg);
    fflush(file);
}
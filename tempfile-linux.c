#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <paths.h>
#include <unistd.h>
#include "tempfile.h"

struct TempFile* make_temp_file(const char* format) {
    // No standardised way to getting the temporary directory on Linux, however
    // the most common way is to use the first available directory out of
    // $TMPDIR, P_tmpdir, _PATH_TMP or /tmp/ in that order.
    const char* temp_dir = getenv("TMPDIR");
    #ifdef P_tmpdir
    if(temp_dir == NULL || temp_dir[0] == '\0') {
        temp_dir = P_tmpdir;
    }
    #endif
    #ifdef _PATH_TMP
    if(temp_dir == NULL || temp_dir[0] == '\0') {
        temp_dir = _PATH_TMP;
    }
    #endif
    if(temp_dir == NULL || temp_dir[0] == '\0') {
        temp_dir = "/tmp/";
    }
    size_t temp_dir_len = strlen(temp_dir);

    size_t temp_file_len = temp_dir_len + strlen(format) + 5;
    char* temp_file;
    if(temp_dir[temp_dir_len - 1] != '/') {
        // Add trailing slash if it doesn't exist.
        temp_file_len += 1;
        temp_dir_len += 1;
        temp_file = malloc((temp_file_len + 1) * sizeof(char));
        strcpy(temp_file, temp_dir);
        temp_file[temp_dir_len - 1] = '/';
        temp_file[temp_dir_len] = '\0';

    } else {
        temp_file = malloc((temp_file_len + 1) * sizeof(char));
        strcpy(temp_file, temp_dir);
    }
    strcpy(temp_file + temp_dir_len, format);

    // Find the wildcard ('%'), and replace it with 'XXXXXX' for mkstemp().
    size_t wildcard_i;
    size_t suffix_len;
    for(wildcard_i = temp_dir_len; wildcard_i < temp_file_len - 5;
            ++wildcard_i) {
        if(temp_file[wildcard_i] == '%') {
            suffix_len = strlen(temp_file + wildcard_i + 1);
            if(suffix_len) {
                // Can't use memcpy since regions overlap, so manually do a
                // backwards copy.
                for(size_t j = 0; j < suffix_len; ++j) {
                    temp_file[temp_file_len - j - 1] = temp_file[
                        wildcard_i + suffix_len - j
                    ];
                }
            }
            temp_file[wildcard_i] = 'X';
            temp_file[wildcard_i + 1] = 'X';
            temp_file[wildcard_i + 2] = 'X';
            temp_file[wildcard_i + 3] = 'X';
            temp_file[wildcard_i + 4] = 'X';
            temp_file[wildcard_i + 5] = 'X';
            temp_file[temp_file_len] = '\0';
            break;
        }
    }
    if(wildcard_i == temp_file_len - 5) {
        free(temp_file);
        return NULL;
    }
    // Open the temporary file
    int fd;
    if(suffix_len == 0) {
        fd = mkstemp(temp_file);
    } else {
        fd = mkstemps(temp_file, suffix_len);
    }

    if(fd == -1) {
        free(temp_file);
        return NULL;
    }
    FILE* file = fdopen(fd, "w+b");
    if(file == NULL) {
        close(fd);
        unlink(temp_file);
        free(temp_file);
        return NULL;
    }
    setvbuf(file, NULL, _IOFBF, FileBufferLen);

    struct TempFile* result = malloc(sizeof(struct TempFile));
    result->file = file;
    result->filename = temp_file;
    return result;
}

void free_temp_file(struct TempFile* temp) {
    free((char*)temp->filename);
    free(temp);
}

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include <Shlwapi.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include "tempfile.h"

static wchar_t alpha_num[] = {
    L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'A', L'B',
    L'C', L'D', L'E', L'F', L'G', L'H', L'I', L'J', L'K', L'L', L'M', L'N',
    L'O', L'P', L'Q', L'R', L'S', L'T', L'U', L'V', L'W', L'X', L'Y', L'Z',
    L'a', L'b', L'c', L'd', L'e', L'f', L'g', L'h', L'i', L'j', L'k', L'l',
    L'm', L'n', L'o', L'p', L'q', L'r', L's', L't', L'u', L'v', L'w', L'x',
    L'y', L'z'
};

struct TempFile* make_temp_file(const char* format) {
    wchar_t temp_dir[MAX_PATH];
    DWORD temp_dir_len = GetTempPath(MAX_PATH, temp_dir);
    if(temp_dir_len == 0 || temp_dir_len >= MAX_PATH) {
        return NULL;
    }
    if(PathAddBackslash(temp_dir) == NULL) {
        return NULL;
    }

    // Length of entire path to the temp file. +4 since we don't want the null
    // character counted by MultiByteToWideChar, nor the '%' character, but we
    // want 6 characters to replace the '%'.
    size_t temp_file_len = wcslen(temp_dir) +
        MultiByteToWideChar(CP_UTF8, 0, format, -1, NULL, 0) + 4;

    wchar_t* temp_file = malloc((temp_file_len + 1) * sizeof(wchar_t));
    wcscpy_s(temp_file, temp_file_len + 1, temp_dir);
    MultiByteToWideChar(
        CP_UTF8, 0, format, -1, temp_file + temp_dir_len,
        temp_file_len + 1 - temp_dir_len
    );

    // Find the wildcard ('%'), and move all the text after it to the end.
    size_t wildcard_i;
    for(wildcard_i = temp_dir_len; wildcard_i < temp_file_len - 5;
            ++wildcard_i) {
        if(temp_file[wildcard_i] == L'%') {
            size_t suffix_len = wcslen(temp_file + wildcard_i + 1);
            if(suffix_len) {
                // See same comment in tempfile-linux.c
                for(size_t j = 0; j < suffix_len; ++j) {
                    temp_file[temp_file_len - j - 1] = temp_file[
                        wildcard_i + suffix_len - j
                    ];
                }
            }
            temp_file[temp_file_len] = L'\0';
            break;
        }
    }
    if(wildcard_i == temp_file_len - 5) {
        free(temp_file);
        return NULL;
    }

    // Try to find a valid filename and open it, essentially doing what
    // mkstemp() does on OS X/Linux.
    uint32_t random;
    int fd = -1;
    for(uint32_t attempt = 0; attempt < UINT32_MAX; ++attempt) {
        if(rand_s(&random)) {
            free(temp_file);
            return NULL;
        }
        temp_file[wildcard_i] = alpha_num[((random >> 26) & 0x3f) % 62];
        temp_file[wildcard_i + 1] = alpha_num[((random >> 20) & 0x3f) % 62];
        temp_file[wildcard_i + 2] = alpha_num[((random >> 14) & 0x3f) % 62];
        temp_file[wildcard_i + 3] = alpha_num[((random >> 8) & 0x3f) % 62];
        temp_file[wildcard_i + 4] = alpha_num[((random >> 2) & 0x3f) % 62];
        if(rand_s(&random)) {
            free(temp_file);
            return NULL;
        }
        temp_file[wildcard_i + 5] = alpha_num[((random >> 26) & 0x3f) % 62];

        // Use _O_CREAT | _O_EXCL to ensure we're the only ones using the file.
        errno_t err = _wsopen_s(
            &fd, temp_file, _O_RDWR | _O_BINARY | _O_NOINHERIT |
            _O_CREAT | _O_EXCL, _SH_DENYRW, _S_IREAD | _S_IWRITE
        );
        if(err != 0 && err != EEXIST) {
            // An error occurred that wasn't EEXIST
            free(temp_file);
            return NULL;
        } else if(err == 0) {
            // Temporary file created successfully.
            break;
        }
        // Continue if the file tried already exists.
    }
    if(fd == -1) {
        free(temp_file);
        return NULL;
    }

    FILE* file = _fdopen(fd, "w+b");
    if(file == NULL) {
        free(temp_file);
        _close(fd);
        return NULL;
    }
    setvbuf(file, NULL, _IOFBF, FileBufferLen);

    struct TempFile* result = malloc(sizeof(struct TempFile));
    result->file = file;
    result->filename = temp_file;
    return result;
}

void free_temp_file(struct TempFile* temp) {
    free((wchar_t*)temp->filename);
    free(temp);
}

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#ifdef _WIN32
    #include <io.h>
    #include <fcntl.h>
    #include <share.h>
    #include <sys/stat.h>
#endif // _WIN32

#include "args.h"
#include "tempfile.h"
#include "trim.h"

static FILE* open_file(const arg_char* name) {
#ifdef _WIN32
    // Open the file allowing shared read, but not shared write
    int fd;
    _wsopen_s(
        &fd, name, _O_RDWR | _O_BINARY | _O_NOINHERIT, _SH_DENYWR,
        _S_IREAD | _S_IWRITE
    );
    if(fd == -1) {
        return NULL;
    }
    FILE* file = _fdopen(fd, "r+b");
    if(file == NULL) {
        _close(fd);
        return NULL;
    }
    setvbuf(file, NULL, _IOFBF, FileBufferLen);
    return file;
#else
    FILE* file = fopen(name, "r+b");
    if(file != NULL) {
        setvbuf(file, NULL, _IOFBF, FileBufferLen);
    }
    return file;
#endif // _WIN32
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
#else
int main(int argc, char** argv) {
#endif
    struct Arguments args = parse_args(argc, argv);
    if(!args.valid) {
        return EXIT_FAILURE;
    }

    bool success = true;
    for(size_t i = 0; i < args.num_filenames; ++i) {
        FILE* file = open_file(args.filenames[i]);
        struct TempFile* temp_file = make_temp_file("newline_%.tmp");
        if(file == NULL) {
            arg_printerr(
                arg_f arg_s(": ") arg_f arg_s(": ") arg_f,
                argv[0], args.filenames[i], arg_strerror(errno)
            );
            if(temp_file != NULL) {
                fclose(temp_file->file);
                delete(temp_file->filename);
                free_temp_file(temp_file);
            }
            success = false;
        } else if(temp_file == NULL) {
            arg_printerr(
                arg_f arg_s(": ") arg_f arg_s(": Unable to create temporary ")
                arg_s("file"), argv[0], args.filenames[i]
            );
            fclose(file);
            success = false;
        } else {
            bool result = trim_file(
                file, temp_file->file, args.newline_type,
                args.trailing_newline, args.strip_whitespace
            );
            if(result) {
                // Need to copy the temp file to original file. It would be
                // faster to just rename() the temporary file to the original
                // file, but this won't preserve file metadata such as
                // permission bits or owners. ReplaceFile() does this on
                // Windows, but an easy solution for Unix systems doesn't seem
                // to exist.
                seek(file, 0, SEEK_SET);
                seek(temp_file->file, 0, SEEK_SET);
                uint8_t* buffer = malloc(FileBufferLen);
                size_t read_bytes = read(
                    temp_file->file, buffer, FileBufferLen
                );
                while(read_bytes) {
                    write(file, buffer, read_bytes);
                    read_bytes = read(
                        temp_file->file, buffer, FileBufferLen
                    );
                }
                free(buffer);
                fflush(file);
                truncate(file);
            }
            if(args.verbose) {
                if(result) {
                    arg_print(
                        arg_s("Processed ") arg_f,
                        args.filenames[i]
                    );
                } else {
                    arg_print(
                        arg_s("No changes made to ") arg_f,
                        args.filenames[i]
                    );
                }
            }
            fclose(file);
            fclose(temp_file->file);
            delete(temp_file->filename);
            free_temp_file(temp_file);
        }
    }
    free_args(&args);
    if(!success) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

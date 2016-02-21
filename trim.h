#ifndef NEWLINE_TRIM_H
#define NEWLINE_TRIM_H

#include <stdbool.h>
#include <stdio.h>
#include "args.h"

// Rely on _FILE_OFFSET_BITS=64 for LFS on both Windows and POSIX.
#ifdef _WIN32
    #include <io.h>
    #define seek(file, offset, whence) fseeko(file, offset, whence)
    #define truncate(file) _chsize_s(_fileno(file), ftello(file))
    #define tell(file) ftello(file)
    #define read(file, ptr, size) fread(ptr, 1, size, file)
    #define write(file, ptr, size) fwrite(ptr, 1, size, file)
    #define delete(file) _wunlink(file)
#else
    #include <unistd.h>
    #define seek(file, offset, whence) fseeko(file, offset, whence)
    #define truncate(file) ftruncate(fileno(file), ftello(file))
    #define tell(file) ftello(file)
    #define read(file, ptr, size) fread(ptr, 1, size, file)
    #define write(file, ptr, size) fwrite(ptr, 1, size, file)
    #define delete(file) unlink(file)
#endif

// Buffer length for file IO operations (64 KiB)
static const size_t FileBufferLen = 64*1024;

bool trim_file(FILE* in_file, FILE* out_file, enum NewlineType newline_type,
               bool trailing_newline, bool strip);

#endif // NEWLINE_TRIM_H

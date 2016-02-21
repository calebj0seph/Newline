#ifndef NEWLINE_TRIM_H
#define NEWLINE_TRIM_H

#include <stdbool.h>
#include <stdio.h>
#include "args.h"

#ifdef _WIN32
    #include <io.h>
    // Use Microsoft-specific 64-bit seek and tell functions for LFS support
    typedef int64_t file_pos;
    #define seek(file, offset, whence) _fseeki64(file, offset, whence)
    #define truncate(file) _chsize_s(_fileno(file), _ftelli64(file))
    #define tell(file) _ftelli64(file)
    #define read(file, ptr, size) fread(ptr, 1, size, file)
    #define write(file, ptr, size) fwrite(ptr, 1, size, file)
    #define delete(file) _wunlink(file)
#else
    #include <unistd.h>
    // Use _FILE_OFFSET_BITS=64 and fseeko/ftello for POSIX LFS support
    typedef off_t file_pos;
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

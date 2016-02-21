#ifndef NEWLINE_TEMPFILE_H
#define NEWLINE_TEMPFILE_H

#include <stdio.h>
#ifdef _WIN32
#include <wchar.h>
#endif // _WIN32

struct TempFile {
    FILE* file;
#ifdef _WIN32
    const wchar_t* filename;
#else
    const char* filename;
#endif // _WIN32
};

/* Returns a file handle to a temporary file with read and write access. The
'format' parameter defines the filename of the temporary file. It must contain
a single '%' character which will be replaced with random letters and digits to
make the filename unique. Returns NULL if 'format' does not contain a single
'%' character, or if a temporary file was not able to be created. */
struct TempFile* make_temp_file(const char* format);

/* Releases memory allocated by make_temp_file. This does not close the file
handle associated with the temporary file, nor deletes it. */
void free_temp_file(struct TempFile* tempfile);

#endif // NEWLINE_TEMPFILE_H

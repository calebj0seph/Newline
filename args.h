#ifndef NEWLINE_ARGS_H
#define NEWLINE_ARGS_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
    #include <wchar.h>
    typedef wchar_t arg_char;
    #define arg_strcmp          wcscmp
    #define arg_strncmp         wcsncmp
    #define arg_stricmp         _wcsicmp
    #define arg_strlen          wcslen
    #define arg_s(val)          L##val
    #define arg_f               "%ls"
    #define arg_fc              "%lc"
    #define arg_print(fmt, ...) wprintf(fmt L"\n", ##__VA_ARGS__)
    #define arg_printerr(fmt, ...) fwprintf(stderr, fmt "\n", ##__VA_ARGS__)
    #define arg_strerror _wcserror
#else
    typedef char arg_char;
    #define arg_strcmp          strcmp
    #define arg_strncmp         strncmp
    int arg_stricmp(const char* lhs, const char* rhs);
    #define arg_strlen          strlen
    #define arg_s(val)          val
    #define arg_f               "%s"
    #define arg_fc              "%c"
    #define arg_print(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
    #define arg_printerr(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
    #define arg_strerror strerror
#endif // _WIN32

enum NewlineType {
    LF,
    CRLF,
    CR,
    KEEP
};

struct Arguments {
    enum NewlineType newline_type; // -t, --type
    bool trailing_newline;         // !(--no-newline)
    bool strip_whitespace;         // !(--no-strip)
    bool verbose;                  // -v, --verbose
    bool valid;                    // Set to true if arguments were valid
    size_t num_filenames;          // Number of files in 'filenames'
    size_t filenames_capacity;     // Capacity of 'filenames'
    const arg_char** filenames;    // Array of filenames
};

struct Arguments parse_args(int argc, arg_char** argv);
void free_args(struct Arguments* args);

#endif // NEWLINE_ARGS_H

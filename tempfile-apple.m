#import <Foundation/Foundation.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "trim.h"
#include "tempfile.h"

struct TempFile* make_temp_file(const char* format) {
    char* temp_file;
    size_t suffix_len;

    // Need to manually create a NSAutoreleasePool to release memory since this
    // isn't a standard OS X app.
    @autoreleasepool {
        NSString* temp_dir_ns = NSTemporaryDirectory();
        if(temp_dir_ns == nil) {
            return NULL;
        }

        // Replace wildcard ('%') with 'XXXXXX' for mkstemp(). This is a
        // hundred times easier using Cocoa than fiddling with C-strings in the
        // Windows/Linux version ;)
        NSString* format_ns = [NSString stringWithUTF8String:format];
        NSRange wildcard_range = [format_ns rangeOfString: @"%"];
        if(wildcard_range.location == NSNotFound) {
            return NULL;
        }
        suffix_len = strlen(
            [[format_ns substringFromIndex:
                wildcard_range.location + 1
             ] UTF8String]
        );
        format_ns = [format_ns stringByReplacingCharactersInRange:
            wildcard_range withString: @"XXXXXX"
        ];

        // Concatenate the directory and filename
        NSString* temp_file_ns = [temp_dir_ns stringByAppendingPathComponent:
            format_ns
        ];

        // Finish up with Cocoa and get a C-string to use with mkstemp().
        size_t temp_file_len = strlen([temp_file_ns UTF8String]);
        temp_file = malloc((temp_file_len + 1) * sizeof(char));
        strcpy(temp_file, [temp_file_ns UTF8String]);
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

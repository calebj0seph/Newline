#ifndef NEWLINE_TRIM_H
#define NEWLINE_TRIM_H

#include <stdbool.h>
#include <stdio.h>
#include "args.h"

/* Processes 'in_file', writing the result to 'out_file'. Requires that
'in_file' be opened for reading in binary mode, and 'out_file' be opened for
reading and writing in binary mode. Both 'in_file' and 'out_file' must support
seeking.

Newline sequences in the file (LF, CRLF or CR) will be converted to the newline
sequence specified by 'newline_type'. If 'newline_type' is KEEP, newline
sequences will remain unchanged, even if they are inconsistent.

If 'trailing_newline' is true, a single newline sequence specified by
'newline_type' will be added to the end of the file if one doesn't already
exist. If 'newline_type' is KEEP, the type of newline sequence to add will be
automatically determined by the most common newline sequence already in the
file.

If 'trailing_newline' is true and multiple newline sequences are at the end of
the file, they shall be merged into a single newline sequence.

If 'strip' is true, any whitespace before each newline sequence or the end of
the file will be removed. Whitespace is considered to be any sequence of
consecutive space or tab characters. */
bool trim_file(FILE* in_file, FILE* out_file, enum NewlineType newline_type,
               bool trailing_newline, bool strip);

#endif // NEWLINE_TRIM_H

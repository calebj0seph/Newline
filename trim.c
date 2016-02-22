#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "args.h"
#include "trim.h"

bool trim_file(FILE* in_file, FILE* out_file, enum NewlineType newline_type,
               bool trailing_newline, bool strip) {
    bool changes_made = false;
    off_t consecutive_whitespace = 0;
    off_t consecutive_newline = 0;
    size_t num_lf = 0;
    size_t num_crlf = 0;
    size_t num_cr = 0;

    uint8_t cur_byte;
    size_t cur_bytes_len = fread(&cur_byte, 1, 1, in_file);
    while(cur_bytes_len) {
        if((char)cur_byte == '\r' || (char)cur_byte == '\n') {
            // Determine current newline type
            uint8_t next_byte;
            size_t next_bytes_len = fread(&next_byte, 1, 1, in_file);
            enum NewlineType cur_newline;
            if(next_bytes_len && (char)cur_byte == '\r' &&
                    (char)next_byte == '\n') {
                cur_newline = CRLF;
                num_crlf += 1;
            } else if((char)cur_byte == '\n') {
                cur_newline = LF;
                num_lf += 1;
            } else {
                cur_newline = CR;
                num_cr += 1;
            }

            // Go back one character if we didn't read a CRLF
            if(next_bytes_len && cur_newline != CRLF) {
                fseeko(in_file, -1, SEEK_CUR);
            }

            // Handle trailing whitespace
            if(strip && consecutive_whitespace > 0) {
                changes_made = true;
                fseeko(out_file, -consecutive_whitespace, SEEK_CUR);
                consecutive_whitespace = 0;
            }

            // Write newline
            enum NewlineType newline_to_write = cur_newline;
            if(newline_type != KEEP && newline_type != cur_newline) {
                changes_made = true;
                newline_to_write = newline_type;
            }
            if(newline_to_write == LF) {
                fwrite((uint8_t[]){10}, 1, 1, out_file);
                if(trailing_newline) {
                    consecutive_newline += 1;
                }
            } else if(newline_to_write == CRLF) {
                fwrite((uint8_t[]){13, 10}, 1, 2, out_file);
                if(trailing_newline) {
                    consecutive_newline += 2;
                }
            } else {
                fwrite((uint8_t[]){13}, 1, 1, out_file);
                if(trailing_newline) {
                    consecutive_newline += 1;
                }
            }
        } else if((char)cur_byte == ' ' || (char)cur_byte == '\t') {
            // Write and count consecutive whitespace
            if(!strip) {
                consecutive_newline = 0;
            } else {
                consecutive_whitespace += 1;
            }
            fwrite(&cur_byte, 1, 1, out_file);
        } else {
            // Write normal character
            consecutive_whitespace = 0;
            consecutive_newline = 0;
            fwrite(&cur_byte, 1, 1, out_file);
        }
        cur_bytes_len = fread(&cur_byte, 1, 1, in_file);
    }

    // Handle trailing whitespace at end of file
    if(strip && consecutive_whitespace > 0) {
        changes_made = true;
        fseeko(out_file, -consecutive_whitespace, SEEK_CUR);
        consecutive_whitespace = 0;
    }

    // Handle trailing newlines
    if(trailing_newline) {
        if(consecutive_newline > 0) {
            // Trim excess trailing newlines
            fseeko(out_file, -consecutive_newline, SEEK_CUR);
            cur_bytes_len = fread(&cur_byte, 1, 1, out_file);
            if(cur_bytes_len && (char)cur_byte == '\n' &&
                    consecutive_newline > 1) {
                // Need to truncate the file after the first LF
                changes_made = true;
            } else if(cur_bytes_len && (char)cur_byte == '\r') {
                // Consume the LF followed by CR if it exists
                cur_bytes_len = fread(&cur_byte, 1, 1, out_file);
                if(cur_bytes_len && (char)cur_byte == '\n') {
                    if(consecutive_newline > 2) {
                        // Need to truncate the file after the first CRLF
                        changes_made = true;
                    }
                } else {
                    if(consecutive_newline > 1) {
                        // Need to truncate the file after the first CR
                        changes_made = true;
                    }
                    if(cur_bytes_len) {
                        // Seek back if we didn't read a CRLF
                        fseeko(out_file, -1, SEEK_CUR);
                    }
                }
            }
        } else {
            // Add trailing newline when none exist
            enum NewlineType trailing_newline_type = newline_type;
            if(trailing_newline_type == KEEP) {
                // Choose best newline format, preferring LF, followed by CRLF
                if(num_lf >= num_crlf && num_lf >= num_cr) {
                    trailing_newline_type = LF;
                } else if(num_crlf >= num_lf && num_crlf >= num_cr) {
                    trailing_newline_type = CRLF;
                } else {
                    trailing_newline_type = CR;
                }
            }
            if(trailing_newline_type == LF) {
                fwrite(((uint8_t[]){10}), 1, 1, out_file);
            } else if(trailing_newline_type == CRLF) {
                fwrite((uint8_t[]){13, 10}, 1, 2, out_file);
            } else {
                fwrite(((uint8_t[]){13}), 1, 1, out_file);
            }
            changes_made = true;
        }
    }

    off_t out_file_len = ftello(out_file);
    fflush(out_file);
    if(changes_made) {
        ftruncate(fileno(out_file), out_file_len);
    }
    return changes_made;
}

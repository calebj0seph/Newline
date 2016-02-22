#include "args.h"
#include <stdlib.h>

#ifndef _WIN32
    #include <ctype.h>
    int arg_stricmp(const char* lhs, const char* rhs) {
        while(*lhs != '\0') {
            if(*rhs == '\0') {
                return 1; // LHS is longer than RHS
            }
            if(tolower((unsigned char)(*lhs)) >
                    tolower((unsigned char)(*rhs))) {
                return 1; // LHS is lexicographically greater than RHS
            }
            if(tolower((unsigned char)(*rhs)) >
                    tolower((unsigned char)(*lhs))) {
                return -1; // RHS is lexicographically greater than LHS
            }
            ++lhs;
            ++rhs;
        }
        if (*rhs != '\0') {
            return -1; // LHS is shorter than RHS
        }
        return 0; // LHS == RHS
    }
#endif // _WIN32

static void parse_arg_file(struct Arguments* args, const arg_char* arg) {
    if(args->filenames == NULL) {
        args->filenames = malloc(sizeof(arg_char*));
        args->filenames_capacity = 1;
        args->num_filenames = 0;
    }
    if(++(args->num_filenames) > args->filenames_capacity) {
        // Grow array by factor of 1.5 if not enough capacity
        size_t new_capacity = args->filenames_capacity + (
            1 + ((args->filenames_capacity - 1) / 2)
        );
        args->filenames = realloc(
            args->filenames, new_capacity * sizeof(arg_char*)
        );
        // Clear newly allocated memory
        memset(
            args->filenames + args->filenames_capacity, 0,
            (new_capacity - args->filenames_capacity) * sizeof(arg_char*)
        );
        args->filenames_capacity = new_capacity;
    }
    args->filenames[args->num_filenames - 1] = arg;
}

static void parse_arg_option_type(struct Arguments* args,
                                  const arg_char* prog_name,
                                  const arg_char* arg_name,
                                  const arg_char* arg) {
    if(arg == NULL) {
        // No argument given for option
        args->valid = false;
        if(!arg_strncmp(arg_name, arg_s("--"), 2)) {
            arg_printerr(
                arg_f arg_s(": option '") arg_f
                arg_s("' requires an argument"), prog_name, arg_name
            );
        } else {
            arg_printerr(
                arg_f arg_s(": option requires an argument -- '") arg_f
                arg_s("'"), prog_name, arg_name
            );
        }
        return;
    }

    // Determine the argument for the 'type' option
    if(!arg_stricmp(arg, arg_s("LF"))) {
        args->newline_type = LF;
    } else if(!arg_stricmp(arg, arg_s("CRLF"))) {
        args->newline_type = CRLF;
    }/* else if(!arg_stricmp(arg, arg_s("CR"))) {
        // This works fine but is disabled to avoid confusion with LF since CR
        // is so rarely used.
        args->newline_type = CR;
    }*/ else if(!arg_stricmp(arg, arg_s("KEEP"))) {
        args->newline_type = KEEP;
    } else {
        // Invalid argument
        args->valid = false;
        if(!arg_strncmp(arg_name, arg_s("--"), 2)) {
            arg_printerr(
                arg_f arg_s(": option '") arg_f
                arg_s("' given invalid argument '") arg_f arg_s("'"),
                prog_name, arg_name, arg
            );
        } else {
            arg_printerr(
                arg_f arg_s(": option given invalid argument '") arg_f
                arg_s("' -- '") arg_f arg_s("'"), prog_name, arg, arg_name
            );
        }
    }
}

struct Arguments parse_args(int argc, arg_char** argv) {
    bool reading_options = true;
    bool display_help = false;
    bool display_version = false;
    struct Arguments args = {
        .valid = true,
        .newline_type = LF,
        .trailing_newline = true,
        .strip_whitespace = true,
        .verbose = false,
        .num_filenames = 0,
        .filenames_capacity = 0,
        .filenames = NULL
    };
    for(int i = 1; i < argc; ++i) {
        size_t arg_len = arg_strlen(argv[i]);
        if(!arg_len) {
            continue;
        }
        if(reading_options && argv[i][0] == arg_s('-')) {
            if(arg_len == 2 && argv[i][1] == arg_s('-')) {
                // '--' argument, ignore and start reading files
                reading_options = false;
                continue;
            } else if(!arg_strcmp(argv[i], arg_s("--help"))) {
                display_help = true;
                break;
            } else if(!arg_strcmp(argv[i], arg_s("--version"))) {
                display_version = true;
                break;
            } else if(!arg_strncmp(argv[i], arg_s("--type"), 6) &&
                    (arg_len == 6 || argv[i][6] == arg_s('='))) {
                if(arg_len == 6) {
                    parse_arg_option_type(
                        &args, argv[0], arg_s("--type"), NULL
                    );
                } else {
                    parse_arg_option_type(
                        &args, argv[0], arg_s("--type"), argv[i] + 7
                    );
                }
                if(!args.valid) {
                    break;
                }
            } else if(!arg_strcmp(argv[i], arg_s("--no-trailing-newline"))) {
                args.trailing_newline = false;
            } else if(!arg_strcmp(argv[i], arg_s("--no-strip-whitespace"))) {
                args.strip_whitespace = false;
            } else if(!arg_strcmp(argv[i], arg_s("--verbose"))) {
                args.verbose = true;
            } else {
                if(arg_len >= 2 && argv[i][1] == arg_s('-')) {
                    // Invalid long option
                    arg_printerr(
                        arg_f arg_s(": unrecognized option '") arg_f
                        arg_s("'"), argv[0], argv[i]
                    );
                    args.valid = false;
                    break;
                } else if(arg_len >=  2) {
                    // Handle single character options
                    for(size_t j = 1; j < arg_len; ++j) {
                        switch(argv[i][j]) {
                            case arg_s('t'):
                                if(j == arg_len - 1) {
                                    if(i == argc - 1) {
                                        // 't' option without argument
                                        parse_arg_option_type(
                                            &args, argv[0], arg_s("t"), NULL
                                        );
                                    } else {
                                        // 't' option with space before
                                        // argument
                                        parse_arg_option_type(
                                            &args, argv[0], arg_s("t"),
                                            argv[++i]
                                        );
                                    }

                                } else {
                                    // 't' option without space before argument
                                    parse_arg_option_type(
                                        &args, argv[0], arg_s("t"),
                                        argv[i] + j + 1
                                    );
                                    j = arg_len;
                                }
                                break;
                            case arg_s('N'):
                                args.trailing_newline = false;
                                break;
                            case arg_s('S'):
                                args.strip_whitespace = false;
                                break;
                            case arg_s('v'):
                                args.verbose = true;
                                break;
                            default:
                                // Invalid short/single character option
                                arg_printerr(
                                    arg_f arg_s(": invalid option -- '") arg_fc
                                    arg_s("'"), argv[0], argv[i][j]
                                );
                                args.valid = false;
                        }
                        if(!args.valid) {
                            break;
                        }
                    }
                    if(!args.valid) {
                        break;
                    }
                } else {
                    // Single '-'
                    arg_printerr(
                        arg_f arg_s(": processing of stdin is currently not ")
                        arg_s("supported"), argv[0]
                    );
                    args.valid = false;
                    break;
                }
            }
        } else {
            // Positional argument
            parse_arg_file(&args, argv[i]);
        }
    }
    if(!(display_help || display_version) && args.valid &&
            args.num_filenames == 0) {
        // No filenames given
        arg_printerr(
            arg_f arg_s(": missing operand"), argv[0]
        );
        args.valid = false;
    } else if((display_help || display_version || !args.valid) &&
            args.num_filenames > 0) {
        // Remove filenames from arguments if filenames were read but '--help'
        // or '--version' were also given, or if there were invalid options
        free_args(&args);
    }

    if(!args.valid) {
        arg_printerr(
            arg_s("Try '") arg_f arg_s(" --help' for more information."),
            argv[0]
        );
    } else if(display_help) {
        arg_print(
            arg_s("Usage: ") arg_f arg_s(" [OPTION]... FILE..."),
            argv[0]
        );
        arg_print(
            arg_s("Reformat newlines and remove trailing whitespace in ")
            arg_s("FILE(s)")
        );
        arg_print(arg_s(""));
        arg_print(
            arg_s("  -t, --type=TYPE            ")
            arg_s("type of newline to change current newlines to,")
        );
        arg_print(
            arg_s("                               ")
            arg_s("must be one of 'lf', 'crlf' or 'keep' (default:")
        );
        arg_print(
            arg_s("                               ")
            arg_s("'lf')")
        );
        arg_print(
            arg_s("  -N, --no-trailing-newline  ")
            arg_s("don't add a trailing newline to the end of the")
        );
        arg_print(
            arg_s("                               ")
            arg_s("file, or remove existing excess trailing")
        );
        arg_print(
            arg_s("                               ")
            arg_s("newlines")
        );
        arg_print(
            arg_s("  -S, --no-strip-whitespace  ")
            arg_s("don't strip whitespace from the end of lines")
        );
        arg_print(
            arg_s("  -v, --verbose              ")
            arg_s("show whether or not changes are made to each file")
        );
        arg_print(
            arg_s("      --help                 ")
            arg_s("display this help and exit")
        );
        arg_print(
            arg_s("      --version              ")
            arg_s("output version information and exit")
        );
    } else if(display_version) {
        arg_print(arg_s("Newline 0.1.3"));
        arg_print(arg_s("Copyright (c) 2016 Caleb Joseph"));
        arg_print(arg_s(""));
        arg_print(arg_s("Licensed under the terms of the MIT license."));
        arg_print(
            arg_s("See <https://github.com/calebj0seph/")
            arg_s("Newline/blob/master/LICENSE> for more")
        );
        arg_print(arg_s("information."));
    }
    return args;
}

void free_args(struct Arguments* args) {
    if(args->filenames != NULL) {
        free(args->filenames);
        args->filenames = NULL;
        args->filenames_capacity = 0;
        args->num_filenames = 0;
    }
}

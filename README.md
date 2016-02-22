# Newline
Newline is a command line utility for reformatting newline characters and removing
trailing whitespace from text files.

## Motivation
Newline was created as an automatable way to keep source code neat and consistent when working across different editors.

For instance, Microsoft Visual Studio saves all files with CRLF newlines without removing trailing whitespace. This can be an issue if your project's coding style prohibits this, or you simply prefer to stick to one newline format across files without having trailing whitespace.

Using Newline, you can make all of your source code neat and consistent before committing it to a repository.

## Features
* Can convert newlines to LF (Unix-style `\n`), CRLF (Windows-style `\r\n`) or leave them unchanged.
* Can strip whitespace from the end of lines.
* Can add a trailing newline to the end of files if one doesn't already exist, and remove excess trailing newlines from the end of files.
* Supports any ASCII-like encoding of files such as UTF-8, UTF-8 without BOM or ISO-8859-1.
* Supports Linux, OS X and Windows (with proper Unicode filename support).
* Fast. Newline is written in C, and can process a 1GiB text file with 21 million lines at a rate of 9.4 MiB/s on a regular HDD.

Newline doesn't perform any conversion of tab characters. If you want to convert tabs to spaces or vice-versa, please refer to the `expand` and `unexpand` programs respectively from the GNU Core Utilities.

## Usage
`newline [OPTION]... FILE...`

Each `FILE` argument specifies a text file to process. Options are applied to all files processed.

| Option | Description |
| ------ | ----------- |
| `-t TYPE`, `--type=TYPE` | <p>The type of newline to use (default: `lf`). `TYPE` must be of either `lf`, `crlf` or `keep` (case insensitive).</p><p>`lf` specifies to use an LF character as the newline (Unix-style `\n`), `crlf` specifies to use the sequence CRLF as the newline (Windows-style `\r\n`), and `keep` specifies to keep newlines unchanged.</p> |
| `-N`, `--no-trailing-newline` | <p>Doesn't add a trailing newline to the file, or modify existing trailing newlines.</p><p>If not given, a trailing newline will be added to the file if one doesn't already exist, or if multiple newlines exist at the end of the file, they will be merged into a single newline.</p><p>If not given, the type of newline added is determined by the `--type` option. In the case of `keep`, the type of newline added is automatically determined.</p> |
| `-S`, `--no-strip-whitespace` | <p>Doesn't strip whitespace from the end of lines.</p><p>If not given, any consecutive tab or space characters before each newline are removed from the file.</p> |
| `-v`, `--verbose` | <p>Displays the name of each file processed, including whether or not any changes were made.</p> |
| `--help` | <p>Show the help message and exit.</p> |
| `--version` | <p>Show version information and exit.</p> |

Standard POSIX command line argument conventions apply, i.e. `newline -- --verbose -N` would process a file named `--verbose` and `-N`, `newline -NSv` is the same as `newline -N -S -v`, and `newline -tCRLF` is the same as `newline -t CRLF`.

### Recursively processing directory
You can make use of a Bash or Batch script to run Newline on an entire directory.

#### Bash script
This example also converts tabs to spaces using `expand`.

```bash
#!/usr/bin/env bash

# Specify the directory to recurse
directory="./My Project/src"

# Specify the types of files to process
files="*.py;*.md"

# Specify what to do with each file
process_file() {
    temp_file="$(mktemp expand_XXXXXX.tmp)"
    expand --tabs=4 "$1" > "$temp_file"
    mv "$temp_file" "$1"
    newline -vt LF "$1"
}

export -f process_file

echo $files | {
    IFS=";" read -r -a file_types
    for file_type in "${file_types[@]}"; do
        find "$directory" -iname "$file_type" \
            -exec bash -c 'process_file "$0"' {} \;
    done
}
```

#### Batch script
```batch
@echo off
rem Specify the directory to recurse
set directory=.\My Project\src

rem Specify the types of files to process
set files=*.py;*.md

rem Specify the command line for Newline
set newline=.\newline.exe -vt CRLF

for /r "%directory%" %%f in (%files%) do (
    %newline% "%%f"
)
```
## Installation
Since Newline has no third-party dependencies and is only comprised of a few C files, a simple Makefile is used to build Newline.

### Unix-like systems
Just run `make install`!

You can configure things like where Newline is installed or the compiler to use from the command line. The following command will install the Newline binary in `/usr/local/bin` (this is the default) and compile with `clang`:

`make install prefix=/usr/local CC=clang`

### Windows
For Windows, prebuilt binaries are available [here](../../releases/latest).

Currently compiling Newline with Visual Studio isn't supported, however you can compile Newline natively for Windows using any C99 compliant compiler such as Mingw-w64.

The makefile doesn't require anything like Cygwin or MSYS to be installed as it uses native Windows commands like `del` and `copy` when it detects it's being run on Windows. All you need access to is a C99 compliant compiler and access to Make. If using Mingw-w64, you can just do something like:

`mingw32-make.exe install prefix=%HOMEPATH%\Desktop\Newline`

This will install Newline to your Desktop.

## License
Newline is licensed under the terms of the MIT license. See the `LICENSE` file for more information.

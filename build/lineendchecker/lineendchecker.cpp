//
// Line-end Checker
//
// Checks an ASCII or UTF-8 text file and make sure that every line conforms
// to the specified line-end format.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>



typedef enum {
    win, unix, mac
} mode_t;



bool CheckWin(const char *buf, size_t size)
{
    const char *p, *q;

    // Make sure that every CR is followed by an LF.
    for (p = buf; p < buf + size; p = q + 2) {
        q = (const char *) memchr(p, '\r', buf + size - p);
        if (q == NULL) {
            break;
        }

        if (q == buf + size - 1 || *(q + 1) != '\n') {
            return false;
        }
    }

    // Make sure that every LF is preceded by a CR.
    for (p = buf; p < buf + size; p = q + 2) {
        q = (const char *) memchr(p, '\n', buf + size - p);
        if (q == NULL) {
            break;
        }

        if (q == buf || *(q - 1) != '\r') {
            return false;
        }
    }

    return true;
}

bool CheckUnix(const char *buf, size_t size)
{
    // Make sure that there is no CR in the file.
    return memchr(buf, '\r', size) == NULL;
}

bool CheckMac(const char *buf, size_t size)
{
    // Make sure that there is no LF in the file.
    return memchr(buf, '\n', size) == NULL;
}

void printUsage(const wchar_t *argv0)
{
    wprintf(L"Usage: %s option file\n", argv0);
    _putws(L"\nOptions:");
    _putws(L"\t/win\t\tWindows line-end (CR+LF)");
    _putws(L"\t/unix\t\tUnix line-end (LF)");
    _putws(L"\t/mac\t\tMac OS line-end (CR)");
    _putws(L"\t/?, /help\tprint this help message");
}

int wmain(int argc, const wchar_t * const argv[])
{
    //
    // Parse command-line arguments.
    //
    if (argc == 2 &&
        (wcscmp(argv[1], L"/?") == 0 || wcscmp(argv[1], L"/help") == 0)) {
        printUsage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (argc != 3) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    mode_t mode;

    if (wcscmp(argv[1], L"/win") == 0) {
        mode = win;
    } else if (wcscmp(argv[1], L"/unix") == 0) {
        mode = unix;
    } else if (wcscmp(argv[1], L"/mac") == 0) {
        mode = mac;
    } else {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    const wchar_t *filename = argv[2];

    //
    // Read the whole file.
    //
    FILE *f;
    if (_wfopen_s(&f, filename, L"rb") != 0) {
        wprintf(L"Can't open %s\n", filename);
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(_fileno(f), &st) != 0) {
        wprintf(L"Can't get size of file %s\n", filename);
        return EXIT_FAILURE;
    }

    char *buf = (char *) malloc(st.st_size);
    if (buf == NULL) {
        wprintf(L"Out of memory\n");
        return EXIT_FAILURE;
    }

    if (fread(buf, 1, st.st_size, f) != (size_t) st.st_size) {
        wprintf(L"Can't read %s\n", filename);
    }

    fclose(f);

    //
    // Check the line-ends.
    //

    bool pass = false;

    switch (mode) {
    case win:
        pass = CheckWin(buf, st.st_size);
        break;
    case unix:
        pass = CheckUnix(buf, st.st_size);
        break;
    case mac:
        pass = CheckMac(buf, st.st_size);
        break;
    }

    free(buf);

    wprintf(L"Line-end checking %s\n", pass? L"passed" : L"failed");
    return pass ? EXIT_SUCCESS : EXIT_FAILURE;
}

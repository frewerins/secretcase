#include <stdio.h>
#include <string.h>
#include <unistd.h>

const int MAX_LENGTH = 4096;
const char* program_code = "print(%s)";
const char* code_language = "python3";

int main()
{
    char str[MAX_LENGTH];
    char program[MAX_LENGTH];
    fgets(str, sizeof(str), stdin);
    char* pos;
    if (pos = strchr(str, '\n')) {
        *pos = '\0';
    }
    if (strlen(str) != 0) {
        snprintf(program, sizeof(program), program_code, str);
        execlp(code_language, code_language, "-c", program, NULL);
    }
    return 0;
}
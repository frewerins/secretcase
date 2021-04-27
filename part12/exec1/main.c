#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

const char* program_code =
    "#include <stdio.h>\nint main(){int expr = (%s); printf(\"%%d\","
    "expr); return 0; }";
const char* code_language = "gcc";

int main()
{
    char str[PATH_MAX];
    fgets(str, sizeof(str), stdin);
    char* end = str + strlen(str) - 1;
    for (char* end = str; end < str + strlen(str); ++end) {
        if (*end == '\n') {
            *end = '\0';
        }
    }
    if (strlen(str) == 0) {
        return 0;
    }
    FILE* file = fopen("a.c", "w");
    fprintf(file, program_code, str);
    fclose(file);
    pid_t pid = fork();
    if (pid == 0) {
        execlp(code_language, code_language, "-o", "a._out", "a.c", NULL);
    }
    int status;
    waitpid(pid, &status, 0);
    execlp("./a._out", "./a._out", NULL);
    return 0;
}
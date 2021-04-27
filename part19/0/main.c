#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
    void *lib = dlopen(argv[1], RTLD_NOW);
    double (*function)(double) = dlsym(lib, argv[2]);
    double value;
    while (scanf("%lf", &value) != EOF) {
        printf("%.3lf\n", function(value));
    }
    dlclose(lib);
    return 0;
}
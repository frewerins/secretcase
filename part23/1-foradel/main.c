#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#define FUSE_USE_VERSION 30   // API version 3.0
#include <fuse.h>

int file_system_size = 0;
int file_system_dirs[2048];

int last(const char* path) {
    int number = -1;
    struct stat* st = malloc(sizeof(struct stat));
    memset(st, 0, sizeof(struct stat));
    for (size_t i = 0; i < file_system_size; ++i) {
        struct stat* buffer = malloc(sizeof(struct stat));
        if (fstatat(file_system_dirs[i], path + 1, buffer, 0) == 0) {
            if (buffer->st_mtime > st->st_mtime) {
                number = i;
                memcpy(st, buffer, sizeof(struct stat));
            }
        }
        free(buffer);
    }
    free(st);
    return number;
}

int my_stat(const char* path, struct stat* st, struct fuse_file_info* fi) {
    if (strcmp("/", path) != 0) {
        int file;
        if (-1 == (file = last(path))) {
            return -ENOENT;
        }
        fstatat(file_system_dirs[file], path + 1, st, 0);
        if (S_ISDIR(st->st_mode)) {
            st->st_mode = S_IFDIR | 0555;
        } else {
            st->st_mode = S_IFREG | 0444;
        }
    } else {
        st->st_mode = S_IFDIR | 0555;
        st->st_nlink = 2;
    }
    return 0;
}
void opening(const char* path, int* data) {
    for (size_t i = 0; i < file_system_size; ++i) {
        if (strcmp(path, "/") != 0) {
            data[i] = openat(file_system_dirs[i], path + 1, O_RDONLY);
        } else {
            data[i] = openat(file_system_dirs[i], ".", O_RDONLY);
        }
    }
}

void closing(int* dirs) {
    for (size_t i = 0; i < file_system_size; ++i) {
        if (dirs[i] != -1) {
            close(dirs[i]);
        }
    }
}
void find(int i, int* dirs, struct dirent* new_file, fuse_fill_dir_t filler, void* out) {
    int not_have_next = 1;
    int j = i + 1;
    while (!not_have_next && j < file_system_size) {
        if (dirs[j] != -1) {
            int file = openat(dirs[j], new_file->d_name, O_RDONLY);
            if (-1 != file) {
                not_have_next = 0;
                close(file);
            }
        }
        ++j;
    }
    if (not_have_next) {
        filler(out, new_file->d_name, NULL, 0, 0);
    }
}

int my_readdir(const char* path, void* out, fuse_fill_dir_t filler, off_t off,
               struct fuse_file_info* fi, enum fuse_readdir_flags flags) {
    int new_file_system_dirs[file_system_size];
    opening(path, new_file_system_dirs);
    int s = sizeof(struct stat);
    struct stat* buffer = malloc(s);
    path = path + 1;
    fstatat(file_system_dirs[last(path - 1)], path, buffer, 0);
    if (S_ISREG(buffer->st_mode)) {
        return -ENOENT;
    }
    for (size_t i = 0; i < file_system_size; ++i) {
        if (new_file_system_dirs[i] == -1) {
            continue;
        }
        DIR* d = fdopendir(new_file_system_dirs[i]);
        if (d == NULL) {
            return 0;
        }
        struct dirent* new_file = readdir(d);
        while (NULL != new_file) {
            find(i, new_file_system_dirs, new_file, filler, out);
            new_file = readdir(d);
        }
    }
    free(buffer);
    closing(new_file_system_dirs);
    return 0;
}

int my_open(const char *path, struct fuse_file_info* fi) {
    if (O_RDONLY != (O_ACCMODE & fi->flags)) {
        return -EACCES;
    }
    return 0;
}

int my_read(const char *path, char *out, size_t size, off_t off, struct fuse_file_info *fi) {
    int s = 2048;
    char* buffer = malloc(s);
    int fd = openat(file_system_dirs[last(path)], path + 1,  O_RDONLY);
    if (fd != -1) {
        int current_size;
        if (-1 == (current_size = read(fd, buffer, s))) {
            return 0;
        }
        if (off > current_size) {
            return 0;
        }
        size = (current_size >= off + size) ? size : (current_size - off);
        char *buffer_offset = buffer + off;
        memcpy(out, buffer_offset, size);
        free(buffer);
        return size;
    } else {
        free(buffer);
        return -ENOENT;
    }
}

struct fuse_operations operations = {
        .open = my_open,
        .getattr = my_stat,
        .read = my_read,
        .readdir = my_readdir,
};

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_opt opt_specs[] = {
            { "--src %s", 0, 0},
            { NULL,       0, 0},
    };
    char* my_options = NULL;
    fuse_opt_parse(&args, &my_options, opt_specs, NULL);
    if (my_options != NULL) {
        char* begin = my_options;
        char* end = begin;
        char* name = malloc(256);
        while ((end = strchr(begin, ':')) != NULL) {
            if (*begin == '/') {
                ++begin;
            }
            memcpy(name, begin, end - begin);
            name[end - begin] = '\0';
            begin = end + 1;
            ++file_system_size;
            file_system_dirs[file_system_size - 1] = open(name, O_RDONLY);
        }
        if (*begin == '/') {
            ++begin;
        }
        end = strchr(begin, '\0');
        ++file_system_size;
        memcpy(name, begin, end - begin);
        name[end - begin] = '\0';
        file_system_dirs[file_system_size - 1] = open(name, O_RDONLY);
        free(name);
    }
    int ret = fuse_main(args.argc, args.argv, &operations, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
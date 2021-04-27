#define FUSE_USE_VERSION 30   // API version 3.0
#include <fuse.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

size_t count_of_dirs = 0;
int dirs[4096];

int choose_right_file(const char* path) {
    struct stat* right_st = malloc(sizeof(struct stat));
    memset(right_st, 0, sizeof(struct stat));

    int number_of_chosen_file = -1;
    for (size_t i = 0; i < count_of_dirs; ++i) {
        struct stat* buf = malloc(sizeof(struct stat));
        if (fstatat(dirs[i], path + 1, buf, 0) == 0) {
            if (right_st->st_mtime < buf->st_mtime) {
                memcpy(right_st, buf, sizeof(struct stat));
                number_of_chosen_file = i;
            }
        }
        free(buf);
    }
    free(right_st);
    return number_of_chosen_file;
}

int my_stat(const char* path, struct stat* st, struct fuse_file_info* fi) {
    if (strcmp("/", path) != 0) {
        int file;
        if ((file = choose_right_file(path)) == -1) {
            return -ENOENT;
        }
        fstatat(dirs[file], path + 1, st, 0);
        if (S_ISDIR(st->st_mode)) {
            st->st_mode = 0555 | S_IFDIR;
        } else {
            st->st_mode = 0444 | S_IFREG;
        }
    } else {
        st->st_nlink = 2;
        st->st_mode = 0555 | S_IFDIR;
    }
    return 0;
}

int check_dir(size_t i, int* current_dirs, void* out, fuse_fill_dir_t filler) {
    if (current_dirs[i] == -1) {
        return 0;
    }
    DIR* dir = fdopendir(current_dirs[i]);
    if (dir == NULL) {
        return 0;
    }
    struct dirent* current_file = readdir(dir);
    while (current_file != NULL) {
        int in_next_dirs = false;
        for (size_t j = i + 1; j < count_of_dirs && !in_next_dirs; ++j) {
            if (current_dirs[j] != -1) {
                int file;
                if ((file = openat(current_dirs[j], current_file->d_name, O_RDONLY)) != -1) {
                    in_next_dirs = true;
                    close(file);
                }
            }
        }
        if (!in_next_dirs) {
            filler(out, current_file->d_name, NULL, 0, 0);
        }
        current_file = readdir(dir);
    }
}

int my_readdir(const char* path, void* out, fuse_fill_dir_t filler, off_t off,
               struct fuse_file_info* fi, enum fuse_readdir_flags flags) {
    int current_dirs[count_of_dirs];
    for (size_t i = 0; i < count_of_dirs; ++i) {
        if (strcmp(path, "/") != 0) {
            current_dirs[i] = openat(dirs[i], path + 1, O_RDONLY);
        } else {
            current_dirs[i] = openat(dirs[i], ".", O_RDONLY);
        }
    }
    struct stat* buf = malloc(sizeof(struct stat));
    fstatat(dirs[choose_right_file(path)], path + 1, buf, 0);
    if (S_ISREG(buf->st_mode)) {
        return -ENOENT;
    }
    free(buf);
    for (size_t i = 0; i < count_of_dirs; ++i) {
        check_dir(i, current_dirs, out, filler);
    }

    for (int i = 0; i < count_of_dirs; ++i) {
        if (current_dirs[i] != -1) {
            close(current_dirs[i]);
        }
    }
    free(buf);
    return 0;
}

int my_open(const char *path, struct fuse_file_info* fi) {
    if (O_RDONLY != (fi->flags & O_ACCMODE)) {
        return -EACCES;
    }
    return 0;
}

int my_read(const char *path, char *out, size_t size, off_t off, struct fuse_file_info *fi) {
    char buf[4096];
    int fd;
    if ((fd = openat(dirs[choose_right_file(path)], path + 1,  O_RDONLY)) == -1) {
        return -ENOENT;
    }
    int current_size;
    if ((current_size = read(fd, buf, 4096)) == -1) {
        return 0;
    }
    if (off > current_size) {
        return 0;
    }
    if (current_size < off + size) {
        size = current_size - off;
    }
    memcpy(out, buf + off, size);
    return size;
}

struct fuse_operations operations = {
        .getattr = my_stat,
        .read = my_read,
        .open = my_open,
        .readdir = my_readdir,
};

char* add_new_dir(char* l, char* r) {
    char dir_name[4096];
    ++count_of_dirs;
    memcpy(dir_name, l, r - l);
    dir_name[r - l] = '\0';
    dirs[count_of_dirs - 1] = open(dir_name, O_RDONLY);
    return r + 1;
}

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_opt opt_specs[] = {
            { "--src %s", 0, 0},
            { NULL,       0, 0},
    };
    char* my_options = NULL;
    fuse_opt_parse(&args, &my_options, opt_specs, NULL);
    if (my_options != NULL) {
        char* l = my_options;
        char* r = strchr(l, ':');
        while (r != NULL) {
            if (*l == '/') {
                ++l;
            }
            l = add_new_dir(l, r);
            r = strchr(l, ':');
        }
        if (*l == '/') {
            ++l;
        }
        r = strchr(l, '\0');
        add_new_dir(l, r);
    }
    int ret = fuse_main(args.argc, args.argv, &operations, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
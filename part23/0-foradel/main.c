#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#define FUSE_USE_VERSION 30   // API version 3.0
#include <fuse.h>

typedef struct {
    int size;
    char filename[2048];
    char context[2048];
} data_t;

data_t files[2048];
int file_system_size = 0;

int my_open(const char *path, struct fuse_file_info* fi) {
    if (path[0] != '/') {
        return -ENOENT;
    }
    path = path + 1;
    for (size_t i = 0; i < file_system_size; ++i) {
        if (strcmp(path, files[i].filename) == 0) {
            if (O_RDONLY == (O_ACCMODE & fi->flags)) {
                return 0;
            }
            return -EACCES;
        }
    }
    return -ENOENT;
}

int my_stat(const char* path, struct stat* st, struct fuse_file_info* fi) {
    if (path[0] != '/') {
        return -ENOENT;
    }
    if (strcmp("/", path) == 0) {
        st->st_nlink = 2;
        st->st_mode = S_IFDIR | 0555;
        return 0;
    }
    path = path + 1;
    for (size_t i = 0; i < file_system_size; ++i) {
        if (strcmp(path, files[i].filename) == 0) {
            st->st_mode = S_IFREG | 0444;
            st->st_size = files[i].size;
            st->st_nlink = 1;
            return 0;
        }
    }
    return -ENOENT;
}

int my_read(const char *path, char *out, size_t size, off_t off, struct fuse_file_info *fi) {
    if (path[0] != '/') {
        return -ENOENT;
    }
    path = path + 1;
    for (size_t i = 0; i < file_system_size; ++i) {
        if (strcmp(path, files[i].filename) == 0) {
            if (off > files[i].size) {
                return 0;
            }
            size = (off + size > files[i].size) ? (files[i].size - off) : size;
            memcpy(out, files[i].context + off, size);
            return size;
        }
    }
    return -ENOENT;
}

int my_readdir(const char* path, void* out, fuse_fill_dir_t filler, off_t off,
               struct fuse_file_info* fi, enum fuse_readdir_flags flags) {
    if (0 != strcmp("/", path)) {
        return -ENOENT;
    }
    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);
    for (size_t i = 0; i < file_system_size; ++i) {
        filler(out, files[i].filename, NULL, 0, 0);
    }
    return 0;
}

struct fuse_operations operations = {
        .getattr = my_stat,
        .read = my_read,
        .open = my_open,
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
        int fd = open(my_options, O_RDONLY);
        if (fd != -1) {
            char buff[2048];
            read(fd, buff, 2048);
            char *pos = buff;
            sscanf(pos, "%d", &file_system_size);
            pos = strchr(pos, '\n');
            ++pos;
            int i = 0;
            while (i < file_system_size) {
                sscanf(pos, "%s %d", files[i].filename, &files[i].size);
                pos = strchr(pos, '\n');
                ++pos;
                ++i;
            }
            ++pos;
            i = 0;
            while (i < file_system_size) {
                memcpy(files[i].context, pos, files[i].size);
                pos += files[i].size;
                ++i;
            }
            close(fd);
        }
    }
    int ret = fuse_main(args.argc, args.argv, &operations, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
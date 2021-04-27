#define FUSE_USE_VERSION 30   // API version 3.0
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

typedef struct {
    char name[4096];
    char data[4096];
    size_t size;
} file_t;

size_t count_of_files = 0;
file_t files[4096];

int my_open(const char *path, struct fuse_file_info* fi) {
    if (path[0] != '/') {
        return -ENOENT;
    }
    ++path;
    for (size_t i = 0; i < count_of_files; ++i) {
        if (strcmp(path, files[i].name) == 0) {
            if ((fi->flags & O_ACCMODE) != O_RDONLY) {
                return -EACCES;
            }
            return 0;
        }
    }
    return -ENOENT;
}

int my_stat(const char* path, struct stat* st, struct fuse_file_info* fi) {
    if (path[0] != '/') {
        return -ENOENT;
    }
    if (strcmp("/", path) == 0) {
        st->st_mode = 0555 | S_IFDIR;
        st->st_nlink = 2;
        return 0;
    }
    ++path;
    for (size_t i = 0; i < count_of_files; ++i) {
        if (strcmp(path, files[i].name) == 0) {
            st->st_mode = S_IFREG | 0444;
            st->st_nlink = 1;
            st->st_size = files[i].size;
            return 0;
        }
    }
    return -ENOENT;
}

int my_read(const char *path, char *out, size_t size, off_t off, struct fuse_file_info *fi) {
    if (path[0] != '/') {
        return -ENOENT;
    }
    ++path;
    for (size_t i = 0; i < count_of_files; ++i) {
        if (strcmp(path, files[i].name) == 0) {
            if (off > files[i].size) {
                return 0;
            }
            if (off + size > files[i].size) {
                size = files[i].size - off;
            }
            const void *data = files[i].data + off;
            memcpy(out, data, size);
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
    for (size_t i = 0; i < count_of_files; ++i) {
        filler(out, files[i].name, NULL, 0, 0);
    }
    return 0;
}

struct fuse_operations operations = {
        .getattr = my_stat,
        .read = my_read,
        .open = my_open,
        .readdir = my_readdir,
};

char* read_name_and_size(char* pos) {
    for (size_t i = 0; i < count_of_files; ++i) {
        sscanf(pos, "%s %d", files[i].name, &files[i].size);
        pos = strchr(pos, '\n');
        ++pos;
    }
    return pos;
}

char* read_file_data(char* pos) {
    for (size_t i = 0; i < count_of_files; ++i) {
        memcpy(files[i].data, pos, files[i].size);
        pos += files[i].size;
    }
    return pos;
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
        int fd;
        if ((fd = open(my_options, O_RDONLY)) != -1) {
            char buff[4096];
            read(fd, buff, 4096);
            char *pos = buff;
            sscanf(pos, "%d", &count_of_files);
            pos = strchr(pos, '\n');
            ++pos;
            pos = read_name_and_size(pos) + 1;
            read_file_data(pos);
            close(fd);
        }
    }

    int ret = fuse_main(args.argc, args.argv, &operations, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
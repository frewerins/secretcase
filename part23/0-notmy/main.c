#define FUSE_USE_VERSION 30   // API version 3.0
#include <fuse.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_SIZE 4096

typedef struct {
    char filename[MAX_SIZE];
    size_t size;
    char content[MAX_SIZE];
} file_info_t;

size_t files_count = 0;
file_info_t files[MAX_SIZE];

int my_stat(const char* path, struct stat* st, struct fuse_file_info* fi) {
    // root directory
    if (0 == strcmp("/", path)) {
        st->st_mode = 0555 | S_IFDIR;
        st->st_nlink = 2;
        return 0;
    }

    // delete first symbol
    if (path[0] != '/') {
        return -ENOENT;
    }
    ++path;

    // files
    for (size_t i = 0; i < files_count; ++i) {
        if (0 == strcmp(path, files[i].filename)) {
            st->st_mode = S_IFREG | 0444;
            st->st_nlink = 1;
            st->st_size = files[i].size;
            return 0;
        }
    }

    return -ENOENT;
}

int my_readdir(const char* path, void* out, fuse_fill_dir_t filler, off_t off,
               struct fuse_file_info* fi, enum fuse_readdir_flags flags) {
    if (0 != strcmp("/", path)) {
        return -ENOENT;
    }

    // directory and its parent
    filler(out, ".", NULL, 0, 0);
    filler(out, "..", NULL, 0, 0);

    // files
    for (size_t i = 0; i < files_count; ++i) {
        filler(out, files[i].filename, NULL, 0, 0);
    }

    return 0;
}

int my_open(const char *path, struct fuse_file_info* fi) {
    if (path[0] != '/') {
        return -ENOENT;
    }
    ++path;
    for (size_t i = 0; i < files_count; ++i) {
        if (0 == strcmp(path, files[i].filename)) {
            if (O_RDONLY != (fi->flags & O_ACCMODE)) {
                return -EACCES;
            }
            return 0;
        }
    }

    return -ENOENT;
}

int my_read(const char *path, char *out, size_t size, off_t off,
            struct fuse_file_info *fi)
{
    if (path[0] != '/') {
        return -ENOENT;
    }
    ++path;
    for (size_t i = 0; i < files_count; ++i) {
        if (0 != strcmp(path, files[i].filename)) {
            continue;
        }
        if (off > files[i].size) {
            return 0;
        }
        if (off + size > files[i].size) {
            size = files[i].size - off;
        }
        const void* data = files[i].content + off;
        memcpy(out, data, size);
        return size;
    }
    return -ENOENT;
}

static struct fuse_operations operations = {
        .getattr = my_stat,
        .readdir = my_readdir,
        .open = my_open,
        .read = my_read,
};

void parse_files_contents(const char* src_file) {
    int fd = open(src_file, O_RDONLY);
    if (fd == -1) {
        return;
    }

    char buffer[MAX_SIZE];
    read(fd, buffer, MAX_SIZE);

    char *now_pos = buffer;

    sscanf(now_pos, "%zu", &files_count);
    now_pos = strchr(now_pos, '\n') + 1;

    for (size_t i = 0; i < files_count; ++i) {
        sscanf(now_pos, "%s %zu", files[i].filename, &files[i].size);
        now_pos = strchr(now_pos, '\n') + 1;
    }
    ++now_pos;

    for (size_t i = 0; i < files_count; ++i) {
        memcpy(files[i].content, now_pos, files[i].size);
        now_pos += files[i].size;
    }
}

int main(int argc, char *argv[]) {
    // arguments to be preprocessed before passing to /sbin/mount.fuse3
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    char* src = NULL;
    struct fuse_opt opt_specs[] = {
            { "--src %s", 0, 0},
            { NULL,       0, 0},
    };

    fuse_opt_parse(&args, &src, opt_specs, NULL);

    if (src) {
        parse_files_contents(src);
    }

    // run daemon
    int ret = fuse_main(
            args.argc, args.argv, // arguments to be passed to /sbin/mount.fuse3
            &operations,          // pointer to callback functions
            NULL                  // optional pointer to user-defined data
    );

    return ret;
}
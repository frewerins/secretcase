#define FUSE_USE_VERSION 30   // API version 3.0
#include <dirent.h>
#include <fuse.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_SIZE 4096

size_t dirs_count = 0;
int dir_fds[MAX_SIZE];

int with_best_mtime(const char* path) {
    int good = -1;
    struct stat* st = malloc(sizeof(struct stat));
    memset(st, 0, sizeof(struct stat));
    for (size_t i = 0; i < dirs_count; ++i) {
        struct stat* statbuf = malloc(sizeof(struct stat));
        if (0 != fstatat(dir_fds[i], path + 1, statbuf, 0)) {
            continue;
        }
        if (statbuf->st_mtime > st->st_mtime) {
            memcpy(st, statbuf, sizeof(struct stat));
            good = i;
        }
        free(statbuf);
    }
    free(st);
    return good;
}

int my_stat(const char* path, struct stat* st, struct fuse_file_info* fi) {
    if (0 == strcmp("/", path)) {
        st->st_mode = 0555 | S_IFDIR;
        st->st_nlink = 2;
        return 0;
    }

    int best = with_best_mtime(path);
    if (best == -1) {
        return -ENOENT;
    }

    fstatat(dir_fds[best], path + 1, st, 0);
    if (S_ISDIR(st->st_mode)) {
        st->st_mode = 0555 | S_IFDIR;
    } else {
        st->st_mode = 0444 | S_IFREG;
    }
    return 0;
}

int my_readdir(const char* path, void* out, fuse_fill_dir_t filler, off_t off,
               struct fuse_file_info* fi, enum fuse_readdir_flags flags) {
    // add to out directory and its parent

    int now_dir_fd[dirs_count];

    for (size_t i = 0; i < dirs_count; ++i) {
        if (0 == strcmp(path, "/")) {
            now_dir_fd[i] = openat(dir_fds[i], ".", O_RDONLY);
        } else {
            now_dir_fd[i] = openat(dir_fds[i], path + 1, O_RDONLY);
        }
    }

    // check that file is directory
    int best = with_best_mtime(path);
    struct stat* statbuf = malloc(sizeof(struct stat));
    fstatat(dir_fds[best], path + 1, statbuf, 0);
    if (S_ISREG(statbuf->st_mode)) {
        return -ENOENT;
    }
    free(statbuf);

    for (size_t i = 0; i < dirs_count; ++i) {
        if (now_dir_fd[i] == -1) {
            continue;
        }
        DIR* now_dir = fdopendir(now_dir_fd[i]);

        if (now_dir == NULL) {
            continue;
        }

        struct dirent* now_file;
        while ((now_file = readdir(now_dir)) != NULL) {
            int next_directories_have = 0;
            for (size_t j = i + 1; j < dirs_count; ++j) {
                if (now_dir_fd[j] == -1) {
                    continue;
                }
                int file = openat(now_dir_fd[j], now_file->d_name, O_RDONLY);
                if (-1 == file) {
                    continue;
                }
                next_directories_have = 1;
                close(file);
                break;
            }
            if (next_directories_have) {
                continue;
            }
            filler(out, now_file->d_name, NULL, 0, 0);
        }
    }

    for (int i = 0; i < dirs_count; ++i) {
        if (now_dir_fd[i] != -1) {
            close(now_dir_fd[i]);
        }
    }

    return 0;
}

int my_open(const char *path, struct fuse_file_info* fi) {
    if (O_RDONLY != (fi->flags & O_ACCMODE)) {
        return -EACCES;   // file system is read-only, so can't write
    }
    return 0;
}

int my_read(const char *path, char *out, size_t size, off_t off,
            struct fuse_file_info *fi)
{
    char buf[MAX_SIZE];
    int best = with_best_mtime(path);
    int fd = openat(dir_fds[best], path + 1,  O_RDONLY);
    if (fd == -1) {
        return -ENOENT;
    }
    int now_size = read(fd, buf, MAX_SIZE);
    if (now_size == -1) {
        return 0;
    }

    if (off > now_size) {
        return 0;
    }
    if (off + size > now_size) {
        size = now_size - off;
    }
    const void* data = buf + off;
    memcpy(out, data, size);
    return size;
}

static struct fuse_operations operations = {
        .getattr = my_stat,
        .readdir = my_readdir,
        .open = my_open,
        .read = my_read,
};

void parse_directories_names(const char* all_dirs) {
    char now_directory[MAX_SIZE];
    char* begin = (char*) all_dirs;
    char* end = NULL;

    while ((end = strchr(begin, ':')) != NULL) {
        if (*begin == '/') {
            ++begin;
        }
        size_t size = end - begin;
        memcpy(now_directory, begin, size);
        now_directory[size] = '\0';
        dir_fds[dirs_count] = open(now_directory, O_RDONLY);
        ++dirs_count, begin = end + 1;
    }

    if (*begin == '/') {
        ++begin;
    }
    end = strchr(begin, '\0');
    size_t size = end - begin;
    memcpy(now_directory, begin, size);
    now_directory[size] = '\0';
    dir_fds[dirs_count] = open(now_directory, O_RDONLY);
    ++dirs_count;
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
        parse_directories_names(src);
    }

    // run daemon
    int ret = fuse_main(
            args.argc, args.argv, // arguments to be passed to /sbin/mount.fuse3
            &operations,          // pointer to callback functions
            NULL                  // optional pointer to user-defined data
    );

    return ret;
}
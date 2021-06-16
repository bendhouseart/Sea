#include <string.h>
#include "functions.h"
#include "logger.h"
#include "config.h"
#include <stdlib.h>

#define DEFINE_SEAREADNEXT(VERSION, TYPE)                                                                                                          \
    struct TYPE *sea_readnext##VERSION(struct TYPE *d, config sea_conf, SEA_DIR *sd)                                                               \
    {                                                                                                                                              \
        log_msg(DEBUG, "sea_readnext%s: D is NULL %d Current index %d %s", #VERSION, d == NULL, sd->curr_index - 1, sd->dirnames[sd->curr_index]); \
        d = ((funcptr_readdir##VERSION)libc_readdir##VERSION)(sd->other_dirp[sd->curr_index - 1]);                                                 \
        log_msg(DEBUG, "sea_readnext%s: done reading", #VERSION);                                                                                  \
                                                                                                                                                   \
        if (d == NULL)                                                                                                                             \
        {                                                                                                                                          \
            log_msg(DEBUG, "sea_readnext%s: recently read d is NULL %d", #VERSION, d == NULL);                                                     \
            while (d == NULL && sd->curr_index + 1 < sd->total_dp)                                                                                 \
            {                                                                                                                                      \
                sd->curr_index++;                                                                                                                  \
                d = ((funcptr_readdir##VERSION)libc_readdir##VERSION)(sd->other_dirp[sd->curr_index - 1]);                                         \
            }                                                                                                                                      \
        }                                                                                                                                          \
        if (d != NULL)                                                                                                                             \
        {                                                                                                                                          \
            log_msg(DEBUG, "sea_readnext%s: read entry %s", #VERSION, d->d_name);                                                                  \
            if (sd->curr_index > 0 && d->d_type == DT_DIR)                                                                                         \
            {                                                                                                                                      \
                log_msg(DEBUG, "sea_readnext%s: Curr dir is a directory, reading next inode.", #VERSION);                                          \
                d = sea_readnext##VERSION(d, sea_conf, sd);                                                                                        \
            }                                                                                                                                      \
        }                                                                                                                                          \
        return d;                                                                                                                                  \
    }

#define DEFINE_READDIR(VERSION, TYPE)                                                                                                   \
    struct TYPE *readdir##VERSION(DIR *dirp)                                                                                            \
    {                                                                                                                                   \
        initialize_passthrough_if_necessary();                                                                                          \
        struct TYPE *d;                                                                                                                 \
        d = NULL;                                                                                                                       \
                                                                                                                                        \
        log_msg(INFO, "readdir%s: readdir%s started", #VERSION, #VERSION);                                                              \
        errno = 0;                                                                                                                      \
        config sea_conf = get_sea_config();                                                                                             \
                                                                                                                                        \
        log_msg(INFO, "readdir%s: passthrough initialized", #VERSION);                                                                  \
        if (sea_conf.parsed == true && sea_conf.n_sources > 1)                                                                          \
        {                                                                                                                               \
            initialize_sea_if_necessary();                                                                                              \
            log_msg(INFO, "readdir%s: sea initialized", #VERSION, #VERSION);                                                            \
                                                                                                                                        \
            SEA_DIR *sd = (SEA_DIR *)dirp;                                                                                              \
            if (sd->issea)                                                                                                              \
            {                                                                                                                           \
                log_msg(DEBUG, "readdir%s: reading dir and sea is enabled", #VERSION);                                                  \
                                                                                                                                        \
                log_msg(DEBUG, "readdir%s: reading sea directory1 %s", #VERSION, sd->dirnames[0]);                                      \
                if (sd->curr_index == 0)                                                                                                \
                {                                                                                                                       \
                    log_msg(DEBUG, "readdir%s: reading sea dir current idx = %d", #VERSION, sd->curr_index);                            \
                    d = ((funcptr_readdir##VERSION)libc_readdir##VERSION)(sd->dirp);                                                    \
                }                                                                                                                       \
                                                                                                                                        \
                log_msg(DEBUG, "readdir%s: curr_idx %d n_sources %d d NULL %d", #VERSION, sd->curr_index + 1, sd->total_dp, d == NULL); \
                if (sd->curr_index + 1 < sd->total_dp && d == NULL)                                                                     \
                {                                                                                                                       \
                    if (sd->curr_index == 0)                                                                                            \
                    {                                                                                                                   \
                        log_msg(DEBUG, "readdir%s: incrementing value of sd->curr_index++ to %d", #VERSION, sd->curr_index + 1);        \
                        sd->curr_index++;                                                                                               \
                    }                                                                                                                   \
                    log_msg(DEBUG, "readdir%s: launching sea_readnext %d %d", #VERSION, sd->curr_index, sd->total_dp);                  \
                    d = sea_readnext##VERSION(d, sea_conf, sd);                                                                         \
                }                                                                                                                       \
            }                                                                                                                           \
            else                                                                                                                        \
            {                                                                                                                           \
                log_msg(INFO, "readdir%s: non seadir %s", #VERSION, sd->dirnames[0]);                                                   \
                d = ((funcptr_readdir##VERSION)libc_readdir##VERSION)(sd->dirp);                                                        \
            }                                                                                                                           \
        }                                                                                                                               \
        else                                                                                                                            \
        {                                                                                                                               \
            log_msg(DEBUG, "readdir%s: passthrough", #VERSION);                                                                         \
            d = ((funcptr_readdir##VERSION)libc_readdir##VERSION)(dirp);                                                                \
        }                                                                                                                               \
        log_msg(DEBUG, "readdir%s: test end readdir1", #VERSION);                                                                       \
        if (d == NULL && errno)                                                                                                         \
            log_msg(ERROR, "readdir%s: failed to read null dir %d", #VERSION, errno);                                                   \
        else if (d != NULL)                                                                                                             \
            log_msg(INFO, "readdir%s: attempting to read %s", #VERSION, d->d_name);                                                     \
        else if (d != NULL && errno)                                                                                                    \
            log_msg(ERROR, "readdir%s: failed to read null dir %d", #VERSION, errno);                                                   \
        else if (d == NULL)                                                                                                             \
            log_msg(DEBUG, "readdir%s: returning null", #VERSION);                                                                      \
                                                                                                                                        \
        log_msg(INFO, "readdir%s: readdir%s ended", #VERSION, #VERSION);                                                                \
        return d;                                                                                                                       \
    }

DEFINE_SEAREADNEXT(, dirent);
DEFINE_SEAREADNEXT(64, dirent64);
DEFINE_READDIR(, dirent);
DEFINE_READDIR(64, dirent64);

/**
 * Function to setup passthrough/Sea environment and return corresponding passpath
 *
 * @param function the function name (only used for logging)
 * @param path the original path name
 * @param passpath the actual path for paths located in the mount directory
 * @param masked if the we should return a path from the view of the mount directory
 * @param nomatch return the original path if no match is found (necessary for functions take a directory fd as input)
 *
 */
void init_path(const char *function, const char *path, char passpath[PATH_MAX], int masked, int nomatch)
{
    initialize_passthrough_if_necessary();
    log_msg(DEBUG, "init_path");
    int match = 0;
    passpath[0] = '\0';

    if (!get_internal())
    {
        log_msg(INFO, "Entering %s with path: %s", function, path);

        struct config sea_conf = get_sea_config();
        if (sea_conf.parsed == true && sea_conf.n_sources > 1)
        {
            initialize_sea_if_necessary();
            match = sea_getpath(path, passpath, masked);
        }
        else
            match = pass_getpath(path, passpath, masked);

        if (match == 0 && nomatch)
        {
            strcpy(passpath, path);
        }

        log_msg(INFO, "Completing %s with updated path  %s", function, passpath);
    }
    else
    {
        strcpy(passpath, path);
    }
}

/**
 * Function to setup passthrough/Sea environment and return corresponding passpath.
 * Will return absolute path in passpath variable as long as path is not NULL or . or ..
 *
 * @param function the function name (only used for logging)
 * @param path the original path name
 * @param passpath the actual path for paths located in the mount directory
 * @param masked if the we should return a path from the view of the mount directory
 *
 */
void init_path(const char *function, const char *path, char passpath[PATH_MAX], int masked)
{
    init_path(function, path, passpath, masked, 0);
}

//TODO: maybe merge init_path with this one?
/**
 * Similar to init_path. Function to setup passthrough/Sea environment and return corresponding passpaths.
 * Necessary for functions that operate on two paths (e.g. rename and link)
 *
 * @param function the function name (only used for logging)
 * @param path the original path name
 * @param passpath the actual path for paths located in the mount directory
 * @param masked if the we should return a path from the view of the mount directory
 *
 */
void init_twopaths(const char *function, const char *oldpath, const char *newpath, char oldpasspath[PATH_MAX], char newpasspath[PATH_MAX], int nomatch)
{
    initialize_passthrough_if_necessary();

    if (!get_internal())
    {
        log_msg(INFO, "Entering %s with paths: %s %s", function, oldpath, newpath);
        struct config sea_conf = get_sea_config();
        int match_old = 0;
        int match_new = 0;
        if (sea_conf.parsed == true && sea_conf.n_sources > 1)
        {
            initialize_sea_if_necessary();
            match_old = sea_getpath(oldpath, oldpasspath, 0);
            match_new = sea_getpath(newpath, newpasspath, 0);
        }
        else
        {
            match_old = pass_getpath(oldpath, oldpasspath, 0);
            match_new = pass_getpath(newpath, newpasspath, 0);
        }

        if (match_old == 0 && nomatch)
            strcpy(oldpasspath, oldpath);
        if (match_new == 0 && nomatch)
            strcpy(newpasspath, newpath);

        log_msg(INFO, "Completing %s with updated paths %s %s", function, oldpasspath, newpasspath);
    }
    else
    {
        strcpy(oldpasspath, oldpath);
        strcpy(newpasspath, newpath);
    }
}

extern "C"
{

    int open(__const char *pathname, int flags, ...)
    {
        char passpath[PATH_MAX];

        mode_t mode;
        if (flags & O_CREAT)
        {
            va_list ap;
            va_start(ap, flags);
            mode = va_arg(ap, mode_t);
            va_end(ap);
        }

        init_path("open", pathname, passpath, 0);

        return ((funcptr_open)libc_open)(passpath, flags, mode);
    }

    int __open(__const char *pathname, int flags, ...)
    {
        char passpath[PATH_MAX];

        mode_t mode;
        if (flags & O_CREAT)
        {
            va_list ap;
            va_start(ap, flags);
            mode = va_arg(ap, mode_t);
            va_end(ap);
        }

        init_path("__open", pathname, passpath, 0);

        return ((funcptr___open)libc___open)(passpath, flags, mode);
    }

    int __open_2(const char *file, int oflag)
    {
        char passpath[PATH_MAX];

        init_path("__open_2", file, passpath, 0);

        return ((funcptr___open_2)libc___open_2)(passpath, oflag);
    }

    int open64(const char *pathname, int flags, ...)
    {
        char passpath[PATH_MAX];

        mode_t mode;
        if (flags & O_CREAT)
        {
            va_list ap;
            va_start(ap, flags);
            mode = va_arg(ap, mode_t);
            va_end(ap);
        }

        init_path("open64", pathname, passpath, 0);

        return ((funcptr_open64)libc_open64)(passpath, flags, mode);
    }

    int __open64(__const char *pathname, int flags, ...)
    {
        char passpath[PATH_MAX];

        mode_t mode;
        if (flags & O_CREAT)
        {
            va_list ap;
            va_start(ap, flags);
            mode = va_arg(ap, mode_t);
            va_end(ap);
        }

        init_path("__open64", pathname, passpath, 0);

        return ((funcptr___open64)libc___open64)(passpath, flags, mode);
    }

    int get_dirpath(const char *pathname, char *dirpath, int fd)
    {
        // check if seadir if we read relative to dirfd
        if ((pathname == NULL || pathname[0] != '/') && fd != AT_FDCWD)
        {
            log_msg(DEBUG, "get_dirpath: pathname %s", pathname);

            char fdloc[PATH_MAX];
            char afd[10];
            sprintf(afd, "%d", fd);

            strcpy(fdloc, fdpath);
            strcat(fdloc, "/");
            strcat(fdloc, afd);

            //struct stat sb;
            char linkname[PATH_MAX];
            ssize_t r;

            r = ((funcptr_readlink)libc_readlink)(fdloc, linkname, PATH_MAX + 1);

            log_msg(DEBUG, "get_dirpath: value of r %lu", r);

            if (r >= 0)
            {
                linkname[r] = '\0';
                strcpy(dirpath, linkname);

                if (pathname != NULL)
                {
                    strcat(dirpath, "/");
                    strcat(dirpath, pathname);
                }

                log_msg(DEBUG, "get_dir: updated name %s", dirpath);

                return 1;
            }
        }

        // is not a sea dir
        log_msg(DEBUG, "get_dirpath: is not a sea dir");

        if (pathname == NULL)
        {
            strcpy(dirpath, ".");
        }
        else
        {
            strcpy(dirpath, pathname);
        }
        return 0;
    }

    int openat(int dirfd, const char *pathname, int flags, ...)
    {
        initialize_passthrough_if_necessary();
        char passpath[PATH_MAX];
        char abspath[PATH_MAX];

        log_msg(DEBUG, "openat: dirfd %d", dirfd);

        get_dirpath(pathname, abspath, dirfd);
        init_path("openat", abspath, passpath, 0, 1);

        struct stat buffer;
        if (((funcptr___xstat)libc___xstat)(_STAT_VER_LINUX, passpath, &buffer) == 0)
        {

            if (S_ISDIR(buffer.st_mode))
            {
                log_msg(INFO, "openat: path is a dir %s", passpath);
                struct config sea_conf;
                sea_conf = get_sea_config();

                if (sea_conf.parsed == true && sea_conf.n_sources > 1)
                {
                    //char mountpath[PATH_MAX];
                    //char fdloc[PATH_MAX];
                    //char afd[10];
                    initialize_sea_if_necessary();

                    sea_getpath(abspath, passpath, 0, 0);
                    // source_match = sea_getpath(abspath, mountpath, 1);

                    log_msg(DEBUG, "openat: passpath is %s", passpath);
                    int main_fd = ((funcptr_openat)libc_openat)(dirfd, passpath, flags);
                    return main_fd;
                }
            }
        }
        return ((funcptr_openat)libc_openat)(dirfd, passpath, flags);
    }

    int openat64(int dirfd, const char *pathname, int flags, ...)
    {
        char passpath[PATH_MAX];
        init_path("openat64", pathname, passpath, 0);
        return ((funcptr_openat)libc_openat)(dirfd, passpath, flags);
    }

    int __openat64_2(int dirfd, const char *pathname, int flags)
    {
        char passpath[PATH_MAX];
        init_path("__openat64_2", pathname, passpath, 0);
        return ((funcptr_openat)libc_openat)(dirfd, passpath, flags);
    }

    extern int __openat_2(int dirfd, const char *pathname, int flags)
    {
        char passpath[PATH_MAX];
        init_path("__openat_2", pathname, passpath, 0);
        return ((funcptr_openat)libc_openat)(dirfd, passpath, flags);
    }

    int openat2(int dirfd, const char *pathname,
                struct open_how *how, size_t size)
    {
        char passpath[PATH_MAX];
        init_path("openat2", pathname, passpath, 0);
        return ((funcptr_openat2)libc_openat2)(dirfd, passpath, how, size);
    }

    //TODO: perhaps convert to use init_path. Leaving it for now
    DIR *opendir(const char *pathname)
    {
        initialize_passthrough_if_necessary();

        char passpath[PATH_MAX];
        char mountpath[PATH_MAX];

        struct config sea_conf;
        sea_conf = get_sea_config();

        //printf("parsed %d nsource %d\n", sea_conf.parsed, sea_conf.n_sources);
        if (sea_conf.parsed == true && sea_conf.n_sources > 1)
        {
            SEA_DIR *sd = (SEA_DIR *)malloc(sizeof(SEA_DIR));
            sd->issea = 0;
            sd->dirnames = (char **)malloc(sizeof(char *) * sea_conf.n_sources);
            sd->other_dirp = (DIR **)malloc(sizeof(DIR *) * sea_conf.n_sources - 1);

            initialize_sea_if_necessary();
            int mount_match = sea_getpath(pathname, passpath, 0, 0);
            //int source_match = sea_getpath(pathname, mountpath, 1);

            // if not a directory within the mountpoint or the source directories, don't need to create a SEA_DIR struct.
            if (mount_match == 0)
            {
                //printf("not in sea mount");
                log_msg(DEBUG, "opendir: %s not within Sea mount", passpath);
                sd->dirnames[0] = strdup(passpath);
                sd->dirp = ((funcptr_opendir)libc_opendir)(passpath);

                if (sd->dirp == NULL)
                    return sd->dirp;

                sd->total_dp = 1;
                return (DIR *)sd;
            }

            if (mount_match)
            {
                //printf("a mount match\n");
                // char *match;
                // char *seapath = strdup(pathname);

                // int len = strlen(pathname);

                // for (int i = 0; i < sea_conf.n_sources; ++i)
                // {
                //     if ((match = strstr(seapath, sea_conf.source_mounts[i])))
                //     {

                //         if (match != NULL && match[0] != '\0' && match[0] == '/')
                //         {
                //             printf("source mount %s seapath %s\n", sea_conf.source_mounts[i], seapath);
                //             seapath = strdup(sea_conf.source_mounts[0]);
                //             *match = '\0';
                //             strcat(seapath, match + len);
                //             break;
                //         }
                //     }
                // }

                strcpy(mountpath, pathname);
                //printf("new path %s\n", mountpath);
            }

            sd->curr_index = 0;
            sd->issea = 1;
            // indicator for if list is empty
            sd->other_dirp[0] = NULL;
            sd->total_dp = sea_conf.n_sources;
            sd->dirp = ((funcptr_opendir)libc_opendir)(passpath);

            if (sd->dirp == NULL)
                return sd->dirp;

            log_msg(INFO, "opened directory %s with fd %d", passpath, dirfd((DIR *)sd));

            sd->dirnames[0] = strdup(passpath);

            for (int i = 1; i < sea_conf.n_sources; i++)
            {
                passpath[0] = '\0';
                sea_getpath(mountpath, passpath, 0, i);
                log_msg(INFO, "opening other directory %s", passpath);
                sd->other_dirp[i - 1] = ((funcptr_opendir)libc_opendir)(strdup(passpath));
                sd->dirnames[i] = strdup(passpath);
                log_msg(INFO, "opened other mount directory %s at other_dirp index %d", passpath, i - 1);
                log_msg(DEBUG, "other_dirp at index %d is null %d", i - 1, sd->other_dirp[i - 1] == NULL);
            }
            return (DIR *)sd;
        }
        else
        {
            return ((funcptr_opendir)libc_opendir)(pathname);
        }
    }

    //TODO: need to fix such that it only uses C structs
    DIR *fdopendir(int fd)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "fdopendir %d", fd);

        struct config sea_conf;
        sea_conf = get_sea_config();

        char passpath[PATH_MAX];
        char abspath[PATH_MAX];
        char mountpath[PATH_MAX];

        log_msg(INFO, "fdopendir: getting directory path");
        get_dirpath(NULL, abspath, fd);
        log_msg(INFO, "fdopendir: obtained directory path %s", abspath);

        //file is not in Sea mount
        if (sea_conf.parsed == true && sea_conf.n_sources > 1)
        {
            initialize_sea_if_necessary();
            int mount_match = sea_getpath(abspath, passpath, 0, 0);
            int source_match = sea_getpath(abspath, mountpath, 1);

            SEA_DIR *sd = (SEA_DIR *)malloc(sizeof(SEA_DIR));
            sd->curr_index = 0;
            sd->dirnames = (char **)malloc(sizeof(char *) * sea_conf.n_sources);
            sd->other_dirp = (DIR **)malloc(sizeof(DIR *) * sea_conf.n_sources - 1);

            log_msg(INFO, "fdopendir: mount_match - %d ; source_match %d", mount_match, source_match);
            if (mount_match == 0 && source_match == 0)
            {
                sd->dirp = ((funcptr_fdopendir)libc_fdopendir)(fd);

                if (sd->dirp == NULL)
                    return sd->dirp;

                sd->dirnames[0] = strdup(passpath);
                sd->total_dp = 1;

                log_msg(INFO, "fdopendir: directory not in Sea %d %s", fd, sd->dirnames[0]);
                return (DIR *)sd;
            }
            if (mount_match)
                strcpy(mountpath, passpath);

            log_msg(INFO, "fdopendir: is a Sea dir %d", fd);

            sd->issea = 1;
            sd->dirp = ((funcptr_fdopendir)libc_fdopendir)(fd);

            if (sd->dirp == NULL)
            {
                log_msg(DEBUG, "fdopendir: sd->dirp is NULL");
                return sd->dirp;
            }

            sd->dirnames[0] = strdup(passpath);

            for (int i = 1; i < sea_conf.n_sources; i++)
            {
                passpath[0] = '\0';
                sea_getpath(mountpath, passpath, 0, i);
                sd->other_dirp[i - 1] = ((funcptr_opendir)libc_opendir)(strdup(passpath));
                sd->dirnames[i] = strdup(passpath);
                log_msg(INFO, "fdopendir: opened other mount directory %s at other_dirp index %d", passpath, i - 1);
                log_msg(DEBUG, "fdopendir: other_dirp at index %d is null %d", i - 1, sd->other_dirp[i - 1] == NULL);
            }

            sd->total_dp = sea_conf.n_sources;

            return (DIR *)sd;
        }
        else
        {
            log_msg(INFO, "fdopendir: not in Sea %d", fd);
            return ((funcptr_fdopendir)libc_fdopendir)(fd);
        }
    }

    DIR *__opendir(const char *pathname)
    {
        char passpath[PATH_MAX];
        init_path("__opendir", pathname, passpath, 0);
        return ((funcptr_opendir)libc_opendir)(passpath);
    }

    DIR *opendir2(const char *name, int flags)
    {
        char passpath[PATH_MAX];
        init_path("opendir2", name, passpath, 0);
        return ((funcptr_opendir)libc_opendir)(passpath);
    }

    DIR *__opendir2(const char *name, int flags)
    {
        char passpath[PATH_MAX];
        init_path("__opendir2", name, passpath, 0);
        return ((funcptr_opendir)libc_opendir)(passpath);
    }

    DIR *opendirat(int dfd, const char *name)
    {
        char passpath[PATH_MAX];
        init_path("opendirat", name, passpath, 0);
        return ((funcptr_opendir)libc_opendir)(passpath);
    }
    DIR *__opendirat(int dfd, const char *name)
    {
        char passpath[PATH_MAX];
        init_path("__opendirat", name, passpath, 0);
        return ((funcptr_opendir)libc_opendir)(passpath);
    }

    static DIR *__opendir_common(int fd, const char *name, int flags)
    {
        char passpath[PATH_MAX];
        init_path("__opendir_common", name, passpath, 0);
        return ((funcptr_opendir)libc_opendir)(passpath);
    }

    int scandir(const char *dirp, struct dirent ***namelist,
                int (*filter)(const struct dirent *),
                int (*compar)(const struct dirent **, const struct dirent **))
    {
        log_msg(INFO, "scandir");
        char passpath[PATH_MAX];
        init_path("scandir", dirp, passpath, 0);
        return ((funcptr_scandir)libc_scandir)(passpath, namelist, filter, compar);
    }

    int scandir64(const char *dirp, struct dirent64 ***namelist,
                  int (*select)(const struct dirent64 *),
                  int (*cmp)(const struct dirent64 **, const struct dirent64 **))
    {
        log_msg(INFO, "scandir64");
        char passpath[PATH_MAX];
        init_path("scandir64", dirp, passpath, 0);
        return ((funcptr_scandir64)libc_scandir64)(passpath, namelist, select, cmp);
    }

    //TODO: remove? definitely fix
    int scandirat(int dfd, const char *dir, struct dirent ***namelist,
                  int (*select)(const struct dirent *),
                  int (*cmp)(const struct dirent **, const struct dirent **))
    {
        log_msg(INFO, "scandirat");
        initialize_passthrough_if_necessary();
        log_msg(INFO, "scandirat test");
        return 0;
    }

    //TODO: remove? definitely fix
    int scandirat64(int dfd, const char *dir, struct dirent64 ***namelist,
                    int (*select)(const struct dirent64 *),
                    int (*cmp)(const struct dirent64 **, const struct dirent64 **))
    {
        log_msg(INFO, "scandirat");
        initialize_passthrough_if_necessary();
        log_msg(INFO, "scandirat64 test");
        return 0;
    }

#undef creat
    int creat(__const char *name, mode_t mode)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "creat %s", name);
        return open(name, O_CREAT | O_WRONLY | O_TRUNC, mode);
        //char passpath[PATH_MAX];
        //pass_getpath(name, passpath);
        //log_msg(INFO, "creat %s", name);
        //return ((funcptr_creat)libc_creat)(name, mode);
    }

    int creat64(__const char *name, mode_t mode)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "creat64 %s", name);
        return open(name, O_CREAT | O_WRONLY | O_TRUNC, mode);
        //char passpath[PATH_MAX];
        //pass_getpath(name, passpath);
        //log_msg(INFO, "creat64 %s", name);
        //return ((funcptr_creat64)libc_creat64)(name, mode);
    }

    int close(int fd)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "closing file");
        return ((funcptr_close)libc_close)(fd);
    }

    int closedir(DIR *dirp)
    {
        initialize_passthrough_if_necessary();

        config sea_conf = get_sea_config();

        if (sea_conf.parsed == true && sea_conf.n_sources > 1)
        {
            initialize_sea_if_necessary();
            SEA_DIR *sd = (SEA_DIR *)dirp;

            free(sd->dirnames[0]);
            if (sd->issea)
            {
                for (int i = 1; i < sd->total_dp; ++i)
                {
                    ((funcptr_closedir)libc_closedir)(sd->other_dirp[i - 1]);
                    free(sd->dirnames[i]);
                }
                free(sd->dirnames);
                free(sd->other_dirp);
            }

            int ret = 0;
            ret = ((funcptr_closedir)libc_closedir)(sd->dirp);

            free(sd);
            return ret;
        }
        log_msg(INFO, "closedir");
        return ((funcptr_closedir)libc_closedir)(dirp);
    }

    void rewinddir(DIR *dirp)
    {
        initialize_passthrough_if_necessary();

        log_msg(INFO, "rewinddir");
        config sea_conf = get_sea_config();

        if (sea_conf.parsed == true && sea_conf.n_sources > 1)
        {
            initialize_sea_if_necessary();
            SEA_DIR *sd = (SEA_DIR *)dirp;

            if (sd->issea)
            {
                for (int i = 1; i < sd->total_dp; ++i)
                {
                    ((funcptr_rewinddir)libc_rewinddir)(sd->other_dirp[i - 1]);
                }
            }

            ((funcptr_rewinddir)libc_rewinddir)(sd->dirp);
        }
        else
        {
            ((funcptr_rewinddir)libc_rewinddir)(dirp);
        }
    }

    ssize_t read(int fd, void *buf, size_t count)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "reading file of count %lu from fd %d", count, fd);

        size_t bytes = ((funcptr_read)libc_read)(fd, buf, count);
        return bytes;
    }

    ssize_t write(int fd, const void *buf, size_t count)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "writing file of count %lu from fd %d", count, fd);
        return ((funcptr_write)libc_write)(fd, buf, count);
    }

    ssize_t pread(int fd, void *buf, size_t count, off_t offset)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "preading file of count %lu", count);
        return ((funcptr_pread)libc_pread)(fd, buf, count, offset);
    }

    ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "pwriting file of count %lu", count);
        return ((funcptr_pwrite)libc_pwrite)(fd, buf, count, offset);
    }

    int dup(int oldfd)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "duplicating fd");
        return ((funcptr_dup)libc_dup)(oldfd);
    }

    int dup2(int oldfd, int newfd)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "duplicating 2 fd");
        return ((funcptr_dup2)libc_dup2)(oldfd, newfd);
    }

    off_t lseek(int fd, off_t offset, int whence)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "lseek");
        return ((funcptr_lseek)libc_lseek)(fd, offset, whence);
    }

    int mkdir(const char *pathname, mode_t mode)
    {
        char passpath[PATH_MAX];
        char mountpath[PATH_MAX];
        //init_path("mkdir", pathname, passpath, 0);
        //return ((funcptr_mkdir)libc_mkdir)(passpath, mode);
        initialize_passthrough_if_necessary();

        config sea_conf = get_sea_config();

        if (sea_conf.parsed && sea_conf.n_sources > 1)
        {
            initialize_sea_if_necessary();
            int mount_match = sea_getpath(pathname, passpath, 0, 0);
            int source_match = sea_getpath(pathname, mountpath, 1);

            // if not a directory within the mountpoint or the source directories open as normal
            if (mount_match == 0 && source_match == 0)
                return ((funcptr_mkdir)libc_mkdir)(passpath, mode);

            for (int i = 0; i < sea_conf.n_sources; ++i)
            {
                passpath[0] = '\0';
                sea_getpath(pathname, passpath, 0, i);
                log_msg(INFO, "creating directory %s", passpath);
                int fd = ((funcptr_mkdir)libc_mkdir)(passpath, mode);
                log_msg(INFO, "created directory %s with fd %d %d", passpath, fd, i);

                if (i == sea_conf.n_sources - 1)
                    return fd;
            }
            // is a sea path but something wenr wrong
            return -1;
        }
        else
            return ((funcptr_mkdir)libc_mkdir)(pathname, mode);
    }

    int mkdirat(int dirfd, const char *pathname, mode_t mode)
    {
        char passpath[PATH_MAX];
        init_path("mkdirat", pathname, passpath, 0);
        return ((funcptr_mkdirat)libc_mkdirat)(dirfd, passpath, mode);
    }

    int chdir(const char *pathname)
    {
        char passpath[PATH_MAX];
        init_path("chdir", pathname, passpath, 0);
        return ((funcptr_chdir)libc_chdir)(passpath);
    }

    int dirfd(DIR *dirp)
    {
        initialize_passthrough_if_necessary();
        config sea_conf = get_sea_config();

        if (sea_conf.parsed && sea_conf.n_sources > 1)
        {
            initialize_sea_if_necessary();

            SEA_DIR *sd = (SEA_DIR *)dirp;
            log_msg(DEBUG, "in dirfd4. returning %d", sd->dirp);
            return ((funcptr_dirfd)libc_dirfd)(sd->dirp);
        }
        else
        {
            log_msg(DEBUG, "dirfd not in sea");
            return ((funcptr_dirfd)libc_dirfd)(dirp);
        }
    }

    int rename(const char *oldpath, const char *newpath)
    {
        char oldpasspath[PATH_MAX];
        char newpasspath[PATH_MAX];
        init_twopaths("rename", oldpath, newpath, oldpasspath, newpasspath, 0);
        return ((funcptr_rename)libc_rename)(oldpasspath, newpasspath);
    }

    int renameat(int olddirfd, const char *oldpath, int newdirfd, const char *newpath)
    {
        char oldpasspath[PATH_MAX];
        char newpasspath[PATH_MAX];
        init_twopaths("renameat", oldpath, newpath, oldpasspath, newpasspath, 0);
        return ((funcptr_renameat)libc_renameat)(olddirfd, oldpasspath, newdirfd, newpasspath);
    }

    int renameat2(int olddirfd, const char *oldpath, int newdirfd, const char *newpath, unsigned int flags)
    {
        char oldpasspath[PATH_MAX];
        char newpasspath[PATH_MAX];
        init_twopaths("renameat2", oldpath, newpath, oldpasspath, newpasspath, 0);
        return ((funcptr_renameat2)libc_renameat2)(olddirfd, oldpasspath, newdirfd, newpasspath, flags);
    }

    int remove(const char *pathname)
    {
        char passpath[PATH_MAX];
        init_path("remove", pathname, passpath, 0);
        return ((funcptr_remove)libc_remove)(passpath);
    }

    int link(const char *oldname, const char *newname)
    {
        char old_passpath[PATH_MAX];
        char new_passpath[PATH_MAX];
        init_twopaths("link", oldname, newname, old_passpath, new_passpath, 0);
        return ((funcptr_link)libc_link)(old_passpath, new_passpath);
    }

    int linkat(int oldfd, const char *oldname, int newfd, const char *newname, int flags)
    {
        char old_passpath[PATH_MAX];
        char new_passpath[PATH_MAX];
        init_twopaths("linkat", oldname, newname, old_passpath, new_passpath, 0);
        return ((funcptr_linkat)libc_linkat)(oldfd, old_passpath, newfd, new_passpath, flags);
    }

    int unlink(const char *pathname)
    {
        char passpath[PATH_MAX];
        init_path("unlink", pathname, passpath, 0);
        return ((funcptr_unlink)libc_unlink)(passpath);
    }

    int unlinkat(int dirfd, const char *pathname, int flags)
    {
        initialize_passthrough_if_necessary();
        char passpath[PATH_MAX];
        char abspath[PATH_MAX];

        get_dirpath(pathname, abspath, dirfd);
        init_path("unlinkat", abspath, passpath, 0, 1);

        struct stat buffer;
        if (((funcptr___xstat)libc___xstat)(_STAT_VER_LINUX, passpath, &buffer) == 0)
        {

            if (S_ISDIR(buffer.st_mode))
            {
                log_msg(INFO, "unlinkat: path is a dir %s", passpath);

                struct config sea_conf;
                sea_conf = get_sea_config();

                if (sea_conf.parsed == true && sea_conf.n_sources > 1)
                {
                    char mountpath[PATH_MAX];
                    initialize_sea_if_necessary();
                    int mount_match = sea_getpath(abspath, passpath, 0, 0);
                    int source_match = sea_getpath(passpath, mountpath, 1);

                    // if not a directory within the mountpoint or the source directories, return just the current dir
                    if (mount_match == 0 && source_match == 0)
                    {

                        return ((funcptr_unlinkat)libc_unlinkat)(dirfd, passpath, flags);
                    }

                    if (mount_match)
                        strcpy(mountpath, abspath);

                    int ret = 0;
                    ret = ((funcptr_unlinkat)libc_unlinkat)(dirfd, passpath, flags);
                    for (int i = 0; i < sea_conf.n_sources; i++)
                    {
                        passpath[0] = '\0';
                        sea_getpath(mountpath, passpath, 0, i);
                        log_msg(INFO, "unlinking other directory %s", passpath);
                        ret = ((funcptr_unlinkat)libc_unlinkat)(dirfd, strdup(passpath), flags);
                    }
                    return ret;
                }
            }
        }
        return ((funcptr_unlinkat)libc_unlinkat)(dirfd, passpath, flags);
    }

    // fix and remove all relevant directories
    int rmdir(const char *pathname)
    {
        initialize_passthrough_if_necessary();
        char passpath[PATH_MAX];
        char abspath[PATH_MAX];

        init_path("rmdir", pathname, passpath, 0);

        struct config sea_conf;
        sea_conf = get_sea_config();

        if (sea_conf.parsed == true && sea_conf.n_sources > 1)
        {
            char mountpath[PATH_MAX];
            initialize_sea_if_necessary();
            int mount_match = sea_getpath(pathname, passpath, 0, 0);

            // if not a directory within the mountpoint or the source directories, return just the current dir
            if (mount_match == 0)
            {

                return ((funcptr_rmdir)libc_rmdir)(passpath);
            }

            if (mount_match)
                strcpy(mountpath, pathname);

            int ret = 0;
            ret = ((funcptr_rmdir)libc_rmdir)(passpath);
            for (int i = 1; i < sea_conf.n_sources; i++)
            {
                passpath[0] = '\0';
                sea_getpath(mountpath, passpath, 0, i);
                log_msg(INFO, "rmdir: removing other directory %s", passpath);
                ret = ((funcptr_rmdir)libc_rmdir)(passpath);
            }
            return ret;
        }
        return ((funcptr_rmdir)libc_rmdir)(passpath);
    }

    int access(const char *pathname, int mode)
    {
        char passpath[PATH_MAX];
        init_path("access", pathname, passpath, 0);
        return ((funcptr_access)libc_access)(passpath, mode);
    }

    int faccessat(int dirfd, const char *pathname, int mode, int flags)
    {
        char passpath[PATH_MAX];
        init_path("faccessat", pathname, passpath, 0, 1);
        return ((funcptr_faccessat)libc_faccessat)(dirfd, passpath, mode, flags);
    }

    int stat(const char *pathname, struct stat *statbuf)
    {
        char passpath[PATH_MAX];
        init_path("stat", pathname, passpath, 0);
        return ((funcptr_stat)libc_stat)(passpath, statbuf);
    }

    int lstat(const char *pathname, struct stat *statbuf)
    {
        char passpath[PATH_MAX];
        init_path("lstat", pathname, passpath, 0);
        return ((funcptr_lstat)libc_lstat)(passpath, statbuf);
    }

    int lstat64(const char *pathname, struct stat64 *statbuf)
    {
        char passpath[PATH_MAX];
        init_path("lstat64", pathname, passpath, 0);
        return ((funcptr_lstat64)libc_lstat64)(passpath, statbuf);
    }

    int fstat(int fd, struct stat *statbuf)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "fstat %d", fd);
        return ((funcptr_fstat)libc_fstat)(fd, statbuf);
    }

    int fstatat(int dirfd, char const *path, struct stat *statbuf, int flags)
    {
        char passpath[PATH_MAX];
        init_path("fstatat", path, passpath, 0, 1);
        return ((funcptr_fstatat)libc_fstatat)(dirfd, passpath, statbuf, flags);
    }

    int statvfs(const char *path, struct statvfs *buf)
    {
        char passpath[PATH_MAX];
        init_path("statvfs", path, passpath, 0);
        return ((funcptr_statvfs)libc_statvfs)(passpath, buf);
    }

    int __xstat(int ver, const char *path, struct stat *statbuf)
    {
        char passpath[PATH_MAX];
        init_path("__xstat", path, passpath, 0);
        return ((funcptr___xstat)libc___xstat)(ver, passpath, statbuf);
    }

    int _xstat(int ver, const char *path, struct stat *statbuf)
    {
        char passpath[PATH_MAX];
        init_path("_xstat", path, passpath, 0);
        return ((funcptr__xstat)libc__xstat)(ver, passpath, statbuf);
    }

    int __xstat64(int ver, const char *path, struct stat64 *statbuf)
    {
        char passpath[PATH_MAX];
        init_path("__xstat64", path, passpath, 0);
        return ((funcptr___xstat64)libc___xstat64)(ver, passpath, statbuf);
    }

    int __fxstat(int ver, int fd, struct stat *statbuf)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "fxstat %d", fd);
        return ((funcptr___fxstat)libc___fxstat)(ver, fd, statbuf);
    }

    int __fxstatat(int ver, int fd, const char *path, struct stat *statbuf, int flag)
    {
        initialize_passthrough_if_necessary();
        log_msg(DEBUG, "accessed fstatat");
        char passpath[PATH_MAX];
        char abspath[PATH_MAX];

        get_dirpath(path, abspath, fd);
        init_path("__fxstatat", abspath, passpath, 0, 1);
        return ((funcptr___fxstatat)libc___fxstatat)(ver, fd, passpath, statbuf, flag);
    }

    int __fxstat64(int ver, int fd, struct stat64 *statbuf)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "fxstat64 %d", fd);
        return ((funcptr___fxstat64)libc___fxstat64)(ver, fd, statbuf);
    }

    int __fxstatat64(int ver, int fd, const char *path, struct stat64 *statbuf, int flag)
    {
        char passpath[PATH_MAX];
        init_path("__fxstatat64", path, passpath, 0, 1);
        return ((funcptr___fxstatat64)libc___fxstatat64)(ver, fd, passpath, statbuf, flag);
    }

    int __lxstat(int ver, const char *path, struct stat *statbuf)
    {
        char passpath[PATH_MAX];
        init_path("__lxstat", path, passpath, 0);
        return ((funcptr___lxstat)libc___lxstat)(ver, passpath, statbuf);
    }

    int __lxstat64(int ver, const char *path, struct stat64 *statbuf)
    {
        char passpath[PATH_MAX];
        init_path("__lxstat64", path, passpath, 0);
        return ((funcptr___lxstat64)libc___lxstat64)(ver, passpath, statbuf);
    }

    int statfs(const char *path, struct statfs *buf)
    {
        char passpath[PATH_MAX];
        init_path("statfs", path, passpath, 0);
        return ((funcptr_statfs)libc_statfs)(passpath, buf);
    }

    //TODO: this function might need some fixing
    FILE *fopen(const char *path, const char *mode)
    {
        initialize_passthrough_if_necessary();
        if (!strcmp(path, get_config_file()))
        {
            // Disable functions because config parsing requires fopen
            initialize_functions();
            FILE *f = ((funcptr_fopen)libc_fopen)(path, mode);
            return f;
        }
        char passpath[PATH_MAX];
        init_path("fopen", path, passpath, 0);
        return ((funcptr_fopen)libc_fopen)(passpath, mode);
    }

    FILE *fopen64(const char *path, const char *mode)
    {
        char passpath[PATH_MAX];
        init_path("fopen64", path, passpath, 0);
        return ((funcptr_fopen64)libc_fopen64)(passpath, mode);
    }

    FILE *freopen(const char *path, const char *mode, FILE *stream)
    {
        char passpath[PATH_MAX];
        init_path("freopen", path, passpath, 0);
        return ((funcptr_freopen)libc_freopen)(passpath, mode, stream);
    }

    FILE *freopen64(const char *path, const char *mode, FILE *stream)
    {
        char passpath[PATH_MAX];
        init_path("freopen64", path, passpath, 0);
        return ((funcptr_freopen64)libc_freopen64)(passpath, mode, stream);
    }

    int truncate(const char *path, off_t offset)
    {
        char passpath[PATH_MAX];
        init_path("truncate", path, passpath, 0);
        return ((funcptr_truncate)libc_truncate)(passpath, offset);
    }

    //int truncate64(const char *path, off64_t offset){
    //    initialize_passthrough_if_necessary();
    //    char passpath[PATH_MAX];
    //    pass_getpath(path, passpath);
    //    log_msg(INFO, "truncate64 %s", passpath);
    //    return ((funcptr_truncate64)libc_truncate64)(passpath, offset);
    //}

    int ftruncate(int fd, off_t offset)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "ftruncate %d", fd);
        return ((funcptr_ftruncate)libc_ftruncate)(fd, offset);
    }

    int chown(const char *pathname, uid_t owner, gid_t group)
    {
        char passpath[PATH_MAX];
        init_path("chown", pathname, passpath, 0);
        return ((funcptr_chown)libc_chown)(passpath, owner, group);
    }

    int lchown(const char *pathname, uid_t owner, gid_t group)
    {
        char passpath[PATH_MAX];
        init_path("lchown", pathname, passpath, 0);
        return ((funcptr_lchown)libc_lchown)(passpath, owner, group);
    }

    int fchownat(int dirfd, const char *pathname, uid_t owner, gid_t group, int flags)
    {
        char passpath[PATH_MAX];
        init_path("fchownat", pathname, passpath, 0, 1);
        return ((funcptr_fchownat)libc_fchownat)(dirfd, passpath, owner, group, flags);
    }

    int chmod(const char *pathname, mode_t mode)
    {
        char passpath[PATH_MAX];
        init_path("chmod", pathname, passpath, 0);
        return ((funcptr_chmod)libc_chmod)(passpath, mode);
    }

    int fchmodat(int dirfd, const char *pathname, mode_t mode, int flags)
    {
        initialize_passthrough_if_necessary();
        log_msg(DEBUG, "accessed fstatat");
        char passpath[PATH_MAX];
        char abspath[PATH_MAX];

        get_dirpath(pathname, abspath, dirfd);
        init_path("fchmodat", pathname, passpath, 0, 1);
        return ((funcptr_fchmodat)libc_fchmodat)(dirfd, passpath, mode, flags);
    }

    int removexattr(const char *path, const char *name)
    {
        char passpath[PATH_MAX];
        init_path("removexattr", path, passpath, 0);
        return ((funcptr_removexattr)libc_removexattr)(passpath, name);
    }

    int lremovexattr(const char *path, const char *name)
    {
        char passpath[PATH_MAX];
        init_path("lremovexattr", path, passpath, 0);
        return ((funcptr_lremovexattr)libc_lremovexattr)(passpath, name);
    }

    ssize_t listxattr(const char *path, char *list, size_t size)
    {
        char passpath[PATH_MAX];
        init_path("listxattr", path, passpath, 0);
        return ((funcptr_listxattr)libc_listxattr)(passpath, list, size);
    }

    ssize_t llistxattr(const char *path, char *list, size_t size)
    {
        char passpath[PATH_MAX];
        init_path("llistxattr", path, passpath, 0);
        return ((funcptr_llistxattr)libc_llistxattr)(passpath, list, size);
    }

    ssize_t getxattr(const char *path, const char *name,
                     void *value, size_t size)
    {
        char passpath[PATH_MAX];
        init_path("getxattr", path, passpath, 0);
        return ((funcptr_lgetxattr)libc_getxattr)(passpath, name, value, size);
    }

    ssize_t lgetxattr(const char *path, const char *name,
                      void *value, size_t size)
    {
        char passpath[PATH_MAX];
        init_path("lgetxattr", path, passpath, 0);
        return ((funcptr_lgetxattr)libc_lgetxattr)(passpath, name, value, size);
    }

    int setxattr(const char *path, const char *name, const void *value, size_t size, int flags)
    {
        char passpath[PATH_MAX];
        init_path("setxattr", path, passpath, 0);
        return ((funcptr_setxattr)libattr_setxattr)(passpath, name, value, size, flags);
    }

    int fsetxattr(int fd, const char *name, const void *value, size_t size, int flags)
    {
        initialize_passthrough_if_necessary();
        log_msg(INFO, "fsetxattr %d", fd);
        return ((funcptr_fsetxattr)libattr_fsetxattr)(fd, name, value, size, flags);
    }

    const char *magic_file(magic_t cookie, const char *filename)
    {
        char passpath[PATH_MAX];
        init_path("magic_file", filename, passpath, 0);
        return ((funcptr_magic_file)libmagic_magic_file)(cookie, passpath);
    }

    int euidaccess(const char *pathname, int mode)
    {
        char passpath[PATH_MAX];
        init_path("euidaccess", pathname, passpath, 0);
        return ((funcptr_euidaccess)libc_euidaccess)(passpath, mode);
    }
    int eaccess(const char *pathname, int mode)
    {
        char passpath[PATH_MAX];
        init_path("eaccess", pathname, passpath, 0);
        return ((funcptr_eaccess)libc_eaccess)(passpath, mode);
    }

    void copy_last6(char *templ, char *passpath, int suffixlen)
    {
        // Copy the last 6 characters of passpath into the
        // last 6 characters of templ. Useful to copy the result
        // of pattern resolution (e.g., sedXXXXXX -> sed123456) in
        // mk*temp calls. If suffixlen is not 0, skips the last suffixlen
        // characters: for instance, the Xs in sedXXXXXX123 would be reached
        // with a suffixlen of 3.
        int len_temp = strlen(templ);
        int len_pass = strlen(passpath);
        for (int i = 0; i < 6; i++)
        {
            templ[len_temp - i - 1 - suffixlen] = passpath[len_pass - i - 1 - suffixlen];
        }
    }

    int mkstemp(char *templ)
    {
        char passpath[PATH_MAX];
        init_path("mkstemp", templ, passpath, 0);
        int ret = ((funcptr_mkstemp)libc_mkstemp)(passpath);
        copy_last6(templ, passpath, 0);
        return ret;
    }
    int mkstemp64(char *templ)
    {
        char passpath[PATH_MAX];
        init_path("mkstemp64", templ, passpath, 0);
        int ret = ((funcptr_mkstemp64)libc_mkstemp64)(passpath);
        copy_last6(templ, passpath, 0);
        return ret;
    }
    int mkostemp(char *templ, int flags)
    {
        char passpath[PATH_MAX];
        init_path("mkostemp", templ, passpath, 0);
        int ret = ((funcptr_mkostemp)libc_mkostemp)(passpath, flags);
        copy_last6(templ, passpath, 0);
        return ret;
    }
    int mkostemp64(char *templ, int flags)
    {
        char passpath[PATH_MAX];
        init_path("mkostemp64", templ, passpath, 0);
        templ = &passpath[0];
        int ret = ((funcptr_mkostemp64)libc_mkostemp64)(templ, flags);
        copy_last6(templ, passpath, 0);
        return ret;
    }
    int mkstemps(char *templ, int suffixlen)
    {
        char passpath[PATH_MAX];
        init_path("mkstemps", templ, passpath, 0);
        int ret = ((funcptr_mkstemps)libc_mkstemps)(passpath, suffixlen);
        copy_last6(templ, passpath, suffixlen);
        return ret;
    }
    int mkstemps64(char *templ, int suffixlen)
    {
        char passpath[PATH_MAX];
        init_path("mkstemps64", templ, passpath, 0);
        int ret = ((funcptr_mkstemps64)libc_mkstemps64)(passpath, suffixlen);
        copy_last6(templ, passpath, suffixlen);
        return ret;
    }
    int mkostemps(char *templ, int suffixlen, int flags)
    {
        char passpath[PATH_MAX];
        init_path("mkostemps", templ, passpath, 0);
        int ret = ((funcptr_mkostemps)libc_mkostemps)(passpath, suffixlen, flags);
        copy_last6(templ, passpath, suffixlen);
        return ret;
    }
    int mkostemps64(char *templ, int suffixlen, int flags)
    {
        char passpath[PATH_MAX];
        init_path("mkostemps64", templ, passpath, 0);
        int ret = ((funcptr_mkostemps64)libc_mkostemps64)(passpath, suffixlen, flags);
        copy_last6(templ, passpath, suffixlen);
        return ret;
    }

    FILE *setmntent(const char *file, const char *mode)
    {
        char passpath[PATH_MAX];
        init_path("setmntent", file, passpath, 0);
        return ((funcptr_setmntent)libc_setmntent)(passpath, mode);
    }

    char *bindtextdomain(const char *domainname, const char *dirname)
    {
        char passpath[PATH_MAX];
        init_path("bindtextdomain", dirname, passpath, 0);
        return ((funcptr_bindtextdomain)libc_bindtextdomain)(domainname, passpath);
    }

    int symlink(const char *oldname, const char *newname)
    {
        char old_passpath[PATH_MAX];
        char new_passpath[PATH_MAX];
        init_twopaths("symlink", oldname, newname, old_passpath, new_passpath, 0);
        return ((funcptr_symlink)libc_symlink)(old_passpath, new_passpath);
    }

    ssize_t readlink(const char *filename, char *buffer, size_t size)
    {
        char passpath[PATH_MAX];
        init_path("readlink", filename, passpath, 0);
        log_msg(INFO, "READLINK  %s %lu", passpath, size);
        return ((funcptr_readlink)libc_readlink)(passpath, buffer, size);
    }

    int nftw(const char *dirpath,
             int (*fn)(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf),
             int nopenfd, int flags)
    {

        char passpath[PATH_MAX];
        init_path("nftw", dirpath, passpath, 0);
        return ((funcptr_nftw)libc_nftw)(passpath, fn, nopenfd, flags);
    }

    int ftw(const char *dirpath,
            int (*fn)(const char *fpath, const struct stat *sb, int typeflag),
            int nopenfd)
    {
        char passpath[PATH_MAX];
        init_path("ftw", dirpath, passpath, 0);
        return ((funcptr_ftw)libc_ftw)(passpath, fn, nopenfd);
    }

    int name_to_handle_at(int dirfd, const char *pathname, struct file_handle *handle, int *mount_id, int flags)
    {
        char passpath[PATH_MAX];
        init_path("name_to_handle_at", pathname, passpath, 0, 1);
        return ((funcptr_name_to_handle_at)libc_name_to_handle_at)(dirfd, passpath, handle, mount_id, flags);
    }

    int chroot(const char *path)
    {
        char passpath[PATH_MAX];
        init_path("chroot", path, passpath, 0);
        return ((funcptr_chroot)libc_chroot)(passpath);
    }

    int execve(const char *pathname, char *const argv[], char *const envp[])
    {
        char passpath[PATH_MAX];
        init_path("execve", pathname, passpath, 0);
        return ((funcptr_execve)libc_execve)(passpath, argv, envp);
    }

    long int pathconf(const char *filename, int parameter)
    {
        char passpath[PATH_MAX];
        init_path("pathconf", filename, passpath, 0);
        return ((funcptr_pathconf)libc_pathconf)(passpath, parameter);
    }

    char *tempnam(const char *dir, const char *prefix)
    {
        char passpath[PATH_MAX];
        init_path("tempnam", dir, passpath, 0);
        return ((funcptr_tempnam)libc_tempnam)(passpath, prefix);
    }

    int mkfifo(const char *path, mode_t mode)
    {
        char passpath[PATH_MAX];
        init_path("mkfifo", path, passpath, 0);
        return ((funcptr_mkfifo)libc_mkfifo)(passpath, mode);
    }

    char *realpath(const char *path, char *resolved_path)
    {
        char passpath[PATH_MAX];
        init_path("realpath", path, passpath, 1);
        return ((funcptr_realpath)libc_realpath)(passpath, resolved_path);
    }

    char *canonicalize_file_name(const char *name)
    {
        char passpath[PATH_MAX];
        init_path("canonicalize_file_name", name, passpath, 1);
        return ((funcptr_canonicalize_file_name)libc_canonicalize_file_name)(passpath);
    }

    char *getcwd(char *buffer, size_t size)
    {

        initialize_passthrough_if_necessary();
        char *path;
        char passpath[PATH_MAX];
        passpath[0] = '\0';
        path = ((funcptr_getwd)libc_getcwd)(buffer, size);

        // get masked path rather than real path
        init_path("getcwd", path, passpath, 1);
        strcpy(path, passpath);
        //memset(path, '\0', strlen(passpath));
        //memmove(path, passpath, strlen(passpath));
        //path[strlen(passpath)] = '\0';
        //path[strlen(passpath) + 1] = '\0';

        // make sure there are no trailing characters after memmove

        log_msg(DEBUG, "in getcwd. converted %s %s", path, passpath);
        return path;
    }

    FILE *popen(const char *command, const char *type)
    {
        initialize_passthrough_if_necessary();
        log_msg(DEBUG, "popen: %s", command);
        return ((funcptr_popen)libc_popen)(command, type);
    }

    //int mknod(const char *path, mode_t mode, dev_t dev){
    //    initialize_passthrough_if_necessary();
    //    char passpath[PATH_MAX];
    //    pass_getpath(path, passpath);
    //    log_msg(INFO, "mknod %s", passpath);
    //    return ((funcptr_mknod)libc_mknod)(passpath, mode, dev);
    //}

    // removed as undefined
    //int execveat(int dirfd, const char *pathname,
    //                char *const argv[], char *const envp[],
    //                int flags){
    //    initialize_passthrough_if_necessary();
    //    char passpath[PATH_MAX];
    //    pass_getpath(pathname, passpath);
    //    log_msg(INFO, "execveat %s", passpath);
    //    return ((funcptr_execveat)libc_execveat)(dirfd, passpath, argv, envp, flags);
    //}

    //int fanotify_mark(int fanotify_fd, unsigned int flags,
    //                     uint64_t mask, int dirfd, const char *pathname){
    //    initialize_passthrough_if_necessary();
    //    char passpath[PATH_MAX];
    //    pass_getpath(pathname, passpath);
    //    log_msg(INFO, "fanotify_mark %s", passpath);
    //    return ((funcptr_fanotify_mark)libc_fanotify_mark)(fanotify_fd, flags, mask, dirfd, passpath);
    //}
}

// Note: don't really need but keeping it here just in case
//char* dirname(char* path){
//    initialize_passthrough_if_necessary();
//    char passpath[PATH_MAX];
//    pass_getpath(path, passpath);
//    log_msg(INFO, "dirname %s", passpath);
//    return ((funcptr_dirname)libc_dirname)(passpath);
//}

// Note: don't really need but keeping it here just in case
//const char* basename (const char *filename) throw() {
//    initialize_passthrough_if_necessary();
//    char passpath[PATH_MAX];
//    pass_getpath(filename, passpath);
//    log_msg(INFO, "basename %s", passpath);
//    return ((funcptr_basename)libc_basename)(passpath);
//}

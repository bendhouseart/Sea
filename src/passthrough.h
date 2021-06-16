/*
 * Copyright (c) 2014 by Matthias Noack, Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

#ifndef PRELOAD_PASSTHROUGH_H_
#define PRELOAD_PASSTHROUGH_H_

#include <stdio.h>

#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <magic.h>
//#include <sys/fanotify.h>

extern char *log_fn;

typedef int (*funcptr_creat)(const char *, mode_t);
typedef int (*funcptr_creat64)(const char *, mode_t);
typedef int (*funcptr_open)(const char *, int, mode_t);
typedef int (*funcptr___open)(const char *, int, mode_t);
typedef int (*funcptr___open_2)(const char *file, int oflag);
typedef int (*funcptr_open64)(const char *, int, mode_t);
typedef int (*funcptr___open64)(const char *, int, mode_t);
typedef int (*funcptr_openat)(int, const char *, int);
typedef DIR *(*funcptr_opendir)(const char *);
typedef DIR *(*funcptr_fdopendir)(int);
typedef int (*funcptr_scandir)(const char *dir, struct dirent ***namelist, int (*selector)(const struct dirent *), int (*cmp)(const struct dirent **, const struct dirent **));
typedef int (*funcptr_scandir64)(const char *dir, struct dirent64 ***namelist, int (*selector)(const struct dirent64 *), int (*cmp)(const struct dirent64 **, const struct dirent64 **));
typedef int (*funcptr_scandirat)(int dfd, const char *dir, struct dirent ***namelist, int (*select)(const struct dirent *), int (*cmp)(const struct dirent **, const struct dirent **));
typedef int (*funcptr_scandirat64)(int dfd, const char *dir, struct dirent64 ***namelist,
                                   int (*select)(const struct dirent64 *),
                                   int (*cmp)(const struct dirent64 **, const struct dirent64 **));
typedef int (*funcptr_close)(int);
typedef int (*funcptr_closedir)(DIR *);
typedef void (*funcptr_rewinddir)(DIR *);

typedef ssize_t (*funcptr_read)(int, void *, size_t);
typedef ssize_t (*funcptr_write)(int, const void *, size_t);
typedef ssize_t (*funcptr_pread)(int, void *, size_t, off_t);
typedef ssize_t (*funcptr_pwrite)(int, const void *, size_t, off_t);

typedef int (*funcptr_dup)(int);
typedef int (*funcptr_dup2)(int, int);
typedef off_t (*funcptr_lseek)(int, off_t, int);

typedef struct dirent *(*funcptr_readdir)(DIR *);
typedef struct dirent64 *(*funcptr_readdir64)(DIR *dirp);
typedef int (*funcptr_mkdir)(const char *, mode_t mode);
typedef int (*funcptr_mkdirat)(int, const char *, mode_t mode);
typedef int (*funcptr_chdir)(const char *);
typedef int (*funcptr_dirfd)(DIR *dirp);
typedef int (*funcptr_rename)(const char *, const char *);
typedef int (*funcptr_renameat)(int, const char *, int, const char *);
typedef int (*funcptr_renameat2)(int, const char *, int, const char *, unsigned int);
typedef int (*funcptr_remove)(const char *);
typedef int (*funcptr_unlink)(const char *);
typedef int (*funcptr_unlinkat)(int dirfd, char const *, int flags);
typedef int (*funcptr_rmdir)(const char *);

typedef int (*funcptr_linkat)(int, const char *, int, const char *, int);
typedef int (*funcptr_link)(const char *, const char *);

typedef int (*funcptr_access)(const char *, int);
typedef int (*funcptr_faccessat)(int, const char *, int, int);
typedef int (*funcptr_stat)(const char *, struct stat *);
typedef int (*funcptr_lstat)(const char *, struct stat *);
typedef int (*funcptr_lstat64)(const char *, struct stat64 *);
typedef int (*funcptr_fstat)(int, struct stat *);
typedef int (*funcptr_fstatat)(int dirfd, const char *, struct stat *buf, int flag);
typedef int (*funcptr_statvfs)(const char *, struct statvfs *);
typedef int (*funcptr___xstat)(int, const char *, struct stat *);
typedef int (*funcptr__xstat)(int, const char *, struct stat *);
typedef int (*funcptr___xstat64)(int, const char *, struct stat64 *);
typedef int (*funcptr___fxstat)(int, int, struct stat *);
typedef int (*funcptr___fxstatat)(int, int, const char *, struct stat *, int);
typedef int (*funcptr___fxstat64)(int, int, struct stat64 *);
typedef int (*funcptr___fxstatat64)(int, int, const char *, struct stat64 *, int);
typedef int (*funcptr___lxstat)(int, const char *, struct stat *);
typedef int (*funcptr___lxstat64)(int, const char *, struct stat64 *);
typedef int (*funcptr_statfs)(const char *, struct statfs *);

typedef FILE *(*funcptr_fopen)(const char *, const char *);
typedef FILE *(*funcptr_fopen64)(const char *, const char *);
typedef FILE *(*funcptr_freopen)(const char *, const char *, FILE *);
typedef FILE *(*funcptr_freopen64)(const char *, const char *, FILE *);
typedef int (*funcptr_truncate)(const char *, off_t);
typedef int (*funcptr_ftruncate)(int, off_t);

typedef int (*funcptr_chown)(const char *, uid_t, gid_t);
typedef int (*funcptr_lchown)(const char *, uid_t, gid_t);
typedef int (*funcptr_fchownat)(int, const char *, uid_t, gid_t, int);

typedef int (*funcptr_chmod)(const char *, mode_t);
typedef int (*funcptr_fchmodat)(int, const char *, mode_t, int);

typedef int (*funcptr_removexattr)(const char *, const char *);
typedef int (*funcptr_lremovexattr)(const char *, const char *);
typedef ssize_t (*funcptr_listxattr)(const char *, char *, size_t);
typedef ssize_t (*funcptr_llistxattr)(const char *, char *, size_t);
typedef ssize_t (*funcptr_getxattr)(const char *, const char *, void *, size_t);
typedef ssize_t (*funcptr_lgetxattr)(const char *, const char *, void *, size_t);
typedef int (*funcptr_setxattr)(const char *, const char *, const void *, size_t, int);
typedef int (*funcptr_fsetxattr)(int, const char *, const void *, size_t, int);

typedef const char *(*funcptr_magic_file)(magic_t, const char *);
typedef int (*funcptr_euidaccess)(const char *filename, int);
typedef int (*funcptr_eaccess)(const char *filename, int);

typedef int (*funcptr_mkstemp)(char *);
typedef int (*funcptr_mkostemp)(char *, int);
typedef int (*funcptr_mkstemps)(char *, int);
typedef int (*funcptr_mkostemps)(char *, int, int);

typedef int (*funcptr_mkstemp64)(char *);
typedef int (*funcptr_mkostemp64)(char *, int);
typedef int (*funcptr_mkstemps64)(char *, int);
typedef int (*funcptr_mkostemps64)(char *, int, int);

typedef FILE *(*funcptr_setmntent)(const char *, const char *);

typedef char *(*funcptr_basename)(const char *);

typedef char *(*funcptr_bindtextdomain)(const char *, const char *);

typedef int (*funcptr_symlink)(const char *, const char *);
typedef ssize_t (*funcptr_readlink)(const char *, char *, size_t);
typedef int (*funcptr_nftw)(const char *, int (*)(const char *, const struct stat *, int, struct FTW *),
                            int, int);
typedef int (*funcptr_ftw)(const char *, int (*)(const char *, const struct stat *, int), int);
typedef int (*funcptr_name_to_handle_at)(int, const char *, struct file_handle *, int *, int);
typedef int (*funcptr_chroot)(const char *);
typedef int (*funcptr_openat2)(int, const char *, struct open_how *, size_t);
typedef int (*funcptr_execve)(const char *, char *const[],
                              char *const[]);
typedef long int (*funcptr_pathconf)(const char *, int);
typedef char *(*funcptr_tempnam)(const char *, const char *);
typedef int (*funcptr_mkfifo)(const char *, mode_t);
typedef char *(*funcptr_realpath)(const char *, char *);
typedef char *(*funcptr_canonicalize_file_name)(const char *);
typedef char *(*funcptr_getwd)(char *, size_t);
//typedef int (*funcptr_mknod)(const char*, mode_t, dev_t);
//typedef int (*funcptr_execveat)(int, const char*, char *const[], char *const[], int);
//typedef int (*funcptr_fanotify_mark)(int, unsigned int, uint64_t, int, const char*);

typedef FILE *(*funcptr_popen)(const char *command, const char *type);

extern void *libc_creat;
extern void *libc_creat64;
extern void *libc_open;
extern void *libc___open;
extern void *libc___open_2;
extern void *libc_open64;
extern void *libc___open64;
extern void *libc_openat;
extern void *libc_opendir;
extern void *libc_fdopendir;
extern void *libc_scandir;
extern void *libc_scandir64;
extern void *libc_scandirat;
extern void *libc_scandirat64;
extern void *libc_close;
extern void *libc___close;
extern void *libc_closedir;
extern void *libc_rewinddir;
extern void *libc_pread;
extern void *libc_pwrite;
extern void *libc_read;
extern void *libc_write;
extern void *libc_dup;
extern void *libc_dup2;
extern void *libc_lseek;

extern void *libc_rename;
extern void *libc_renameat;
extern void *libc_renameat2;

extern void *libc_link;
extern void *libc_linkat;
extern void *libc_remove;
extern void *libc_unlink;
extern void *libc_unlinkat;
extern void *libc_rmdir;
extern void *libc_mkdir;
extern void *libc_mkdirat;
extern void *libc_chdir;
extern void *libc_dirfd;
extern void *libc_readdir;
extern void *libc_readdir64;

extern void *libc_access;
extern void *libc_faccessat;
extern void *libc_stat;
extern void *libc_lstat;
extern void *libc_lstat64;
extern void *libc_fstat;
extern void *libc_fstatat;
extern void *libc_statvfs;
extern void *libc___xstat;
extern void *libc__xstat;
extern void *libc___xstat64;
extern void *libc___fxstat;
extern void *libc___fxstatat;
extern void *libc___fxstat64;
extern void *libc___fxstatat64;
extern void *libc___lxstat;
extern void *libc___lxstat64;
extern void *libc_statfs;

extern void *libc_fopen;
extern void *libc_fopen64;
extern void *libc_freopen;
extern void *libc_freopen64;
extern void *libc_truncate;
extern void *libc_ftruncate;

extern void *libc_chown;
extern void *libc_lchown;
extern void *libc_fchownat;

extern void *libc_chmod;
extern void *libc_fchmodat;

extern void *libc_removexattr;
extern void *libc_lremovexattr;
extern void *libc_listxattr;
extern void *libc_llistxattr;
extern void *libc_getxattr;
extern void *libc_lgetxattr;

extern void *libattr_setxattr;
extern void *libattr_fsetxattr;

extern void *libmagic_magic_file;
extern void *libc_euidaccess;
extern void *libc_eaccess;

extern void *libc_mkstemp;
extern void *libc_mkstemp64;
extern void *libc_mkostemp;
extern void *libc_mkostemp64;
extern void *libc_mkstemps;
extern void *libc_mkstemps64;
extern void *libc_mkostemps;
extern void *libc_mkostemps64;

extern void *libc_setmntent;

extern void *libc_basename;

extern void *libc_bindtextdomain;

extern void *libc_symlink;
extern void *libc_readlink;
extern void *libc_nftw;
extern void *libc_ftw;
extern void *libc_name_to_handle_at;
extern void *libc_chroot;
extern void *libc_openat2;
extern void *libc_execve;
extern void *libc_pathconf;
extern void *libc_tempnam;
extern void *libc_mkfifo;
extern void *libc_realpath;
extern void *libc_canonicalize_file_name;
extern void *libc_getcwd;
extern void *libc_popen;
//extern void* libc_mknod;

//extern void* libc_execveat;
//extern void* libc_fanotify_mark;

void initialize_passthrough_if_necessary();
void initialize_functions();

char *make_file_name_canonical(char const *file_path);
int pass_getpath(const char *oldpath, char passpath[PATH_MAX], int masked_path);
int pass_getpath(const char *oldpath, char passpath[PATH_MAX], int masked_path, int sea_lvl);
void get_pass_canonical(char path[PATH_MAX], char passpath[PATH_MAX], char *mount_dir, char *sea_source, int masked_path);
int check_if_seapath(char path[PATH_MAX], char canonical[PATH_MAX], char passpath[PATH_MAX]);

#ifdef XTREEMFS_PRELOAD_QUIET
#define xprintf(...)
#else
#define xprintf(...) fprintf(xtreemfs_stdout() ? xtreemfs_stdout() : stdout, __VA_ARGS__)
#endif

FILE *xtreemfs_stdout();

#endif // PRELOAD_PASSTHROUGH_H_

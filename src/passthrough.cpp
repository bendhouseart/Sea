/*
 * Copyright (c) 2014 by Matthias Noack, Zuse Institute Berlin
 *
 * Licensed under the BSD License, see LICENSE file for details.
 *
 */

#include "logger.h"
#include "passthrough.h"
#include "config.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <errno.h>

void *libc;
void *libc_creat;
void *libc_creat64;
void *libc_open;
void *libc___open;
void *libc___open_2;
void *libc_open64;
void *libc___open64;
void *libc_openat;
void *libc_opendir;
void *libc_fdopendir;
void *libc_scandir;
void *libc_scandir64;
void *libc_scandirat;
void *libc_scandirat64;
void *libc_close;
void *libc___close;
void *libc_closedir;
void *libc_rewinddir;
void *libc_read;
void *libc_write;
void *libc_pread;
void *libc_pwrite;
void *libc_dup;
void *libc_dup2;
void *libc_lseek;
void *libc_rename;
void *libc_renameat;
void *libc_renameat2;
void *libc_link;
void *libc_linkat;
void *libc_remove;
void *libc_unlink;
void *libc_unlinkat;
void *libc_rmdir;
void *libc_readdir;
void *libc_readdir64;
void *libc_mkdir;
void *libc_mkdirat;
void *libc_chdir;
void *libc_dirfd;
void *libc_access;
void *libc_faccessat;
void *libc_stat;
void *libc_fstat;
void *libc_fstatat;
void *libc_lstat;
void *libc_lstat64;
void *libc_statvfs;
void *libc___xstat;
void *libc__xstat;
void *libc___xstat64;
void *libc___fxstat;
void *libc___fxstatat;
void *libc___fxstat64;
void *libc___fxstatat64;
void *libc___lxstat;
void *libc___lxstat64;
void *libc_statfs;

void *libc_fopen;
void *libc_fopen64;
void *libc_freopen;
void *libc_freopen64;
void *libc_truncate;
void *libc_ftruncate;

void *libc_chown;
void *libc_lchown;
void *libc_fchownat;

void *libc_chmod;
void *libc_fchmodat;

void *libc_removexattr;
void *libc_lremovexattr;
void *libc_listxattr;
void *libc_llistxattr;
void *libc_getxattr;
void *libc_lgetxattr;

void *libattr;
void *libattr_setxattr;
void *libattr_fsetxattr;

void *libmagic;
void *libmagic_magic_file;

void *libc_eaccess;
void *libc_euidaccess;

void *libc_mkstemp;
void *libc_mkstemp64;
void *libc_mkostemp;
void *libc_mkostemp64;
void *libc_mkstemps;
void *libc_mkstemps64;
void *libc_mkostemps;
void *libc_mkostemps64;

void *libc_setmntent;

void *libc_basename;

void *libc_bindtextdomain;

void *libc_symlink;
void *libc_readlink;
void *libc_nftw;
void *libc_ftw;
void *libc_name_to_handle_at;
void *libc_chroot;
void *libc_openat2;
void *libc_execve;
void *libc_pathconf;
void *libc_tempnam;
void *libc_mkfifo;
void *libc_realpath;
void *libc_canonicalize_file_name;
void *libc_getcwd;
//void* libc_mknod;
//void* libc_execveat;
//void* libc_fanotify_mark;
void *libc_popen;

// Our "copy" of stdout, because the application might close stdout
// or reuse the first file descriptors for other purposes.
static FILE *fdout = 0;

FILE *xtreemfs_stdout()
{
  return fdout;
}

// Taken from https://stackoverflow.com/questions/11034002/how-to-get-absolute-path-of-file-or-directory-that-does-not-exist
/**
 Return the input path in a canonical form. This is achieved by expanding all
 symbolic links, resolving references to "." and "..", and removing duplicate
 "/" characters.

 If the file exists, its path is canonicalized and returned. If the file,
 or parts of the containing directory, do not exist, path components are
 removed from the end until an existing path is found. The remainder of the
 path is then appended to the canonical form of the existing path,
 and returned. Consequently, the returned path may not exist. The portion
 of the path which exists, however, is represented in canonical form.

 If successful, this function returns a C-string, which needs to be freed by
 the caller using free().

   @param file_path File path, whose canonical form to return.

   @return On success, returns the canonical path to the file, which needs to be freed
   by the caller.

   On failure, returns NULL.
*/
char *make_file_name_canonical(char const *file_path)
{
  if (file_path == NULL)
  {
    log_msg(INFO, "In make_file_name_canonical NULL");
    return NULL;
  }
  else if (!strcmp(file_path, ""))
  {
    log_msg(INFO, "In make_file_name_canonical ''");
    return strdup("");
  }
  else if (strstr(file_path, "\n"))
  {
    log_msg(INFO, "In make_file_name_canonical '\\n'");
    return strdup("\n");
  }
  log_msg(INFO, "In make_file_name_canonical %s", file_path);

  char *canonical_file_path = (char *)malloc(sizeof(char) * PATH_MAX);
  unsigned int file_path_len = strlen(file_path);

  if (file_path_len > 0)
  {

    if (file_path[0] == '/')
    {
      return strdup(file_path);
    }
    canonical_file_path = ((funcptr_realpath)libc_realpath)(file_path, NULL);

    if (canonical_file_path == NULL && errno == ENOENT)
    {
      // The file was not found. Back up to a segment which exists,
      // and append the remainder of the path to it.
      char *file_path_copy = NULL;

      if ((strncmp(file_path, "./", 2) == 0) ||
          (strncmp(file_path, "../", 3) == 0))
      {
        // Path starts with "./" or "../"
        file_path_copy = strdup(file_path);
      }
      else
      {
        // Relative path
        file_path_copy = (char *)malloc(strlen(file_path) + 3);
        strcpy(file_path_copy, "./");
        strcat(file_path_copy, file_path);
      }

      // Remove path components from the end, until an existing path is found
      for (int char_idx = strlen(file_path_copy) - 1;
           char_idx >= 0 && canonical_file_path == NULL;
           --char_idx)
      {
        if (file_path_copy[char_idx] == '/')
        {
          // Remove the slash character
          file_path_copy[char_idx] = '\0';

          canonical_file_path = ((funcptr_realpath)libc_realpath)(file_path_copy, NULL);
          if (canonical_file_path != NULL)
          {
            // An existing path was found. Append the remainder of the path
            // to a canonical form of the existing path.
            char *combined_file_path = (char *)malloc(strlen(canonical_file_path) + strlen(file_path_copy + char_idx + 1) + 2);
            strcpy(combined_file_path, canonical_file_path);
            strcat(combined_file_path, "/");
            strcat(combined_file_path, file_path_copy + char_idx + 1);
            free(canonical_file_path);
            canonical_file_path = combined_file_path;
          }
          else
          {
            // The path segment does not exist. Replace the slash character
            // and keep trying by removing the previous path component.
            file_path_copy[char_idx] = '/';
          }
        }
        else if (file_path_copy[char_idx] == '\n')
        {
          // assumes file paths do not have newline characters
          return NULL;
        }
      }

      free(file_path_copy);
    }
    else if (canonical_file_path != NULL && strlen(canonical_file_path) == 1 && canonical_file_path[0] == '/')
    {
      return canonical_file_path;
    }
  }

  // realpath removes trailing slashes
  if (canonical_file_path != NULL)
  {
    int len = strlen(canonical_file_path);
    if (canonical_file_path[len - 1] == '/')
    {
      canonical_file_path[len - 1] = '\0';
    }
  }

  if (canonical_file_path != NULL)
    log_msg(INFO, "Canonical path %s", canonical_file_path);

  return canonical_file_path;
}

/**
 * Assign the proper canonical paths to path and passpath given the masked_path value.
 * If masked_path is 1, path will point to the real location of the file/directory whereas
 * passpath will point to the "mounted" location, and vice-versa.
 *
 * @param path the path of the file that should be matched with the path provided by the user
 * @param passpath the path returned to the user
 * @param mount_dir the canonical path of the library's mountpoint
 * @param sea_source the source directory for all the files created within the mountpoint
 * @param masked_path whether to return the mounted path or the real path to the user.
 *
 */
void get_pass_canonical(char path[PATH_MAX], char passpath[PATH_MAX], char *mount_dir, char *sea_source, int masked_path)
{

  if (masked_path == 1)
  {
    strcpy(path, sea_source);
    strcpy(passpath, mount_dir);
  }
  else
  {
    strcpy(path, mount_dir);
    strcpy(passpath, sea_source);
  }

  log_msg(DEBUG, "get pass canonical %s %s %d", passpath, mount_dir, masked_path);
}

/**
 * Determine whether the user provided path is located within the mountpoint or not. Store
 * the absolute "real" path in passpath. Return 1 if it is a mounted path, otherwise 0.
 *
 * @param path the location of the mountpoint
 * @param canonical the canonical version of the path provided by the user
 * @param passpath the canonical version of the real path for which the remainder of the user path will be appended to.
 *
 */
int check_if_seapath(char path[PATH_MAX], char canonical[PATH_MAX], char passpath[PATH_MAX])
{
  int len = strlen(path);
  char *match;
  int match_found = 0;

  //printf("canonical %s path %s passpath %s\n", canonical, path, passpath);

  if (canonical != NULL && path != NULL)
  {
    log_msg(DEBUG, "checking for match between %s and %s", canonical, path);
    if ((match = strstr(canonical, path)))
    {

      if (match == NULL || match[0] != '/')
        log_msg(DEBUG, "match null");

      if (match[0] == '/')
      {
        log_msg(DEBUG, "match %s", passpath);
        *match = '\0';
        strcat(passpath, match + len);
        match_found = 1;
      }
    }
    else
    {
      log_msg(DEBUG, "no match");
      strcpy(passpath, canonical);
    }
  }

  return match_found;
}

int pass_getpath(const char *oldpath, char passpath[PATH_MAX], int masked_path)
{
  return pass_getpath(oldpath, passpath, masked_path, 0);
}

/**
 * Return either the true absolute path or the masked mounted path to the user.
 *
 * @param oldpath user-provided path
 * @param passpath path to be returned to the user
 * @param masked_path 1 to return to the the masked mounted path or 0 to return the true path (in passpath)
 * @param sea_lvl the level within the hierarchy to use as the source path
 */
int pass_getpath(const char *oldpath, char passpath[PATH_MAX], int masked_path, int sea_lvl)
{

  if (oldpath == NULL)
    return 0;

  char *canonical;
  char path[PATH_MAX];

  // Get config
  struct config sea_config = get_sea_config();
  char *mount_dir = sea_config.mount_dir;

  canonical = make_file_name_canonical(oldpath);

  int match_found = 0;

  //log_msg(DEBUG, "oldpath: %s, actualpath: %s, mount_dir: %s", oldpath, actualpath, mount_dir);

  get_pass_canonical(path, passpath, mount_dir, sea_config.source_mounts[sea_lvl], masked_path);
  match_found = check_if_seapath(path, canonical, passpath);

  // check if there's a match with a source if no matches found
  // if (match_found == 0)
  // {
  //   for (int i = 0; i < sea_config.n_sources; i++)
  //   {
  //     get_pass_canonical(path, passpath, sea_config.source_mounts[i], sea_config.source_mounts[sea_lvl], masked_path);
  //     match_found = check_if_seapath(path, canonical, passpath);

  //     if (match_found == 1)
  //       break;
  //   }
  // }
  //if (canonical != NULL)
  //  free(canonical);
  if (passpath == NULL)
    return 0;

  //printf("sea_lvl=%d  :   old fn %s ---> new fn %s\n", sea_lvl, oldpath, passpath);
  log_msg(INFO, "sea_lvl=%d  :   old fn %s ---> new fn %s", sea_lvl, oldpath, passpath);
  return match_found;
}

void initialize_functions()
{

  libc = dlopen("libc.so.6", RTLD_LAZY); // TODO: link with correct libc, version vs. 32 bit vs. 64 bit
  libc_creat = dlsym(libc, "creat");
  libc_creat64 = dlsym(libc, "creat64");
  libc_open = dlsym(libc, "open");
  libc___open = dlsym(libc, "__open");
  libc___open_2 = dlsym(libc, "__open_2");
  libc_open64 = dlsym(libc, "open64");
  libc___open64 = dlsym(libc, "__open64");
  libc_openat = dlsym(libc, "openat");
  libc_opendir = dlsym(libc, "opendir");
  libc_fdopendir = dlsym(libc, "fdopendir");
  libc_scandir = dlsym(libc, "scandir");
  libc_scandir64 = dlsym(libc, "scandir64");
  libc_scandir = dlsym(libc, "scandir");
  libc_scandir64 = dlsym(libc, "scandir64");
  libc_scandirat = dlsym(libc, "scandirat");
  libc_scandirat64 = dlsym(libc, "scandirat64");
  libc_close = dlsym(libc, "close");
  libc___close = dlsym(libc, "__close");
  libc_closedir = dlsym(libc, "closedir");
  libc_rewinddir = dlsym(libc, "rewinddir");
  libc_read = dlsym(libc, "read");
  libc_write = dlsym(libc, "write");
  libc_pread = dlsym(libc, "pread");
  libc_pwrite = dlsym(libc, "pwrite");
  libc_dup = dlsym(libc, "dup");
  libc_dup2 = dlsym(libc, "dup2");
  libc_lseek = dlsym(libc, "lseek");

  libc_readdir = dlsym(libc, "readdir");
  libc_readdir64 = dlsym(libc, "readdir64");
  libc_mkdir = dlsym(libc, "mkdir");
  libc_mkdirat = dlsym(libc, "mkdirat");
  libc_chdir = dlsym(libc, "chdir");
  libc_dirfd = dlsym(libc, "dirfd");
  libc_rename = dlsym(libc, "rename");
  libc_renameat = dlsym(libc, "renameat");
  libc_renameat2 = dlsym(libc, "renameat2");
  libc_remove = dlsym(libc, "remove");
  libc_unlink = dlsym(libc, "unlink");
  libc_unlinkat = dlsym(libc, "unlinkat");
  libc_rmdir = dlsym(libc, "rmdir");
  libc_link = dlsym(libc, "link");
  libc_linkat = dlsym(libc, "linkat");

  libc_access = dlsym(libc, "access");
  libc_faccessat = dlsym(libc, "faccessat");
  libc_stat = dlsym(libc, "stat");
  libc_lstat = dlsym(libc, "lstat");
  libc_lstat64 = dlsym(libc, "lstat64");
  libc_fstat = dlsym(libc, "fstat");
  libc_fstatat = dlsym(libc, "fstatat");
  libc_statvfs = dlsym(libc, "statvfs");
  libc___xstat = dlsym(libc, "__xstat");
  libc__xstat = dlsym(libc, "_xstat");
  libc___xstat64 = dlsym(libc, "__xstat64");
  libc___fxstat = dlsym(libc, "__fxstat");
  libc___fxstatat = dlsym(libc, "__fxstatat");
  libc___fxstat64 = dlsym(libc, "__fxstat64");
  libc___fxstatat64 = dlsym(libc, "__fxstatat64");
  libc___lxstat = dlsym(libc, "__lxstat");
  libc___lxstat64 = dlsym(libc, "__lxstat64");
  libc_statfs = dlsym(libc, "statfs");

  libc_fopen = dlsym(libc, "fopen");
  libc_fopen64 = dlsym(libc, "fopen64");
  libc_freopen = dlsym(libc, "freopen");
  libc_freopen64 = dlsym(libc, "freopen64");
  libc_truncate = dlsym(libc, "truncate");
  libc_ftruncate = dlsym(libc, "ftruncate");

  libc_chown = dlsym(libc, "chown");
  libc_lchown = dlsym(libc, "lchown");
  libc_fchownat = dlsym(libc, "fchownat");

  libc_chmod = dlsym(libc, "chmod");
  libc_fchmodat = dlsym(libc, "fchmodat");

  libc_removexattr = dlsym(libc, "removexattr");
  libc_lremovexattr = dlsym(libc, "lremovexattr");
  libc_listxattr = dlsym(libc, "listxattr");
  libc_llistxattr = dlsym(libc, "llistxattr");
  libc_getxattr = dlsym(libc, "getxattr");
  libc_lgetxattr = dlsym(libc, "lgetxattr");

  libattr = dlopen("libattr.so.1", RTLD_LAZY);
  libattr_setxattr = dlsym(libattr, "setxattr");
  libattr_fsetxattr = dlsym(libattr, "setxattr");

  //added libmagic
  libmagic = dlopen("libmagic.so.1", RTLD_LAZY);
  libmagic_magic_file = dlsym(libmagic, "magic_file");

  libc_eaccess = dlsym(libc, "eaccess");
  libc_euidaccess = dlsym(libc, "euidaccess");

  libc_mkstemp = dlsym(libc, "mkstemp");
  libc_mkstemp64 = dlsym(libc, "mkstemp64");
  libc_mkostemp = dlsym(libc, "mkostemp");
  libc_mkostemp64 = dlsym(libc, "mkostemp64");
  libc_mkstemps = dlsym(libc, "mkstemps");
  libc_mkstemps64 = dlsym(libc, "mkstemps64");
  libc_mkostemps = dlsym(libc, "mkostemps");
  libc_mkostemps64 = dlsym(libc, "mkostemps64");

  libc_setmntent = dlsym(libc, "setmntent");

  libc_basename = dlsym(libc, "basename");

  libc_bindtextdomain = dlsym(libc, "bindtextdomain");

  libc_symlink = dlsym(libc, "symlink");
  libc_readlink = dlsym(libc, "readlink");
  libc_nftw = dlsym(libc, "nftw");
  libc_ftw = dlsym(libc, "ftw");
  libc_name_to_handle_at = dlsym(libc, "name_to_handle_at");
  libc_chroot = dlsym(libc, "chroot");
  libc_openat2 = dlsym(libc, "openat2");
  libc_execve = dlsym(libc, "execve");
  libc_pathconf = dlsym(libc, "pathconf");
  libc_tempnam = dlsym(libc, "tempnam");
  libc_mkfifo = dlsym(libc, "mkfifo");
  libc_realpath = dlsym(libc, "realpath");
  libc_canonicalize_file_name = dlsym(libc, "canonicalize_file_name");
  libc_getcwd = dlsym(libc, "getcwd");
  libc_popen = dlsym(libc, "popen");
  //libc_mknod = dlsym(libc, "mknod");
  //libc_execveat = dlsym(libc, "execveat"); -- function not defined
  //libc_fanotify_mark = dlsym(libc, "fanotify_mark");

  int stdout2 = ((funcptr_dup)libc_dup)(1);

  if (stdout2 != -1)
  {
    fdout = fdopen(stdout2, "a");
  }

  // Print the last error that occurred in dlsym, if any
  char *error = dlerror();
  if (error)
    xprintf("dlerror: %s\n", dlerror());
}

static void initialize_passthrough()
{
  initialize_functions();
}

static pthread_once_t passthrough_initialized = PTHREAD_ONCE_INIT;

void initialize_passthrough_if_necessary()
{
  pthread_once(&passthrough_initialized, initialize_passthrough);
}

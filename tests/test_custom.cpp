#include <gtest/gtest.h>
#include "../src/passthrough.h"
#include "../src/functions.h"
#include "../src/sea.h"
#include "../src/config.h"
#include <stdlib.h>

// helpers
void close_seadir(SEA_DIR *sd)
{
    ((funcptr_closedir)libc_closedir)(sd->dirp);

    for (int i = 1; i < sd->total_dp; i++)
    {
        ((funcptr_closedir)libc_closedir)(sd->other_dirp[i - 1]);
    }
}

// need to add symlink tests here
TEST(Passthrough, GetCanonical)
{
    const char paths[6][PATH_MAX] = {{"mount"}, {"./mount/subdir"}, {"../mount/"}, {"/dev/shm/tmp"}, {"/"}, {NULL}};

    for (int i = 0; i < 6; i++)
    {
        const char *path = paths[i];
        char *actualpath;
        char *expected = (char *)malloc(sizeof(char *) * PATH_MAX);

        actualpath = make_file_name_canonical(path);
        realpath(path, expected);

        if (path == NULL || path[0] == '\0')
        {
            if (actualpath == expected)
            {
                ASSERT_TRUE(true);
            }
        }
        else
            ASSERT_STREQ(actualpath, expected);

        free(expected);
    }
}

TEST(Passthrough, GetPassCanonical)
{
    char path[PATH_MAX];
    char passpath[PATH_MAX];

    char *mount_dir = (char *)malloc(sizeof(char *) * PATH_MAX);
    char *source_0 = (char *)malloc(sizeof(char *) * PATH_MAX);
    char *source_1 = (char *)malloc(sizeof(char *) * PATH_MAX);

    realpath("../mount", mount_dir);
    realpath("../source", source_0);
    realpath("/dev/shm/source", source_1);

    get_pass_canonical(path, passpath, mount_dir, source_0, 0);

    ASSERT_STREQ(passpath, source_0);
    ASSERT_STREQ(path, mount_dir);

    get_pass_canonical(path, passpath, mount_dir, source_1, 0);

    ASSERT_STREQ(passpath, source_1);
    ASSERT_STREQ(path, mount_dir);

    get_pass_canonical(path, passpath, mount_dir, source_0, 1);
    ASSERT_STREQ(passpath, mount_dir);
    ASSERT_STREQ(path, source_0);
}

TEST(Passthrough, CheckIfSeaPath)
{

    char oldpaths[4][PATH_MAX] = {"mount/test.txt", "mount/subdir/test.txt", "/dev/shm/test.txt", "/"};
    char expaths[4][PATH_MAX] = {"source/test.txt", "source/subdir/test.txt", "/dev/shm/test.txt", "/"};
    char path[PATH_MAX];

    ((funcptr_realpath)libc_realpath)("mount", path);

    for (int i = 0; i < 4; i++)
    {
        char *op = oldpaths[i];
        // need to reinitialize each time otherwise may retain tail of previous value
        char passpath[PATH_MAX];
        ((funcptr_realpath)libc_realpath)("source", passpath);

        char *actualpath;
        char expected[PATH_MAX];
        actualpath = make_file_name_canonical(op);
        ((funcptr_realpath)libc_realpath)(expaths[i], expected);
        int res = check_if_seapath(path, actualpath, passpath);

        if (i < 2)
            ASSERT_EQ(res, 1);
        else
            ASSERT_EQ(res, 0);

        ASSERT_STREQ(passpath, expected);
    }
}

TEST(Passthrough, GetRealPath)
{
    initialize_passthrough_if_necessary();

    // test relative mount path
    const char *oldpath = (const char *)"mount/file.txt";
    char passpath[PATH_MAX];
    pass_getpath(oldpath, passpath, 0);
    char rp[PATH_MAX];
    ((funcptr_realpath)libc_realpath)("source/file.txt", rp);
    ASSERT_STREQ(passpath, rp);

    // test masking
    const char *oldpath_1 = (const char *)"source/file.txt";
    pass_getpath(oldpath_1, passpath, 1);
    realpath("mount/file.txt", rp);
    ASSERT_STREQ(passpath, rp);

    // test non mount path without masking
    const char *oldpath_2 = (const char *)"/dev/shm/file.txt";
    pass_getpath(oldpath_2, passpath, 0);
    strcpy(rp, oldpath_2);
    ASSERT_STREQ(passpath, rp);

    // test non mount path with masking
    pass_getpath(oldpath_2, passpath, 1);
    strcpy(rp, oldpath_2);
    ASSERT_STREQ(passpath, rp);

    int match;
    // test source path without masking
    char *oldpath_3 = (char *)malloc(PATH_MAX * sizeof(char *));
    ((funcptr_realpath)libc_realpath)("source_1", oldpath_3);
    match = pass_getpath(oldpath_3, passpath, 0);
    strcpy(rp, oldpath_3);
    ASSERT_STREQ(passpath, rp);
    ASSERT_EQ(match, 0);

    // test non mount path with masking
    match = pass_getpath(oldpath_3, passpath, 1);
    strcpy(rp, oldpath_3);
    ASSERT_EQ(match, 1);
}

//TODO: Also check error values
TEST(Sea, OpenDir)
{

    char *sd = "/subdir";

    // Test if path outside of sea can be opened
    char *regdir = "newdir";

    DIR *rdp = opendir(regdir);
    EXPECT_TRUE(rdp != NULL);
    EXPECT_TRUE(((SEA_DIR *)rdp)->dirp != NULL);
    //printf("is null %d\n", ((SEA_DIR *)rdp)->dirp != NULL);
    //((funcptr_closedir)libc_closedir)((DIR *)((SEA_DIR *)rdp)->dirp);
    closedir(rdp);

    // Test if subdir in path outside of Sea can be opened
    char regsd[PATH_MAX];
    strcpy(regsd, regdir);
    strcat(regsd, sd);

    DIR *rsdp = opendir(regsd);
    EXPECT_TRUE(rsdp != NULL);
    closedir(rsdp);
    //((funcptr_closedir)libc_closedir)(((SEA_DIR *)rsdp)->dirp);

    // Test if Sea mount point can be opened
    char *seadir = "mount";
    DIR *sdp = opendir(seadir);
    EXPECT_TRUE(sdp != NULL);

    SEA_DIR *seadp = (SEA_DIR *)sdp;

    if (seadp->dirp == NULL)
        printf("%s is NULL\n", seadp->dirnames[0]);

    //close_seadir(seadp);
    closedir(sdp);

    // Test if subdir in sea can be opened
    char seasd[PATH_MAX];
    strcpy(seasd, seadir);
    strcat(seasd, sd);

    DIR *ssdirp = opendir(seasd);
    EXPECT_TRUE(sdp != NULL);

    SEA_DIR *seasdp = (SEA_DIR *)ssdirp;
    //close_seadir(seasdp);
    closedir(ssdirp);

    //free(seadp);
    //free(seasdp);

    // Check that a directory which does not exist fails
    char *fakedir = "mount/fkdir";
    DIR *fkdir = opendir(fakedir);

    EXPECT_TRUE(fkdir == NULL);
}

TEST(Sea, ReadDir)
{

    initialize_sea_if_necessary();
    // Test nonseadir
    char *regdir = "newdir";
    struct dirent *entry;
    std::set<std::string> reg_files = {".", "..", "subdir"};
    std::set<std::string> found_files;

    // 32bit
    DIR *rd = opendir(regdir);

    while ((entry = readdir(rd)))
    {
        found_files.insert(entry->d_name);
    }
    closedir(rd);
    //((funcptr_closedir)libc_closedir)(rd);

    ASSERT_EQ(reg_files, found_files);

    // 64bit
    rd = opendir(regdir);
    struct dirent64 *entry64;
    found_files.clear();

    while ((entry64 = readdir64(rd)))
    {
        found_files.insert(entry64->d_name);
    }
    closedir(rd);
    //((funcptr_closedir)libc_closedir)(rd);

    ASSERT_EQ(reg_files, found_files);

    // Test sea mount
    std::set<std::string> sea_files = {".", "..", "file_in_source.txt", "subdir", "complex_file.txt", "bin", "file_in_mem.txt"};
    found_files.clear();

    // 32bit
    char *seadir = "mount";
    DIR *sdp = opendir(seadir);

    SEA_DIR *sd = (SEA_DIR *)sdp;
    while ((entry = readdir(sdp)))
    {
        found_files.insert(entry->d_name);
    }
    closedir(sdp);
    //close_seadir((SEA_DIR *)sdp);
    ASSERT_EQ(sea_files, found_files);

    //64bit
    found_files.clear();
    sdp = opendir(seadir);

    while ((entry64 = readdir64(sdp)))
    {
        found_files.insert(entry64->d_name);
    }
    closedir(sdp);
    //close_seadir((SEA_DIR *)sdp);
    ASSERT_EQ(sea_files, found_files);

    // Test sea subdirectory
    seadir = "mount/subdir";
    std::set<std::string> subdir_files = {".", "..", "file_in_subdir.txt"};

    // 32bit
    sdp = opendir(seadir);
    found_files.clear();

    while ((entry = readdir(sdp)))
    {
        found_files.insert(entry->d_name);
    }
    closedir(sdp);
    //close_seadir((SEA_DIR *)sdp);
    ASSERT_EQ(subdir_files, found_files);

    //64bit
    found_files.clear();
    sdp = opendir(seadir);

    while ((entry64 = readdir64(sdp)))
    {
        found_files.insert(entry64->d_name);
    }
    closedir(sdp);
    //close_seadir((SEA_DIR *)sdp);
    ASSERT_EQ(subdir_files, found_files);
}

TEST(Sea, InitializeSea)
{

    initialize_sea();

    initialize_passthrough_if_necessary();
    config sea_config = get_sea_config();

    char *added_dir = new char[PATH_MAX];
    strcpy(added_dir, sea_config.source_mounts[1]);
    strcat(added_dir, "/newdir");

    initialize_sea();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    initialize_passthrough_if_necessary();
    ((funcptr_mkdir)libc_mkdir)("newdir", 0666);
    ((funcptr_mkdir)libc_mkdir)("newdir/subdir", 0666);

    return RUN_ALL_TESTS();
}

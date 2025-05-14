
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>


int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') {
        return -1;  // abs
    }

    int inumber = ROOT_INUMBER;

    char path_copy[1024];
    strncpy(path_copy, pathname, sizeof(path_copy));
    path_copy[sizeof(path_copy) - 1] = '\0';

    char *token = strtok(path_copy, "/");
    while (token != NULL) {
        struct direntv6 dirEnt;
        if (directory_findname(fs, token, inumber, &dirEnt) < 0) {
            return -1;  // not found
        }
        inumber = dirEnt.d_inumber;
        token = strtok(NULL, "/");
    }

    return inumber;
}

#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
    struct inode in;
    // error al cargar inode
    if (inode_iget(fs, dirinumber, &in) < 0) {
        return -1;
    }
    // verifico que el inode sea un directorio
    if (!(in.i_mode & IALLOC) || ((in.i_mode & IFMT) != IFDIR)) {
        return -1;
    }

    int size = inode_getsize(&in);
    int numBlocks = (size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;
    char buf[DISKIMG_SECTOR_SIZE];

    for (int bno = 0; bno < numBlocks; bno++) {
        int bytes = file_getblock(fs, dirinumber, bno, buf);
        // verifico si el sector leyo bien
        if (bytes < 0) {
            return -1;
        }

        int numEntries = bytes / sizeof(struct direntv6);
        struct direntv6 *entries = (struct direntv6 *)buf;

        for (int i = 0; i < numEntries; i++) {
            if (entries[i].d_inumber == 0) {
                continue; 
            }
            if (strncmp(entries[i].d_name, name, 14) == 0) {
                *dirEnt = entries[i];
                return 0;
            }
        }
    }

    return -1;
}

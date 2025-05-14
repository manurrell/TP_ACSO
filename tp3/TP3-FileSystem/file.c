#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"


int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;
    // error al cargar el inode
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }

    int diskBlockNum = inode_indexlookup(fs, &in, blockNum);
    // error al buscar el block
    if (diskBlockNum < 0) {
        return -1;
    }
    // error al leer el secotr
    if (diskimg_readsector(fs->dfd, diskBlockNum, buf) != DISKIMG_SECTOR_SIZE) {
        return -1;
    }

    int filesize = inode_getsize(&in);
    int offset = blockNum * DISKIMG_SECTOR_SIZE;
    // si el offset es mayor al tamaño del archivo, no hay nada que leer
    if (offset >= filesize) {
        return 0;
    }

    int remaining = filesize - offset;
    int validBytes = remaining < DISKIMG_SECTOR_SIZE ? remaining : DISKIMG_SECTOR_SIZE;

    return validBytes;
}


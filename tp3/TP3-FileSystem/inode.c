#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"



int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    int inodesPerBlock = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int blockNum = INODE_START_SECTOR + (inumber - 1) / inodesPerBlock;
    int offset = (inumber - 1) % inodesPerBlock;

    struct inode block[inodesPerBlock];
    int res = diskimg_readsector(fs->dfd, blockNum, block);
    // me fijo que no se pase el read
    if (res != DISKIMG_SECTOR_SIZE) {
        return -1;
    }
    // busco el inode en el bloque
    *inp = block[offset];
    return 0;
}


int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {  
        if (inp == NULL || blockNum < 0) {
            return -1;
        }
    
        // direct addressing
        if ((inp->i_mode & ILARG) == 0) {
            if (blockNum >= 8) return -1;
            int addr = inp->i_addr[blockNum];
            if (addr == 0) return -1;
            return addr;
        }
    
        // indirect addressing
        if (blockNum < 7 * 256) {
            int indirect_block = inp->i_addr[blockNum / 256];
            if (indirect_block == 0) return -1;
    
            uint16_t buf[DISKIMG_SECTOR_SIZE / sizeof(uint16_t)];
            if (diskimg_readsector(fs->dfd, indirect_block, buf) != DISKIMG_SECTOR_SIZE) {
                return -1;
            }
    
            int index = blockNum % 256;
            int addr = buf[index];
            if (addr == 0) return -1;
            return addr;
        }
    
        // double indirect
        blockNum -= 7 * 256;
        int double_indirect_block = inp->i_addr[7];
        if (double_indirect_block == 0) return -1;
    
        uint16_t first_level[DISKIMG_SECTOR_SIZE / sizeof(uint16_t)];
        if (diskimg_readsector(fs->dfd, double_indirect_block, first_level) != DISKIMG_SECTOR_SIZE) {
            return -1;
        }
    
        int outer_index = blockNum / 256;
        int inner_index = blockNum % 256;
        int indirect_block = first_level[outer_index];
        if (indirect_block == 0) return -1;
    
        uint16_t second_level[DISKIMG_SECTOR_SIZE / sizeof(uint16_t)];
        if (diskimg_readsector(fs->dfd, indirect_block, second_level) != DISKIMG_SECTOR_SIZE) {
            return -1;
        }
    
        int addr = second_level[inner_index];
        if (addr == 0) return -1;
    
        return addr;
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}

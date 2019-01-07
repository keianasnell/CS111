//
//  lab3a.c
//  p3A
//
//  Created by Keiana Snell on 11/18/18.
//  Copyright © 2018 Keiana Snell. All rights reserved.
//
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "ext2_fs.h"

const char *fs;
int fs_fd = -1;
uint32_t block_size;

//Superblock vars
struct ext2_super_block superblock;
uint32_t total_num_blocks;
uint32_t total_num_inodes;
uint32_t inode_size;
uint32_t blocks_per_group;
uint32_t inodes_per_group;
uint32_t first_non_reserved_inode;

//Group vars
struct ext2_group_desc group;
uint16_t num_free_blocks;
uint16_t num_free_inodes;
uint32_t block_bitmap;
uint32_t inode_bitmap;
uint32_t first_block_inodes;

//inode vars
struct ext2_inode inode;

void cleanup() {
    close(fs_fd);
}

void directory_entries(int p, int block_num){
    struct ext2_dir_entry dir_entry;
    ssize_t size = sizeof(struct ext2_dir_entry);
    unsigned int i;
    for(i = 0; i < block_size; i += dir_entry.rec_len){
        if(pread(fs_fd, &dir_entry, size, (block_size*block_num)+i) == -1){
            fprintf(stderr, "Error in reading for directory entries\n");
        }
        
        int inode = dir_entry.inode;
        if(inode){
            fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,\'%s\'\n", p, i, dir_entry.inode, dir_entry.rec_len, dir_entry.name_len, dir_entry.name);
        }
    }
}


void indirect_block_refs(int p, int level, int offset, int inode){
    uint32_t current_block;
    const int b = block_size*inode;
    unsigned int i;
    for(i = 0; i < total_num_blocks; i++){
        if(pread(fs_fd, &current_block, sizeof(uint32_t), (b+(i*sizeof(uint32_t)))) == -1){
            fprintf(stderr, "Error in reading for indirect block references\n");
        }
    
        if(current_block){
            int logical_block_off;
            if(level == 2) logical_block_off = offset + (i*256);
            else if(level == 3) logical_block_off = offset + (i*65536);
            else logical_block_off = offset + i;
        
        
        fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", p, level, logical_block_off, inode, current_block);
        if (level > 1) {
            indirect_block_refs(p, level-1, logical_block_off, current_block);
            }
        }
    }
        
}

void inode_summary(){
    unsigned int i;
    for (i = 0; i < total_num_inodes; i++) {
        if(pread(fs_fd, &inode, inode_size, first_block_inodes*block_size + inode_size*i) == -1){
            fprintf(stderr, "Error in reading for inode summary\n");
        }
    
    
    if (inode.i_mode == 0 || inode.i_links_count == 0)
        continue;
    
    char file_type = '?';
    if (inode.i_mode & 0x4000)  file_type = 'd';
    else if (inode.i_mode & 0x8000) file_type = 'f';
    else if (inode.i_mode & 0xA000) file_type = 's';
        
        char create_time[18];
        time_t time = inode.i_ctime;
        struct tm* ctime = gmtime(&time);
        strftime(create_time, 18, "%D %H:%M:%S", ctime);
        
        char modify_time[18];
        time = inode.i_mtime;
        struct tm* mtime = gmtime(&time);
        strftime(modify_time, 18, "%D %H:%M:%S", mtime);
        
        char access_time[18];
        time = inode.i_atime;
        struct tm* atime = gmtime(&time);
        strftime(access_time, 18, "%D %H:%M:%S", atime);
        
        fprintf(stdout, "INODE,%u,%c,%o,%u,%u,%u,%s,%s,%s,%d,%d", i+1, file_type, inode.i_mode & 0xFFF, inode.i_uid, inode.i_gid, inode.i_links_count, create_time, modify_time, access_time, inode.i_size, inode.i_blocks);
        
        int j;
        for (j = 0; j < EXT2_N_BLOCKS; j++) {
            fprintf(stdout, ",%u", inode.i_block[j]);
        }
        
        fprintf(stdout, "\n");
        
        if(file_type == 'd'){
            int j;
            for (j = 0; j < EXT2_N_BLOCKS; j++) {
                if(inode.i_block[j]){
                    directory_entries(i+1, inode.i_block[j]);
                }
            }
        }
    
        if(file_type == 'f' || file_type == 'd'){
            if(inode.i_block[EXT2_IND_BLOCK]) indirect_block_refs(i+1, 1, 12, inode.i_block[EXT2_IND_BLOCK]);
            if(inode.i_block[EXT2_DIND_BLOCK]) indirect_block_refs(i+1, 2, 268, inode.i_block[EXT2_DIND_BLOCK]);
            if(inode.i_block[EXT2_TIND_BLOCK]) indirect_block_refs(i+1, 3, 65804, inode.i_block[EXT2_TIND_BLOCK]);
        }
    }
}


void superblock_summary(){
    
    if (pread(fs_fd, &superblock, sizeof(struct ext2_super_block), 1024) == -1) {
        fprintf(stderr, "Error in reading for superblock summary\n");
    }
    total_num_blocks = superblock.s_blocks_count;
    total_num_inodes = superblock.s_inodes_count;
    block_size = 1024 << superblock.s_log_block_size;
    inode_size = superblock.s_inode_size;
    blocks_per_group = superblock.s_blocks_per_group;
    inodes_per_group = superblock.s_inodes_per_group;
    first_non_reserved_inode = superblock.s_first_ino;
    
    fprintf(stdout, "SUPERBLOCK,%u,%u,%u,%u,%u,%u,%u\n", total_num_blocks, total_num_inodes, block_size, inode_size, blocks_per_group, inodes_per_group, first_non_reserved_inode);
}


void scan_free_inodes() {
    uint8_t byte = 0;
    unsigned int i, j;
    for (i = 0; i < total_num_inodes; i++) {
        if (pread(fs_fd, &byte, 1, inode_bitmap*block_size+i) == -1) {
            fprintf(stderr, "Error in reading for free inode entries\n");
        }
        for (j = 0; j < 8; j++) {
            if (!(byte & (1 << j))) {
                fprintf(stdout, "IFREE,%u\n", i*8+j+1);
            }
        }
    }
    
}

void scan_free_blocks() {
    uint8_t byte = 0;
    unsigned int i, j;
    for (i = 0; i < total_num_blocks; i++) {
        if (pread(fs_fd, &byte, 1, block_bitmap*block_size+i) == -1) {
            fprintf(stderr, "Error in reading for free block entries\n");
        }
        for (j = 0; j < 8; j++) {
            if (!(byte & (1 << j))) {
                fprintf(stdout, "BFREE,%u\n", i*8+j+1);
            }
        }
    }
}


void group_summary(){
    if (pread(fs_fd, &group, sizeof(struct ext2_group_desc), 1024+sizeof(struct ext2_super_block)) == -1) {
        fprintf(stderr, "Error in reading for group summary\n");
    }
    
    num_free_blocks = group.bg_free_blocks_count;
    num_free_inodes = group.bg_free_inodes_count;
    block_bitmap = group.bg_block_bitmap;
    inode_bitmap = group.bg_inode_bitmap;
    first_block_inodes = group.bg_inode_table;

    fprintf(stdout, "GROUP,0,%u,%u,%u,%u,%u,%u,%u\n", total_num_blocks, total_num_inodes, num_free_blocks, num_free_inodes, block_bitmap, inode_bitmap, first_block_inodes);
    
    scan_free_blocks();
    scan_free_inodes();
    inode_summary();

}

int main(int argc, char* argv[]){
    atexit(cleanup);
    
    if (argc != 2) {
        fprintf(stderr, "Error, incorrect number of arguments. Usage: ./lab3a __file_sys_img__ \n");
        exit(1);
    }
    
    fs = argv[1];
    if ((fs_fd = open(fs, O_RDONLY)) == -1) {
        fprintf(stderr, "Error in opening specified filesystem img %s\n", strerror(errno));
        exit(1);
    }
    
    superblock_summary();
    group_summary();

    exit(0);
}

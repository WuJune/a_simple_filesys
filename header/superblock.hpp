/*
 * @Author: your name
 * @Date: 2020-12-07 18:52:20
 * @LastEditTime: 2020-12-08 20:06:16
 * @LastEditors: Please set LastEditors
 * @Description: VFS super blokc class for VFS
 * @FilePath: /code/SuperBlokc.hpp
 */

#ifndef __SUPERBLOCK_HPP_
#define __SUPERBLOCK_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#define FILE_SYSTEM_MAX_BYTE 0x00900c00

class Superblock
{
protected:
    uint32_t s_inodes_count = 0;        //文件系统所有的 Inode 数量
    uint32_t s_blocks_count = 0;        //文件系统所有的 block 数量
    uint32_t s_free_blocks_count = 0;   //文件系统空余可用 block 数量
    uint32_t s_free_inodes_count = 0;   //文件系统剩余 inode 数量
    uint32_t s_first_data_block = 0;    //superblock 的 block id
    uint32_t s_log_block_size = 0;      //block size = 1024 << s_log_block_size;
    uint32_t s_blocks_per_group = 0;    //每个 group 的 block 数量
    uint32_t s_inodes_per_group = 0;    //每个 group 的 inode 数量
    uint16_t s_inode_size = 128;        //inode 的大小 In revision 0, this value is always 128

    uint64_t VFS_block_size = 0;        //文件系统中, 每个 block 的大小

public:
    int initialize(uint64_t file_system_full_size = FILE_SYSTEM_MAX_BYTE, uint32_t log_block_size = 0)
    {
        s_log_block_size = log_block_size;

        s_first_data_block = log_block_size == 0 ? 1 : 0;

        VFS_block_size = 1024 << log_block_size;

        s_inodes_per_group = 1024 * 8; //每组共 1024 * 8 个 inode 写死
        s_blocks_per_group = 1024 * 8; //每组共 1024 * 8 个 block 写死

        file_system_full_size -= 2 * VFS_block_size;
        // s_blocks_count = file_system_full_size / VFS_bytes_per_group * s_blocks_per_group;
        s_blocks_count = 1024 * 8; // 总共 1024 * 8 个 block 写死
        s_inodes_count = 1024 * 8; // 总共 1024 * 8 个 block 写死

        s_free_blocks_count = s_blocks_count;
        s_free_inodes_count = s_inodes_count;

        return 0;
    }


    // 将 SuperBlock 信息写入模拟磁盘的二进制文件中
    int write_to_disk(std::string disk_file_path)
    {
        std::fstream disk_file;
        disk_file.open(disk_file_path, std::ios::binary | std::ios::out | std::ios::in);
        if(!disk_file.good())
        {
            std::cout << "disk_file is not exist" << std::endl;
        }
        
        disk_file.seekp(0);
        disk_file.write((char*)&s_inodes_count, 4);
        disk_file.write((char*)&s_blocks_count, 4);
        disk_file.write((char*)&s_free_blocks_count, 4);
        disk_file.write((char*)&s_free_inodes_count, 4);
        disk_file.write((char*)&s_first_data_block, 4);
        disk_file.write((char*)&s_log_block_size, 4);
        disk_file.write((char*)&s_blocks_per_group, 4);
        disk_file.write((char*)&s_inodes_per_group, 4);
        disk_file.write((char*)&s_inode_size, 2);

        disk_file.seekp(9440256);
        disk_file << "DISK_END";
        disk_file.close();
        return 0;
    }


    // 从模拟磁盘的二进制文件中恢复 SuperBlock 信息
    int read_to_VFS(std::string disk_file_path)
    {
        std::fstream disk_file;
        disk_file.open(disk_file_path, std::ios::binary | std::ios::in);
        if(!disk_file.good())
        {
            std::cout << "disk_file is not exist" << std::endl;
        }
        
        disk_file.seekp(0);
        disk_file.read((char*)&s_inodes_count, 4);
        disk_file.read((char*)&s_blocks_count, 4);
        disk_file.read((char*)&s_free_blocks_count, 4);
        disk_file.read((char*)&s_free_inodes_count, 4);
        disk_file.read((char*)&s_first_data_block, 4);
        disk_file.read((char*)&s_log_block_size, 4);
        disk_file.read((char*)&s_blocks_per_group, 4);
        disk_file.read((char*)&s_inodes_per_group, 4);
        disk_file.read((char*)&s_inode_size, 2);

        disk_file.close();
        return 0;
    }


    int print()
    {
        std::cout << "--- superblock ---" << std::endl;
        std::cout << "s_inodes_count: " << s_inodes_count << std::endl;
        std::cout << "s_blocks_count: " << s_blocks_count << std::endl;
        std::cout << "s_free_blocks_count: " << s_free_blocks_count  << std::endl;
        std::cout << "s_free_inodes_count: " << s_free_inodes_count << std::endl;
        std::cout << "s_first_data_block: " << s_first_data_block << std::endl;
        std::cout << "s_log_block_size: " << s_log_block_size << std::endl;
        std::cout << "s_blocks_per_group: " << s_blocks_per_group << std::endl;
        std::cout << "s_inodes_per_group: " << s_inodes_per_group << std::endl;
        std::cout << "s_inode_size: " << s_inode_size << std::endl;
        
        return 0;
    }
  

    uint32_t get_free_inode_count()
    {
        return s_free_inodes_count;
    }

    int set_free_inode_count(uint32_t n)
    {
        this->s_free_inodes_count = n;
        return 0;
    }

    uint32_t get_free_blocks_count()
    {
        return s_free_blocks_count;
    }

    int set_free_blocks_count(uint32_t n)
    {
        this->s_free_blocks_count = n;
        return 0;
    }
};

#endif
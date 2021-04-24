/*
 * @Author: your name
 * @Date: 2020-12-07 18:45:12
 * @LastEditTime: 2020-12-08 23:12:57
 * @LastEditors: Please set LastEditors
 * @Description: file block class header for VFS
 * @FilePath: /code/file_block.hpp
 */

#ifndef __FILE_BLOCK_HPP_
#define __FILE_BLOCK_HPP_

#include <iostream>
#include <string>
#include <fstream>

class VFS_file_block
{
protected:  
    char VFS_block[1024] = {};
    uint32_t VFS_offset_beg = 0;
    uint32_t VFS_block_id = 0;
    uint32_t VFS_bg_id = 0;

public:
    VFS_file_block(uint32_t block_id)
    {
        VFS_block_id = block_id;
        VFS_bg_id = 0;
        VFS_offset_beg = 1051648 + block_id * 1024;
    }

    int print()
    {
        std::cout << "VFS_bg_id: " << VFS_bg_id <<std::endl;
        std::cout << "VFS_block_id: " << VFS_block_id <<std::endl;
        std::cout << "VFS_offset_beg: " << VFS_offset_beg <<std::endl;
        std::cout << "context: " << std::endl;
        std::cout << VFS_block << std::endl;
        return 0;
    }


    int write_to_disk(std::string disk_file_path)
    {
        std::fstream disk_file;
        disk_file.open(disk_file_path, std::ios::binary | std::ios::out | std::ios::in);
        if(!disk_file.good())
        {
            std::cout << "disk_file is not exist" << std::endl;
        }
        
        disk_file.seekp(VFS_offset_beg);
        disk_file.write(VFS_block, 1024);
        disk_file.close();
        return 0;
    }


    int read_to_VFS(std::string disk_file_path)
    {
        std::fstream disk_file;
        disk_file.open(disk_file_path, std::ios::binary | std::ios::out | std::ios::in);
        if(!disk_file.good())
        {
            std::cout << "disk_file is not exist" << std::endl;
        }
        
        disk_file.seekp(VFS_offset_beg);
        disk_file.read(VFS_block, 1024);
        disk_file.close();
        return 0;
    }

    char* get_block_pointer()
    {
        return this->VFS_block;
    }
    
    uint32_t get_VFS_block_id()
    {
        return this->VFS_block_id;
    }

    uint32_t get_VFS_offset_beg()
    {
        return this->VFS_offset_beg;
    }
    
    int reset()
    {
        std::string tempstr(1024, 0);
        tempstr.copy(VFS_block, 1024);
        return 0;
    }

    int test()
    {
        VFS_block[0] = '@';
        VFS_block[1] = 'X';
        for(int i = 2; i < 1023; ++i)
        {
            VFS_block[i] = '1';
        }
        VFS_block[1023] = 'E';
        return 0;
    }
};

#endif
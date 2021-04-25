/*
 * @Author: your name
 * @Date: 2020-12-08 13:02:12
 * @LastEditTime: 2020-12-08 19:16:47
 * @LastEditors: Please set LastEditors
 * @Description: Block Bitmap class for VFS
 * @FilePath: /code/block_bitmap.hpp
 */

#ifndef _BLOCK_BITMAP_HPP_
#define _BLOCK_BITMAP_HPP_

#include <iostream>
#include <fstream>

class Block_bitmap
{
protected:
    char bitmap_byte[1024] = {};    // 1024 * 8 bit

    uint32_t VFS_bg_id = 0;
    uint32_t VFS_offset_beg = 0;
    uint8_t bitmap[1024 * 8] = {};  // 1024 * 8 ¸ö uint8

public:
    Block_bitmap()
    {
        VFS_bg_id = 0;
        VFS_offset_beg = 1024; // Block 1, Ð´ËÀ
    }


    int write_to_disk(std::string disk_file_path)
    {
        for(int i = 0; i < 1024; ++i)
        {
            bitmap_byte[i] = 0;
            for(int j = 0; j < 8; ++j)
            {
                if(bitmap[i*8 + j])
                {
                    bitmap_byte[i] += 0x80 >> j;

                }
            }
        }

        std::fstream disk_file;
        disk_file.open(disk_file_path, std::ios::binary | std::ios::out | std::ios::in);
        if(!disk_file.good())
        {
            std::cout << "disk_file is not exist" << std::endl;
        }
        
        disk_file.seekp(VFS_offset_beg);
        for(uint32_t i = 0; i < 1024; ++i)
        {
            disk_file.write(bitmap_byte+i, 1);
        }

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
        for(uint32_t i = 0; i < 1024; ++i)
        {
            disk_file.read(bitmap_byte+i, 1);
        }

        for(int i = 0; i < 1024; ++i)
        {
            for(int j = 0; j < 8; ++j)
            {
                if((bitmap_byte[i] << j) & 0x80)
                {
                    bitmap[i*8 + j] = 1;
                }
            }
        }

        disk_file.close();
        return 0;
    }

    int print()
    {
        std::cout << "--- block bitmap ---" << std::endl;
        std::cout << "VFS_offset_beg" << VFS_offset_beg << std::endl;
        return 0;
    }

    uint32_t get_next_free_inode()
    {
        for(uint32_t i = 0; i < 1024 * 8; ++i)
        {
            if(bitmap[i] == 0)
            {
                return i;
            }
        }
        return 0;
    }

    int set_block(uint32_t i)
    {
        bitmap[i] = 1;
        return 0;
    }
    
    int reset_block(uint32_t i)
    {
        bitmap[i] = 0;
        return 0;
    }

    int test()
    {
        bitmap_byte[0] = 1;
        bitmap[8] = 1;
        bitmap[1024*8-2] = 1;
        bitmap[1024*8-1] = 1;
        return 0;
    }
};

#endif
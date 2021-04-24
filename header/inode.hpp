/*
 * @Author: your name
 * @Date: 2020-12-08 19:20:55
 * @LastEditTime: 2020-12-08 22:43:59
 * @LastEditors: Please set LastEditors
 * @Description: Inode class for VFS
 * @FilePath: /code/inode.hpp
 */

#ifndef _INODE_HPP_
#define _INODE_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "./file_block.hpp"
#include "./superblock.hpp"

class Inode
{
protected:
    uint16_t    i_mode = 0;        // 文件类型和 RWX 等权限描述
    uint16_t    i_uid = 0;         // 用户目录 id
    uint32_t    i_atime = 0;       // inode 上一次被访问时间 unix_time
    uint32_t    i_ctime = 0;       // inode 创建时间
    uint32_t    i_mtime = 0;       // inode 上一次 modified 时间
    uint32_t    i_dtime = 0;       // inode 上一次 删除时间
    uint32_t    indirect_block = 0;// 一级索引
    std::string i_name;            // inode 对应文件的名字
    
    uint32_t VFS_offset_beg = 0;    // Inode 的起始地址
    uint32_t VFS_inode_id = 0;      // Inode 的 ID
    uint32_t VFS_ingroup_id = 0;    // Inode 在 Inode Table 里面的对应位置
    std::vector<VFS_file_block> block_tabel;    // Inode 文件 所有对应 Block 的 Table


public:
    Inode(uint16_t inode_id, std::string str_name = "", uint16_t uid = 0)
    {
        i_uid = uid;    // 默认为用户 0
        i_name = str_name;
        VFS_inode_id = inode_id;
        VFS_ingroup_id = inode_id;
        VFS_offset_beg = 3072 + (inode_id) *  128;
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
        disk_file.write((char*)&i_mode, 2);
        disk_file.write((char*)&i_uid, 2);
        disk_file.write((char*)&i_atime, 4);
        disk_file.write((char*)&i_ctime, 4);
        disk_file.write((char*)&i_mtime, 4);
        disk_file.write((char*)&i_dtime, 4);
        disk_file.write((char*)&indirect_block, 4);
        char temp_c_str[103] = {};
        disk_file.write(temp_c_str, 103);
        disk_file.seekp(-103, std::ios::cur);
        disk_file.write(i_name.c_str(), i_name.size() < 98 ? i_name.size() : 98);
        disk_file.close();

        for(int i = 0; i < block_tabel.size(); ++i)
        {
            block_tabel[i].write_to_disk(disk_file_path);
        }

        VFS_file_block temp_block(indirect_block);
        char* temp_block_p = temp_block.get_block_pointer();
        for(uint32_t i = 0; i < block_tabel.size(); ++i)
        {
            *(uint32_t*)temp_block_p = (uint32_t)block_tabel[i].get_VFS_block_id();
            temp_block_p += 4;
        }
        temp_block.write_to_disk(disk_file_path);

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
        disk_file.read((char*)&i_mode, 2);
        disk_file.read((char*)&i_uid, 2);
        disk_file.read((char*)&i_atime, 4);
        disk_file.read((char*)&i_ctime, 4);
        disk_file.read((char*)&i_mtime, 4);
        disk_file.read((char*)&i_dtime, 4);
        disk_file.read((char*)&indirect_block, 4);

        char temp_c_str[103] = {};
        disk_file.read(temp_c_str, 103);
        i_name = std::string(temp_c_str);

        disk_file.close();

        return 0;
    }



    int load_blocks(std::string path)  // 将所有 Block 读到 Block table 中
    {
        VFS_file_block temp_block(indirect_block);
        uint32_t temp_block_id = 0;
        temp_block.read_to_VFS(path);
        char* temp_block_p = temp_block.get_block_pointer();
        while((uint32_t)*temp_block_p != 0)
        {
            block_tabel.push_back(VFS_file_block((uint32_t)*temp_block_p));
            block_tabel.back().read_to_VFS(path);
            temp_block_p += 4;
        }
        return 0;
    }



    char* trans_pointer(uint32_t file_offset)   // 将文件偏移量转换为对应 Block 内的指针
    {
        if(file_offset > block_tabel.size() * 1024)
        {
            std::cout << "FILE OFFSET OUT OF INDEX!" << std::endl;
        }
        return block_tabel[file_offset / 1024].get_block_pointer() + file_offset % 1024;
    }



    int read(char* output, uint32_t file_offset ,uint32_t size) // 从文件的 Block 中读取内容
    {
        for(uint32_t i = 0; i < size; ++i)
        {
            char temp_byte = *trans_pointer(file_offset + i);
            *(output+i) = temp_byte;
        }
        return 0;
    }



    int write(char* input, uint32_t file_offset ,uint32_t size) // 从文件的 Block 中读取内容
    {
        for(uint32_t i = 0; i < size; ++i)
        {
            *trans_pointer(file_offset + i) = *(input+i);
        }
        return 0;
    }


    // 将对应 block 重置，并添加到 block table 中
    int add_block(uint32_t block_id)
    {
        block_tabel.push_back(VFS_file_block(block_id));
        return 0;
    }



    int truncate()
    {
        block_tabel = std::vector<VFS_file_block>();

        VFS_file_block temp_block(indirect_block);
        char* temp_block_p = temp_block.get_block_pointer();
        for(uint32_t i = 0; i < 1024; ++i)
        {
            *(temp_block_p+i) = 0;
        }
        return 0;
    }



    bool is_null()
    {
        return i_name == "";
    }



    uint32_t get_VFS_inode_id()
    {
        return this->VFS_inode_id;
    }



    uint32_t get_indirect_block()
    {
        return this->indirect_block;
    }
    


    uint32_t get_used_blocks_count()
    {
        return block_tabel.size();
    }



    int set_indirect_block(uint32_t block_id, std::string disk_file_path)
    {
        this->indirect_block = block_id;
        VFS_file_block temp_block(block_id);
        temp_block.reset();
        temp_block.write_to_disk(disk_file_path);
        return 0;
    }



    std::string get_i_name()
    {
        return this->i_name;
    }



    int set_i_name(std::string name_str)
    {
        this->i_name = name_str;
        return 0;
    }



    uint16_t get_uid()
    {
        return this->i_uid;
    }



    int set_uid(uint16_t uid)
    {
        this->i_uid = uid;
        return 0;
    }



    int print()
    {
        std::cout << "i_name: " << i_name.c_str() << std::endl;
        std::cout << "VFS_inode_id: " << VFS_inode_id << std::endl;
        std::cout << "VFS_blocks_count: " << block_tabel.size() << std::endl;
        std::cout << "VFS_ingroup_id: " << VFS_ingroup_id  << std::endl;
        std::cout << "VFS_offset_beg: " << VFS_offset_beg << std::endl;
        std::cout << "indirect_block :" << indirect_block << std::endl; 
        return 0;
    }


    // 当需要删除文件时，获取一串即将被释放的Block Number
    std::vector<uint32_t> get_delete_blocks_ids()
    {
        std::vector<uint32_t> temp_vec;
        for(uint32_t i = 0; i < block_tabel.size(); ++i)
        {
            temp_vec.push_back(block_tabel[i].get_VFS_block_id());
        }
        temp_vec.push_back(indirect_block);
        return temp_vec;
    }



    // 当需要截断文件时，获取一串即将被释放的Block Number
    std::vector<uint32_t> get_truncate_block_ids()
    {
        std::vector<uint32_t> temp_vec;
        for(uint32_t i = 0; i < block_tabel.size(); ++i)
        {
            temp_vec.push_back(block_tabel[i].get_VFS_block_id());
        }
        return temp_vec;
    }



    int operator=(Inode temp)
    {
        this->i_mode = temp.i_mode;        // 文件类型和 RWE 权限描述
        this->i_uid = temp.i_uid;         // 用户 id
        this->i_atime = temp.i_atime;       // inode 上一次被访问时间 unix_time
        this->i_ctime = temp.i_ctime;       // inode 创建时间
        this->i_mtime = temp.i_mtime;       // inode 上一次 modified 时间
        this->i_dtime = temp.i_dtime;       // inode 上一次 删除时间
        this->indirect_block = temp.indirect_block;// 写死，128
        this->i_name = temp.i_name;
        
        this->VFS_offset_beg = temp.VFS_offset_beg;    // Inode 的起始地址
        this->VFS_inode_id = temp.VFS_inode_id;      // Inode 的 ID
        this->VFS_ingroup_id = temp.VFS_ingroup_id;    // Inode 在 Inode Table 里面的对应位置

        this->block_tabel = temp.block_tabel;    // Inode 文件 所有对应 Block 的 Table 
        
        return 0;
    }



    int ls_print()
    {
        std::cout << "--- file name: " << i_name << std::endl;
        std::cout << "uid: " << i_uid << std::endl;
        std::cout << "VFS_inode_id: " << VFS_inode_id << std::endl;
        std::cout << "indirect_block " << indirect_block << std::endl;
        std::cout << "count of blocks: " << block_tabel.size() << std::endl;
        std::cout << "used blocks: ";
        for(int i = 0; i < block_tabel.size(); ++i)
        {
            std::cout << block_tabel[i].get_VFS_block_id() << " ";
        }
        std::cout << std::endl;
        return 0;
    }



    int ls_i_print()
    {
        std::cout << "--- file name: " << i_name << std::endl;
        std::cout << "VFS_inode_id: " << VFS_inode_id << std::endl;
        std::cout << "count of blocks: " << block_tabel.size() << std::endl;
        return 0;
    }
};

#endif
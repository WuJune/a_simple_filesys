# a_simple_filesystem

这是一个通过二进制文件模拟简单索引式文件系统的实验。

通过二进制文件模拟磁盘；通过对二进制文件的读写，模拟对磁盘的读写。

## 文件系统简述

对于一个文件系统，我们分为内存内和磁盘上两部分。

对于内存内，我们需要维护一个虚拟文件系统（VFS），对于磁盘上，我们需要维护一个实际的、完整的文件在磁盘上的内容布局。

通过VFS，提供高层次的对文件的各种操作：打开、删除、链接、重命名等。通过文件系统，将 VFS 所表达的各种文件操作通过高效合理的方式组织保存在磁盘上。

## 实验中模拟的文件系统（磁盘上）

本次模拟的文件系统很大程度上参考了曾经 Linux 发行版的默认文件系统 `Ext2`。（关于Ext2详细可以参考 [The Second Extended File System](https://www.nongnu.org/ext2-doc/ext2.html)、[鸟哥的 Linux 私房菜 -- Linux 磁盘与文件系统管理](http://cn.linux.vbird.org/linux_basic/0230filesystem_1.php)）

### 简述

总的来说，本次实验模拟实现了一个包括文件创建、截断、扩大、删除、重命名的二级文件系统，该系统是一个一级索引的索引式文件系统。

相比 `Ext2`，做出如下简化：

* 将多级文件系统改为二级文件系统，系统逻辑结构示例如下图所示

  ```
  ├── user_00
  │   ├── file_00.txt
  │   ├── file_01.txt
  │   └── file_test.txt
  ├── user_01
  │   └── file_97.txt
  ├── user_02
  │   └── file_01.txt
  ├── user_03
  └── user_04
      ├── file_97.txt
      └── file_99.txt
  ```

* 取消目录项，将原先由目录项承担的文件名功能合并到 `Inode` 中

* 将多个 Block Group 改为固定一个 Block Group

* 固定 Inode、Block 的数量

### 磁盘上整体布局

| **Offset Beg   (bytes)** | **Offset End (bytes)** | **Description**                 |
| ------------------------ | ---------------------- | ------------------------------- |
| 0                        | 1023                   | SuperBlock                      |
| 1024                     | 2047                   | Block BItmap（1024 * 8 Bit）    |
| 2048                     | 3071                   | Inode BItmap（1024 *  8 Bit）   |
| 3072                     | 1051647                | Inode Table （1024 * 8  Inode） |
| 1051648                  | 9440255                | Block Table （1024 *  8 Block） |

### 各个组成部分

该文件系统由上表5个部分维护。

* SuperBlock 记录文件系统整体资源情况
* Block BitMa 记录 Block 使用情况
* Inode BitMap记录 Inode使用情况
* Inode Table 由连续的 Inode 组成，每个 Inode 代表一个文件
* Block Table 由连续的 Block 组成，一个或多个离散 Block 记录一个文件的内容。

#### SuperBlock

| **Offset (bytes)** | **Size (bytes)** | **Description**                                              |
| ------------------ | ---------------- | ------------------------------------------------------------ |
| 0                  | 4                | [s_inodes_count](http://www.nongnu.org/ext2-doc/ext2.html#s-inodes-count) 文件系统所有的 Inode 数量 = 1024 *  8 |
| 4                  | 4                | [s_blocks_count](http://www.nongnu.org/ext2-doc/ext2.html#s-blocks-count) 文件系统所有的 block 数量 = 1024 *  8 |
| 8                  | 4                | [s_free_blocks_count](http://www.nongnu.org/ext2-doc/ext2.html#s-free-blocks-count) 文件系统空余可用 block 数量 |
| 12                 | 4                | [s_free_inodes_count](http://www.nongnu.org/ext2-doc/ext2.html#s-free-inodes-count) 文件系统剩余 inode 数量 |
| 16                 | 4                | [s_log_block_size](http://www.nongnu.org/ext2-doc/ext2.html#s-log-block-size) block size = 1024 <<  s_log_block_size; = 0 |
| 20                 | 4                | [s_blocks_per_group](http://www.nongnu.org/ext2-doc/ext2.html#s-blocks-per-group) 每个 group 的 block 数量 = 1024 *  8 |
| 24                 | 4                | [s_inodes_per_group](http://www.nongnu.org/ext2-doc/ext2.html#s-inodes-per-group) 每个 group 的 inode 数量 = 1024 *  8 |
| 28                 | 2                | [s_inode_size](http://www.nongnu.org/ext2-doc/ext2.html#s-inode-size) inode 的大小 in ext2 ver0 this value  is always 128 |
| 30                 | 993              | 预留空间，可以后续完善用户管理系统或拓展其他功能用           |

#### Block BitMap

记录文件系统中各个Block是否被分配使用，按Bit记录和操作。Bit为1时为被使用，为0时表示未被使用。在本文件系统中，有1024 * 8个Block，所以对应BitMap有1024 * 8 Bit，即1024Byte（一个Block）。

#### Inode BitMap

同Block BitMap，只是对应管理的对象为Inode。

#### Inode Table

一个由连续 Inode 组成的区域。

#### Inode

| **Offset (bytes)** | **Size (bytes)** | **Description**                                              |
| ------------------ | ---------------- | ------------------------------------------------------------ |
| 0                  | 2                | [i_mode](http://www.nongnu.org/ext2-doc/ext2.html#i-mode) 文件类型和 RWX 权限描述 (预留接口) |
| 2                  | 2                | [i_uid](http://www.nongnu.org/ext2-doc/ext2.html#i-uid) 用户 id 用于记录所在用户目录 默认为 uid 0，默认已创建 0x00ff - 0xffff 个用户 |
| 4                  | 4                | [i_atime](http://www.nongnu.org/ext2-doc/ext2.html#i-atime) inode 上一次被访问时间 unix_time （预留接口） |
| 8                  | 4                | [i_ctime](http://www.nongnu.org/ext2-doc/ext2.html#i-ctime) inode 创建时间 unix_time （预留接口） |
| 12                 | 4                | [i_mtime](http://www.nongnu.org/ext2-doc/ext2.html#i-mtime) inode 上一次 modified 时间（预留接口） |
| 16                 | 4                | [i_dtime](http://www.nongnu.org/ext2-doc/ext2.html#i-dtime) inode 上一次 删除时间（预留接口） |
| 20                 | 4                | indirect_block 一级索引所在的Block的Block Number             |
| 24                 | 1-103            | i_name 文件名字，最长为 103 Byte，文件名字为空时判定为空Inode |

#### Block Table

一个由连续 Block 组成的区域

#### Block

一个可以自由读写的区域，大小为 1024Byte

#### indirect_block

indirect_block 中存放的不是文件内容，而是文件用来存放内容的 block 的索引，因此每个文件至少有一个或多个block。

例如：

 <img src=".\for_doc\image\index_block_00.png" alt="index_block_00" width="500px;" />

文件 `file_01.txt` 的索引所在 `Block` 为 `Block 1`，用来存放文件内容的 `Blokc` 有 2、3、4、5、6、8、9、0xA

`Block 1` 内容如下

 <img src=".\for_doc\image\index_block_01.png" alt="index_block_01"  width="500px;" />

------

### 已实现的功能（命令行形式）

#### 1. 格式化

每次启动程序时询问是否进行格式化

 <img src=".\for_doc\image\格式化.png" alt="格式化"  width="500px;" />

#### 2. exit

`exit`

退出命令交互界面，并结束程序

#### 3. create

`create file_name block_count`

 <img src=".\for_doc\image\create.png" alt="create" width="500px;" />

在当前用户目录下创建指定大小的文件

#### 4. find

`find file_name`

`find test1.txt`

 <img src=".\for_doc\image\find.png" alt="find" width="500px;" />

在当前用户目录下寻找目标文件并打印文件信息

#### 5. remove

`remove file_name`

`remove test1.txt`

 <img src=".\for_doc\image\remove.png" alt="create" width="500px;" />

在当前用户目录下删除目标文件

#### 6. truncate

`truncate file_name`

`truncate test1.txt`

 <img src=".\for_doc\image\truncate.png" alt="truncate" width="500px;" />

截断当前目录下的目标文件，释放目标文件的block（但不释放用来存放索引的 Block）

#### 7. increase

`increase file_name inc_cout`

 <img src=".\for_doc\image\increase.png" alt="increase" width="500px;" />

为当前目录下指定文件分配添加block

#### 8. write

`write file_name string`

 <img src=".\for_doc\image\write.png" alt="write" width="500px;" />

将目标字符串写入当前目录下目标文件

在命令行下默认从 offset 0 开始，字符串最长为 200

#### 9. read

`read file_name size`

 <img src=".\for_doc\image\read.png" alt="read" width="500px;" />

读取当前目录下目标文件从 offset 0 开始的 0-size 个字符并输出

> 命令行交互不提供随机读写
>
> 随机读写只提供 c++ 代码接口

#### 10. rename

`rename file_name file_new_name`

`rename test1.txt test1_new.txt`

 <img src=".\for_doc\image\rename.png" alt="rename" width="500px;" />

重命名当前目录下目标文件

#### 11. ls_i

`ls_i`

 <img src=".\for_doc\image\ls_i.png" alt="ls_i" width="500px;" />

打印当前用户目录下所有文件

#### 12. ls

`ls file_name`

`ls test1.txt`

 <img src=".\for_doc\image\ls.png" alt="ls" width="500px;" />

打印当前目录下目标文件信息

#### 13. cd

`cd uid`

`cd 1`

切换到目标 uid 目录

#### 14. id

`id`

获取当前访问目录对应的用户 uid

#### 15. login

`login uid`

`login 1`

登录到 uid，但不切换当前访问目录（登录 uid 和 当前目录 uid 区分开）

----

### TO DO

⬜️ 更友好的交互提醒

⬜️ 更顺畅的命令逻辑

⬜️ 更清晰的代码结构

### BUG TO FIX

⬜️ 查找不存在的文件后返回查找结果但程序直接结束，需修改为程序继续运行

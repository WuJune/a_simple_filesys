/*
 * @Author: your name
 * @Date: 2020-12-07 19:20:33
 * @LastEditTime: 2020-12-08 22:44:08
 * @LastEditors: Please set LastEditors
 * @Description: do some dev test
 * @FilePath: /code/test.cpp
 */
#include "./header/VFS.hpp"
int main()
{
    VFS vfs;
    char flag = 0;
    while(flag != 'Y' && flag != 'N')
    {
        std::cout << "是否执行格式化 Y/N" << std::endl;
        std::cin >> flag;
    }
    if(flag == 'Y')
    {
        vfs.format();
    }


    vfs.load_from_file();


    flag = '0';
    while(flag != 'Y' && flag != 'N')
    {
        std::cout << "是否执行测试样例 Y/N" << std::endl;
        std::cin >> flag;
    }
    if(flag == 'Y')
    {
        vfs.test();
    }


    vfs.wait_op();

    return 0;
}
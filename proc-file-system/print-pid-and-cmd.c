/*
 * 编写一个程序，以用户名作为命令行参数，列表显示该用户下所有正在运行的进程
 * ID 和命令名。（程序清单 8-1 中的 userIdFromName()函数对本题程序的编写可
 * 能会有所帮助。）通过分析系统中/proc/PID/status 文件的 Name：和 Uid：各行
 * 信息，可以实现此功能。遍历系统的所有/proc/PID 目录需要使用 readdir(3)函数，
 * 18.8 节对其进行了描述。程序必须能够正确处理如下可能性：在确定目录存在与
 * 程序尝试打开相应/proc/PID/status 文件之间，/proc/PID 目录消失了。
 */

/*
 * 任务整理
 * 1.显示用户所属的进程ID和命令字符串
 * 2.userIdFromName()函数会有帮助
 * 3.实现：通过分析系统中/proc/PID/status 文件的 Name：和 Uid
 * 4.遍历系统的所有/proc/PID 目录需要使用 readdir(3)函数
 * 5.问题：确定目录存在，但程序尝试打开相应/proc/PID/status 文件之间，/proc/PID 目录消失了
 */

/*
 * 程序思路
 * 1.通过userIdFromName查询UID
 * 2.opendir打开目录，readdir()遍历目录（当readdir返回NULL且errno不变时，表示到达了目录尾，而非目录消失了。注意在调用之前需要设置errno。）
 * 3.根据遍历得到的目录名，open每个PID的status文件
 * 4.读取UID字段的值，若真实ID匹配则输出进程ID，并读取/pid/cmdline的内容后输出
 */

/*
 * 程序设计
 * 1.userIdFromName得到UID
 * 2.opendir得到目录流
 * 3.readdir遍历目录（当返回NULL且errno不变，结束循环），获得目录名
 * 4.根据目录名open对应的status文件
 * 5.读取文件中的uid，与UID进行对比
 * 6.uid不匹配，readdir遍历下一个目录；uid匹配，输出pid，open /proc/pid/cmdline，读取并打印
 */




#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


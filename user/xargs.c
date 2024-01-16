#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define buf_size 512

// int main(int argc, char *argv[])
// {
//     char buf[buf_size + 1] = {0};
//     char *xargv[MAXARG] = {0};
//     int occupy_size = 0;  // 当前用了多少
//     int stdin_end = 0;  // 是否标准输入里面结束了

//     for (int i = 1; i < argc; i ++ ) {
//         xargv[i - 1] = argv[i];
//     }

//     while (!(stdin_end && occupy_size == 0)) {
//         // 第一部分读取管道左边的内容
//         if (!stdin_end) {
//             int remain_size = buf_size - occupy_size;
//             int read_bytes = read(0, buf + occupy_size, remain_size);
//             if (read_bytes < 0) {
//                 printf("xargs: read error\n");
//             }

//             if (read_bytes == 0) {
//                 close(0);
//                 stdin_end = 1;
//             }
//             occupy_size += read_bytes;
//         }
//         // 第二部分执行右边的程序

//         char *line_end = strchr(buf, '\n');
//         while (line_end) {
//             char xbuf[buf_size + 1] = {0};
//             memcpy(xbuf, buf, line_end - buf);  // 第一段命令
//             xargv[argc - 1] = xbuf;  // 再次加入一个参数
//             int ret = fork();
//             if (ret == 0) {
//                 if (!stdin_end) {
//                     close(0);  // 不能从标
//                 }
//                 if (exec(argv[1], xargv) < 0) {
//                     printf("xargs: exec file error\n");
//                     exit(1);
//                 }
//             } else {
//                 int prev_size = line_end - buf + 1;
//                 memcpy(buf, line_end + 1, occupy_size - prev_size);
//                 occupy_size -= prev_size;
//                 memset(buf + occupy_size, 0, buf_size - occupy_size);
//                 int pid;
//                 wait(&pid);

//                 line_end = strchr(buf, '\n');
//             }
//         }
//     }
//     exit(0);
// }

int main(int argc, char *argv[])
{
    char buf[buf_size + 1] = {0};
    char *xargv[MAXARG] = {0};

    for (int i = 1; i < argc; i ++ ) {
        xargv[i - 1] = argv[i];
    }

    // 第一部分读取
    read(0, buf, sizeof buf);
    // 第二部分执行右边的程序
    char *p = buf;  // 移动指针

    char *line_end = strchr(p, '\n');
    while (line_end) {
        char xbuf[buf_size + 1] = {0};
        memcpy(xbuf, p, line_end - p);  // 第一段命令
        xargv[argc - 1] = xbuf;  // 再次加入一个参数
        int ret = fork();
        if (ret == 0) {
            if (exec(argv[1], xargv) < 0) {
                printf("xargs: exec file error\n");
                exit(1);
            }
        } else {
            int len = line_end - p;
            p += len + 1;
            int pid;
            wait(&pid);
            line_end = strchr(p, '\n');
        }
    }
    exit(0);
}

/* 思路：
xargs的作用：把标准输入按'\n'分开，一段一段去处理，然后每一段都是后面参数的输入。
argv的格式：xargs 命令 参数1 参数2 参数3，然后剩下的参数在标准输入里面，即read(0, buf, sizeof buf)
*/
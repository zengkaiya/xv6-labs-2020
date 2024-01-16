#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    // p[0]是读，p[1]是写
    // read(文件描述符，缓存区，大小)
    // write(文件描述符，缓存区，大小)
    int p[2];
    pipe(p);
    char buf[10];

    if (fork() == 0) {  // 子进程，收到ping，写入pong
        read(p[0], buf, sizeof buf);
        fprintf(1, "%d: received %s\n", getpid(), buf);  // 靠，打错字了
        write(p[1], "pong", 10);
        exit(0);  // 子进程退出必须加exit(0)
    } else {
        write(p[1], "ping", 10);
        wait(0); // 可以&pid，0就是不管了
        read(p[0], buf, sizeof buf);
        fprintf(1, "%d: received %s\n", getpid(), buf);
    }

    exit(0);
}

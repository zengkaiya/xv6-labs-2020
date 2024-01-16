#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void listen_prime(int listen_fd)
{
    int my_num = 0;  // 存自己的数
    int pass_num = 0;  // 待传递的数
    int forked = 0;  // 一个进程只能fork一个子进程
    int p[2];  // 只要有fork的地方，基本提前都要开一个管道

    while(1) {
        int read_bytes = read(listen_fd, &pass_num, 4);

        // 读完了
        if (read_bytes == 0) {  
            close(listen_fd);  // 关闭一切用不到的fd
            if (forked) {
                close(p[1]);  // 不用再写了
                int pid;
                wait(&pid);  // 得等子进程挂了
            }
            exit(0);
        }

        // 第一次收到数字
        if (my_num == 0) {
            my_num = pass_num;
            printf("prime %d\n", my_num);
        }

        // 后面收到，且不是我的倍数，就要传递下去了
        if (pass_num % my_num != 0) {
            if (!forked) {
                pipe(p);
                forked = 1;
                int ret = fork();  // fork会继承一切内存
                if (ret == 0) {  // 子进程
                    close(p[1]);
                    close(listen_fd);
                    listen_prime(p[0]);
                } else {
                    close(p[0]);  // 父进程不需要用这个读，它是用listen_fd读的
                }
            }
            write(p[1], &pass_num, 4);
        }
        // 是我的倍数，筛掉不用管
    }
}

int main(int argc, char *argv[])  // 这个是第一个进程，不管什么时候
{
    int p[2];
    pipe(p);
    for (int i = 2; i <= 35; i ++ ) {
        write(p[1], &i, 4);  // 直接传递i的地址就可以呀，不用想着转成int*
    }

    close(p[1]);
    listen_prime(p[0]);  // 质数筛
    exit(0);
}

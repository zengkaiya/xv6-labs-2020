#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// 返回一个完整路径名下的文件名
char* basename(char* completename)
{
    char *prev = 0; // 0就是空字符串的意思
    char *curr = strchr(completename, '/'); 
    while (curr != 0) {
        prev = curr;
        curr = strchr(curr + 1, '/');
    }
    return prev;  // 这个是带一个'/'的
}

void find(char* curr_path, char* target)
{
    char buf[512], *p;  // p是备用的指针
    int fd;  // 文件描述符
    struct dirent de;  // 目录项
    struct stat st;  // 文件状态

    // 这个是从ls里面学到的
    if ((fd = open(curr_path, O_RDONLY)) < 0) {
        printf("find: cannot open %s\n", curr_path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        printf("find: cannot stat %s\n", curr_path);
        close(fd);  // 打开了的fd一定要记得关掉
        return;
    }

    switch (st.type) {
        case T_FILE:;
            char *filename = basename(curr_path);
            int match = 1;
            if (filename == 0 || strcmp(filename + 1, target) != 0) {
                match = 0;
            }

            if (match) {
                printf("%s\n", curr_path);  // 输出肯定是当前路径呀，不是文件名，不然你的buf里面存路径干啥
            }

            close(fd);
            break;
            
        case T_DIR:  // 这个要递归来读取，就是先把当前目录存入buf，然后把后面的部分继续读取
            // 1. 存目录
            memset(buf, 0, sizeof buf);
            uint curr_path_len = strlen(curr_path);
            memcpy(buf, curr_path, curr_path_len);
            buf[curr_path_len] = '/';
            p = buf + curr_path_len + 1;   // 用来凭借下一段目录的

            while (read(fd, &de, sizeof(de)) == sizeof(de)) {  // 递归寻找目录项
                if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                    continue;
                }

                memcpy(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;  // 此时又读取了一级目录
                find(buf, target);
            }
            close(fd);
            break;
    }
}


int main(int argc, char *argv[])
{
    // find + . + b，后面可以加很多参数，"."代表当前所有目录，".."代表找上一级所有目录
    if (argc != 3)
    {
        printf("usage: find [directory] [target filename]\n");
        exit(0);
    }
    
    find(argv[1], argv[2]);
    exit(0);
}

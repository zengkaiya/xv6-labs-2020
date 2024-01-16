#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// 返回最后的文件名
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];  // DIRSIZ就是一个目录名的长度
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;  // 文件描述符
  struct dirent de;  // 目录项
  struct stat st;  // 文件状态

  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){ // 因为目录名可以一直叠加，所以可能会超过buf长度
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);  // 拼接到buf后面
    p = buf+strlen(buf);  // 指针右移
    *p++ = '/'; // 加一个'/'
    while(read(fd, &de, sizeof(de)) == sizeof(de)){  // 如果是目录的话，读进dirent这个结构体里面，不停读入
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);  // 加到p后面
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit(0);
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit(0);
}

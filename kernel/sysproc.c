#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

uint64
sys_exit(void)
{
	int n;
	if (argint(0, &n) < 0)
		return -1;
	exit(n);
	return 0; // not reached
}

uint64
sys_getpid(void)
{
	return myproc()->pid;
}

uint64
sys_fork(void)
{
	return fork();
}

uint64
sys_wait(void)
{
	uint64 p;
	if (argaddr(0, &p) < 0)
		return -1;
	return wait(p);
}

uint64
sys_sbrk(void)
{
	int addr;
	int n;

	if (argint(0, &n) < 0)
		return -1;
	addr = myproc()->sz;
	if (growproc(n) < 0)
		return -1;
	return addr;
}

uint64
sys_sleep(void)
{
	int n;
	uint ticks0;

	if (argint(0, &n) < 0)
		return -1;
	acquire(&tickslock);
	ticks0 = ticks;
	while (ticks - ticks0 < n)
	{
		if (myproc()->killed)
		{
			release(&tickslock);
			return -1;
		}
		sleep(&ticks, &tickslock);
	}
	release(&tickslock);
	return 0;
}

uint64
sys_kill(void)
{
	int pid;

	if (argint(0, &pid) < 0)
		return -1;
	return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
	uint xticks;

	acquire(&tickslock);
	xticks = ticks;
	release(&tickslock);
	return xticks;
}

// The functions to retrieve system call arguments from user space are in kernel/syscall.c,
uint64 sys_trace(void)
{
	int n;
	if (argint(0, &n) < 0)
		return -1;
	
	// 开始解析 n 的二进制数
	myproc()->trace_mask = n;
	return 0;
}

uint64 sys_sysinfo(void)
{
	struct proc *my_proc = myproc();
	uint64 p;  // 这个是用来拿地址的，目前是处于系统调用状态，所以无法直接进行数据拷贝。
	if (argaddr(0, &p) < 0)
		return -1;
	struct sysinfo s;  // 直接在kernel内定义这个结构体，然后填充它就是了
	s.freemem = get_mem_bytes();
	s.nproc = get_proc_nums();
	// 从内核态拷贝到用户态
	// 拷贝len字节数的数据, 从src指向的内核地址开始, 到由pagetable下的dstv用户地址
	// 成功则返回 0, 失败返回 -1
	if (copyout(my_proc->pagetable, p, (char*)&s, sizeof(s)) < 0)  // 把kernel中的sysinfo结构体填充到用户态地址p处。
		return -1;
	return 0;
}
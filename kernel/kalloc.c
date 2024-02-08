// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

// 用于访问物理页计数数组
#define PA2PGREF_ID(p) (((p) - KERNBASE)/PGSIZE)
#define PGREF_MAX_ENTRIES PA2PGREF_ID(PHYSTOP)

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// 加个锁，防止多个进程的竞态条件引起内存泄漏
struct spinlock pgreflock;
int pageref[PGREF_MAX_ENTRIES];

// 直接访问数组
#define PA2PGREF(p) pageref[PA2PGREF_ID((uint64)p)]

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&pgreflock, "pgref");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&pgreflock);
  if (--PA2PGREF(pa) <= 0) {
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }

  // 拿了锁就一定要释放
  release(&pgreflock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  // 注意 kalloc() 可以不用加锁，因为 kmem 的锁已经保证了同一个物理页不会同时被两个进程分配，并且在 kalloc() 返回前，其他操作 pageref() 的函数也不会被调用，因为没有任何其他进程能够在 kalloc() 返回前得到这个新页的地址。

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    // 这里无需加锁
    PA2PGREF(r) = 1;
  }
   
  return (void*)r;
}

void krefpgae(void *pa)
{
  acquire(&pgreflock);
  PA2PGREF(pa) ++ ;
  release(&pgreflock);
}

void* cow_copy(void *pa)
{
  acquire(&pgreflock);
  // 当引用数小于等于1时，不用创建和分配，直接返回
  if (PA2PGREF(pa) <= 1) {
    release(&pgreflock);
    return pa;
  }

  // 创建新页
  uint64 newpa = (uint64)kalloc();
  if (newpa == 0) {
    release(&pgreflock);
    return 0;
  }
  memmove((void*)newpa, pa, PGSIZE);

  PA2PGREF(pa) --;
  release(&pgreflock);
  return (void*)newpa;
}
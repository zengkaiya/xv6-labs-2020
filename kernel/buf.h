struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint dev;
  uint blockno;
  struct sleeplock lock;
  uint refcnt;
  uint lastuse;  // used to keep track of the least-recently-used buf, instead of prev pointer
  struct buf *next;
  uchar data[BSIZE];
};
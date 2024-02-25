// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#define ARRAY_SIZE 100 // Size of the array
#define ARRAY_THRESHOLD 80 // Threshold to switch from array to linked list


void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  int free_array[ARRAY_SIZE]; // Array to manage some free list chunks
  int use_array; // Flag indicating whether to use the array or linked list
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
  kmem.use_array = 1; // Start using the array initially
}


void freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(char *v)
{
  struct run *r;
  int i;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  
  if(kmem.use_array) {
    // Try to add the frame to the array
    for(i = 0; i < ARRAY_SIZE; i++) {
      if(kmem.free_array[i]) {
        kmem.free_array[i] = 0; // Mark frame as allocated
        if(kmem.use_lock)
          release(&kmem.lock);
        return;
      }
    }
    // If array is full, switch to linked list
    kmem.use_array = 0;
  }

  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;

  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char* kalloc(void)
{
  struct run *r;
  int i;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  if(kmem.use_array) {
    // Try to allocate from the array
    for(i = 0; i < ARRAY_SIZE; i++) {
      if(kmem.free_array[i]) {
        kmem.free_array[i] = 0; // Mark frame as allocated
        if(kmem.use_lock)
          release(&kmem.lock);
        return (char*)V2P(kmem.free_array + i * PGSIZE);
      }
    }
    // If array is empty, switch to linked list
    kmem.use_array = 0;
  }

  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;

  if(kmem.use_lock)
    release(&kmem.lock);
  
  if(r)
    return (char*)r;
  return 0;
}

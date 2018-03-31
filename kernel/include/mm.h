#ifndef __MM_H__
#define __MM_H__
extern uint64_t memory_size;

void bm_realloc(void);
void init_bitmap(void);
void *pmm_alloc(size_t);
void pmm_free(void *, size_t);
void pmm_init(void);

#define PAGE_SIZE 4096
#endif

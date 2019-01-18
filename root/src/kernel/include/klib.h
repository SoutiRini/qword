#ifndef __KLIB_H__
#define __KLIB_H__

#include <stdint.h>
#include <stddef.h>
#include <task.h>

#define KPRN_MAX_TYPE 3

#define KPRN_INFO   0
#define KPRN_WARN   1
#define KPRN_ERR    2
#define KPRN_DBG    3
#define KPRN_PANIC  4

#define EMPTY ((void *)(size_t)(-1))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define ht_get(table, hash, entry, predicate) for (entry = \
        ht_get_bucket(table, hash); entry; entry = entry->next) {  \
            if (predicate) break; }

#define ht_foreach(table, entry, body) for (int i = 0; i < (table)->size; i++) \
        {   entry = (table)->buckets[i]; \
            if (!entry) continue; \
            for (; entry->next; entry = entry->next){body}}

struct ht_entry_t {
    uint64_t hash;
    struct ht_entry_t *next;
};

struct hashtable_t {
    struct ht_entry_t **buckets;
    int num_entries;
    int size;
};

int exec(pid_t, const char *, const char **, const char **);

pid_t kexec(const char *, const char **, const char **,
            const char *, const char *, const char *);

void execve_send_request(pid_t, const char *, const char **, const char **, lock_t **, int **);
void exit_send_request(pid_t, int);
void userspace_request_monitor(void *);

int ktolower(int);
char *kstrchrnul(const char *, int);
char *kstrcpy(char *, const char *);
size_t kstrlen(const char *);
int kstrcmp(const char *, const char *);
int kstrncmp(const char *, const char *, size_t);
void kprint(int type, const char *fmt, ...);
void *kalloc(size_t);
void kfree(void *);
void *krealloc(void *, size_t);

void *kmemset(void *, int, size_t);
void *kmemset64(void *, uint64_t, size_t);
void *kmemcpy(void *, const void*, size_t);
int kmemcmp(const void *, const void *, size_t);
void *kmemmove(void *, const void *, size_t);

void readline(int, const char *, char *, size_t);

int ht_init(struct hashtable_t *, int);
int ht_add(struct hashtable_t *, struct ht_entry_t*, uint64_t);
struct ht_entry_t *ht_get_bucket(struct hashtable_t *, uint64_t);
uint64_t ht_hash_str(const char *);

#endif

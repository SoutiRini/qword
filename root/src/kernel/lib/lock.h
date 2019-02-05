#ifndef __LOCK_H__
#define __LOCK_H__

#include <stdint.h>
#include <lib/qemu.h>

typedef volatile int64_t lock_t;

#define spinlock_inc(lock) ({ \
    asm volatile ( \
        "lock inc qword ptr ds:[rbx];" \
        : \
        : "b" (lock) \
    ); \
})

#define spinlock_dec(lock) ({ \
    int nz; \
    asm volatile ( \
        "lock dec qword ptr ds:[rbx];" \
        : "=@ccnz" (nz) \
        : "b" (lock) \
    ); \
    nz; \
})

#define spinlock_read(lock) ({ \
    lock_t ret; \
    asm volatile ( \
        "xor eax, eax;" \
        "lock xadd qword ptr ds:[rbx], rax;" \
        : "=a" (ret) \
        : "b" (lock) \
    ); \
    ret; \
})

#define spinlock_acquire(lock) ({ \
    __label__ retry; \
    uint64_t deadlock = 0xffffff; \
retry: \
    asm volatile ( \
        "xor eax, eax;" \
        "1: " \
        "lock xchg rax, qword ptr ds:[rbx];" \
        "test rax, rax;" \
        "jnz 2f;" \
        "dec ecx;" \
        "jnz 1b;" \
        /* deadlock */ \
        "2: " \
        : "=c" (deadlock) \
        : "b" (lock), "c" (deadlock) \
        : "rax" \
    ); \
    if (!deadlock) { \
        qemu_debug_puts("\npossible deadlock at: spinlock_acquire(" #lock ")\n"); \
        qemu_debug_puts("file: " __FILE__ "\n"); \
        qemu_debug_puts("function: "); \
        qemu_debug_puts(__func__); \
        qemu_debug_puts("\n"); \
        deadlock = 0xffffffffffffffff; \
        goto retry; \
    } \
})

#define spinlock_test_and_acquire(lock) ({ \
    lock_t ret; \
    asm volatile ( \
        "xor eax, eax;" \
        "lock xchg rax, qword ptr ds:[rbx];" \
        : "=a" (ret) \
        : "b" (lock) \
        : \
    ); \
    ret; \
})

#define spinlock_release(lock) ({ \
    asm volatile ( \
        "xor eax, eax;" \
        "inc eax;" \
        "lock xchg rax, qword ptr ds:[rbx];" \
        : \
        : "b" (lock) \
        : "rax" \
    ); \
})

#endif

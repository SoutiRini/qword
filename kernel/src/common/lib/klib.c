#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <lock.h>
#include <klib.h>
#include <serial.h>
#include <vga_textmode.h>
#include <mm.h>

int kstrcmp(const char *dst, const char *src) {
    size_t i;

    for (i = 0; dst[i] == src[i]; i++) {
        if ((!dst[i]) && (!src[i])) return 0;
    }

    return 1;
}

int kstrncmp(const char *dst, const char *src, size_t count) {
    size_t i;

    for (i = 0; i < count; i++)
        if (dst[i] != src[i]) return 1;

    return 0;
}

size_t kstrlen(const char *str) {
    size_t len;

    for (len = 0; str[len]; len++);

    return len;
}

static void kputchar(char c) {
    #ifdef _KERNEL_SERIAL_
        com1_write(c);
    #endif
    #ifdef _KERNEL_VGA_
        text_putchar(c);
    #endif
    return;
}

static void kputs(const char *string) {
    size_t i;
    
    for (i = 0; string[i]; i++) {
        kputchar(string[i]);
    }

    return;
}

static void kprn_ui(uint64_t x) {
    int i;
    char buf[21] = {0};

    if (!x) {
        kputchar('0');
        return;
    }

    for (i = 19; x; i--) {
        buf[i] = (x % 10) + 0x30;
        x = x / 10;
    }

    i++;
    kputs(buf + i);

    return;
}

static const char hex_to_ascii_tab[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

static void kprn_x(uint64_t x) {
    int i;
    char buf[17] = {0};

    if (!x) {
        kputs("0x0");
        return;
    }

    for (i = 15; x; i--) {
        buf[i] = hex_to_ascii_tab[(x % 16)];
        x = x / 16;
    }

    i++;
    kputs("0x");
    kputs(buf + i);

    return;
}

static lock_t kprint_lock = 1;

void kprint(int type, const char *fmt, ...) {
    spinlock_acquire(&kprint_lock);

    va_list args;

    va_start(args, fmt);

    /* print timestamp */
    /*kputs("["); kprn_ui(uptime_sec); kputs(".");
    kprn_ui(uptime_raw); kputs("] ");*/

    switch (type) {
        case KPRN_INFO:
            kputs("\e[36minfo\e[37m: ");
            break;
        case KPRN_WARN:
            kputs("\e[33mwarning\e[37m: ");
            break;
        case KPRN_ERR:
            kputs("\e[31mERROR\e[37m: ");
            break;
        case KPRN_DBG:
            kputs("\e[36mDEBUG\e[37m: ");
            break;
        default:
            goto out;
    }

    char *str;

    for (;;) {
        char c;
        size_t len;
        while (*fmt && *fmt != '%')
            kputchar(*(fmt++));
        if (!*fmt++) {
            va_end(args);
            kputchar('\n');
            goto out;
        }
        switch (*fmt++) {
            case 's':
                str = (char *)va_arg(args, const char *);
                if (!str)
                    kputs("(null)");
                else
                    kputs(str);
                break;
            case 'u':
                kprn_ui((uint64_t)va_arg(args, unsigned int));
                break;
            case 'U':
                kprn_ui((uint64_t)va_arg(args, uint64_t));
                break;
            case 'x':
                kprn_x((uint64_t)va_arg(args, unsigned int));
                break;
            case 'X':
                kprn_x((uint64_t)va_arg(args, uint64_t));
                break;
            case 'c':
                c = (char)va_arg(args, int);
                kputchar(c);
                break;
            default:
                kputchar('?');
                break;
        }
    }

out:
    spinlock_release(&kprint_lock);
    return;
}

typedef struct {
    size_t pages;
    size_t size;
} alloc_metadata_t;

void *kalloc(size_t size) {
    size_t page_count = size / PAGE_SIZE;

    if (size % PAGE_SIZE) page_count++;

    char *ptr = pmm_alloc(page_count + 1);

    if (!ptr) {
        return (void *)0;
    }

    alloc_metadata_t *metadata = (alloc_metadata_t *)ptr;
    ptr += PAGE_SIZE;

    metadata->pages = page_count;
    metadata->size = size;

    // Zero pages.
    for (size_t i = 0; i < (page_count * PAGE_SIZE); i++) {
        ptr[i] = 0;
    }

    return (void *)ptr;
}

void kfree(void *ptr) {
    alloc_metadata_t *metadata = (alloc_metadata_t *)((size_t)ptr - PAGE_SIZE);

    pmm_free((void *)metadata, metadata->pages + 1);
}


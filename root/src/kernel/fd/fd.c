#include <stdint.h>
#include <stddef.h>
#include <fd/fd.h>
#include <lib/lock.h>

public_dynarray_new(struct file_descriptor_t, file_descriptors);

int fd_create(struct file_descriptor_t *fd) {
    return dynarray_add(struct file_descriptor_t, file_descriptors, fd);
}

int dup(int fd) {
    struct file_descriptor_t *fd_ptr = dynarray_getelem(struct file_descriptor_t, file_descriptors, fd);
    int intern_fd = fd_ptr->intern_fd;
    int new_intern_fd = fd_ptr->fd_handler.dup(intern_fd);
    dynarray_unref(file_descriptors, fd);

    if (new_intern_fd == -1)
        return -1;

    struct file_descriptor_t new_fd = {0};

    new_fd.intern_fd = new_intern_fd;
    new_fd.fd_handler = file_descriptors[fd]->data->fd_handler;

    return fd_create(&new_fd);
}

int readdir(int fd, struct dirent *buf) {
    struct file_descriptor_t *fd_ptr = dynarray_getelem(struct file_descriptor_t, file_descriptors, fd);
    int intern_fd = fd_ptr->intern_fd;
    int ret = fd_ptr->fd_handler.readdir(intern_fd, buf);
    dynarray_unref(file_descriptors, fd);
    return ret;
}

int read(int fd, void *buf, size_t len) {
    struct file_descriptor_t *fd_ptr = dynarray_getelem(struct file_descriptor_t, file_descriptors, fd);
    int intern_fd = fd_ptr->intern_fd;
    int ret = fd_ptr->fd_handler.read(intern_fd, buf, len);
    dynarray_unref(file_descriptors, fd);
    return ret;
}

int write(int fd, const void *buf, size_t len) {
    struct file_descriptor_t *fd_ptr = dynarray_getelem(struct file_descriptor_t, file_descriptors, fd);
    int intern_fd = fd_ptr->intern_fd;
    int ret = fd_ptr->fd_handler.write(intern_fd, buf, len);
    dynarray_unref(file_descriptors, fd);
    return ret;
}

int lseek(int fd, off_t offset, int type) {
    struct file_descriptor_t *fd_ptr = dynarray_getelem(struct file_descriptor_t, file_descriptors, fd);
    int intern_fd = fd_ptr->intern_fd;
    int ret = fd_ptr->fd_handler.lseek(intern_fd, offset, type);
    dynarray_unref(file_descriptors, fd);
    return ret;
}

int fstat(int fd, struct stat *st) {
    struct file_descriptor_t *fd_ptr = dynarray_getelem(struct file_descriptor_t, file_descriptors, fd);
    int intern_fd = fd_ptr->intern_fd;
    int ret = fd_ptr->fd_handler.fstat(intern_fd, st);
    dynarray_unref(file_descriptors, fd);
    return ret;
}

int close(int fd) {
    struct file_descriptor_t fd_copy = *dynarray_getelem(struct file_descriptor_t, file_descriptors, fd);
    dynarray_unref(file_descriptors, fd);
    dynarray_remove(file_descriptors, fd);
    int intern_fd = fd_copy.intern_fd;
    if (fd_copy.fd_handler.close(intern_fd))
        return -1;
    return 0;
}

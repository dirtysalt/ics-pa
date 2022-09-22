#include <fs.h>
// #include <assert.h>

typedef size_t (*ReadFn)(void* buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void* buf, size_t offset, size_t len);

typedef struct {
    char* name;
    size_t size;
    size_t disk_offset;
    ReadFn read;
    WriteFn write;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

size_t invalid_read(void* buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t invalid_write(const void* buf, size_t offset, size_t len) {
    panic("should not reach here");
    return 0;
}

size_t serial_write(const void* buf, size_t offset, size_t len);
size_t events_read(void* buf, size_t offset, size_t len);
size_t dispinfo_read(void* buf, size_t offset, size_t len);
size_t fb_write(const void* buf, size_t offset, size_t len);

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
        [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
        [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
        [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
#include "files.h"
        {"/dev/events", 0, 0, events_read, invalid_write},
        {"/dev/fb", 0, 0, invalid_read, fb_write},
        {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
};

typedef struct FdStatus {
    size_t offset;
} FdStatus;

FdStatus fd_status[128];

void init_fs() {
    // TODO: initialize the size of /dev/fb
}

size_t ramdisk_read(void* buf, size_t offset, size_t len);
size_t ramdisk_write(const void* buf, size_t offset, size_t len);

int fs_open(const char* pathname, int flags, int mode) {
    Log("fs open filename = %s", pathname);
    size_t items = sizeof(file_table) / sizeof(file_table[0]);
    for (int i = 0; i < items; i++) {
        Finfo* info = &file_table[i];
        if (strcmp(info->name, pathname) == 0) {
            fd_status[i].offset = 0;
            return i;
        }
    }
    return -1;
}

size_t fs_read(int fd, void* buf, size_t len) {
    // check if over range.
    if (file_table[fd].size > 0 && (fd_status[fd].offset + len) > file_table[fd].size) {
        Log("read over range. fd offset = %p, len = %p, size = %p", fd_status[fd].offset, len, file_table[fd].size);
        len = file_table[fd].size - fd_status[fd].offset;
        if (len == 0) return 0;
    }

    ReadFn fn = file_table[fd].read;
    if (fn == NULL) {
        fn = ramdisk_read;
    }
    size_t offset = file_table[fd].disk_offset + fd_status[fd].offset;
    fd_status[fd].offset += len;
    size_t ret = fn(buf, offset, len);
    return ret;
}

size_t fs_write(int fd, const void* buf, size_t len) {
    WriteFn fn = file_table[fd].write;
    if (fn == NULL) {
        fn = ramdisk_write;
    }

    // ramdisk_write as write function.
    size_t offset = file_table[fd].disk_offset + fd_status[fd].offset;
    fd_status[fd].offset += len;
    size_t ret = fn(buf, offset, len);
    return ret;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
    size_t after = fd_status[fd].offset;
    if (whence == SEEK_SET) {
        after = offset;
    } else if (whence == SEEK_CUR) {
        after += offset;
    } else {
        after = file_table[fd].size + offset;
    }
    if (file_table[fd].size == 0 || after < file_table[fd].size) {
        fd_status[fd].offset = after;
    } else {
        after = file_table[fd].size;
    }
    return after;
}

int fs_close(int fd) {
    fd_status[fd].offset = 0;
    return 0;
}

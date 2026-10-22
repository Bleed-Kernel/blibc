#include <bleed/syscalls.h>

static inline long syscall_chdir(const char *path) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_CHDIR),
          "D"(path)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _chdir(const char *path) {
    return syscall_chdir(path);
}

static inline int syscall_close(int fd) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_CLOSE), "D"(fd)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _close(int fd) {
    return syscall_close(fd);
}

static inline long syscall_dup2(int oldfd, int newfd) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_DUP2), "D"(oldfd), "S"(newfd)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _dup2(int oldfd, int newfd) {
    return syscall_dup2(oldfd, newfd);
}

long _exec(const char *path, const char *const argv[], uint64_t argc) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_EXEC), "D"(path), "S"(argv), "d"(argc)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long sys_exit(long n, long a1) {
    long ret;
    asm volatile (
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void _exit(int code) {
    sys_exit(SYS_EXIT, code);
    __builtin_unreachable();
}

long _fork(void) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_FORK)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall_getcwd(char *buf, long size) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_GETCWD),
          "D"(buf),
          "S"(size)
        : "rcx", "r11", "memory"
    );
    return ret;
}

char *_getcwd(char *buf, long size) {
    if (!buf) return NULL;
    long r = syscall_getcwd(buf, size);
    if (r < 0) return NULL;
    return buf;
}

long _getpid(void) {
    long ret;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_GETPID)
        : "rcx", "r11", "memory"
    );

    return ret;
}

static inline long syscall_ioctl(int fd, unsigned long request, void* arg) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_IOCTL),
          "D"(fd),
          "S"(request),
          "d"(arg)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _ioctl(int fd, unsigned long request, void* arg) {
    return (int)syscall_ioctl(fd, request, arg);
}

long _ipc_send(uint64_t target_pid, uint64_t src_addr, uint64_t pages) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_IPC_SEND), "D"(target_pid), "S"(src_addr), "d"(pages)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _ipc_recv(ipc_message_t *msg) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_IPC_RECV), "D"(msg)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _kill(long pid, long sig) {
    long ret;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_KILL),
          "D"(pid),
          "S"(sig)
        : "rcx", "r11", "memory"
    );

    return ret;
}

static inline void* syscall_mapfb(size_t *out_pages) {
    void *ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MAPFB),
          "D"(out_pages)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void* mapfb(size_t *out_pages) {
    return syscall_mapfb(out_pages);
}

static inline int syscall_mkdir(const char *path, int mode) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MKDIR), "D"(path), "S"(mode)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _mkdir(const char *path, int mode) {
    return syscall_mkdir(path, mode);
}

static inline void* syscall_mmap(size_t pages) {
    void* ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MMAP), "D"(pages)
        : "rcx", "r11", "memory"
    );
    if ((uintptr_t)ret == (uintptr_t)-1)
        return NULL;
    return ret;
}

void* _mmap(unsigned long pages) {
    return syscall_mmap(pages);
}

static inline void syscall_munmap(void* addr) {
    asm volatile(
        "syscall"
        :
        : "a"(SYS_MUNMAP), "D"(addr)
        : "rcx", "r11", "memory"
    );
}

void _munmap(void* addr) {
    syscall_munmap(addr);
}

static inline long syscall_open(const char *path, int flags) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_OPEN),
          "D"(path),
          "S"(flags)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _open(const char *path, int flags) {
    return (int)syscall_open(path, flags);
}

static inline long syscall_pipe(int fds[2]) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_PIPE), "D"(fds)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _pipe(int fds[2]) {
    return syscall_pipe(fds);
}

static inline long syscall_read(int fd, void *buf, unsigned long len) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_READ),
          "D"(fd),
          "S"(buf),
          "d"(len)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _read(int fd, void *buf, unsigned long len){
    return syscall_read(fd, buf, len);
}

static inline long syscall_readdir(int fd, size_t index, dirent_t *out) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_READDIR),
          "D"(fd),
          "S"(index),
          "d"(out)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _readdir(int fd, size_t index, dirent_t *out) {
    if (!out) return -1;
    return syscall_readdir(fd, index, out);
}

static inline int syscall_rename(const char *oldpath, const char *newpath) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_RENAME), "D"(oldpath), "S"(newpath)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _rename(const char *oldpath, const char *newpath) {
    return syscall_rename(oldpath, newpath);
}

static inline long syscall_seek(int fd, long offset, int whence) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SEEK),
          "D"(fd),
          "S"(offset),
          "d"(whence)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _seek(int fd, long offset, int whence) {
    return syscall_seek(fd, offset, whence);
}

static inline long syscall_sigaction(int sig, const sigaction_t *act, sigaction_t *oldact) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SIGACTION),
          "D"(sig),
          "S"(act),
          "d"(oldact)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _sigaction(int sig, const sigaction_t *act, sigaction_t *oldact) {
    return syscall_sigaction(sig, act, oldact);
}

static inline long syscall_sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SIGPROCMASK),
          "D"(how),
          "S"(set),
          "d"(oldset)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    return syscall_sigprocmask(how, set, oldset);
}

long _sigreturn(void) {
    long ret;

    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SIGRETURN)
        : "rcx", "r11", "memory"
    );

    return ret;
}

static inline long syscall_stat(const char *path, struct stat *st) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_STAT),
          "D"(path),
          "S"(st)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _stat(const char *path, struct stat *st) {
    return syscall_stat(path, st);
}

static inline int syscall_meminfo(system_memory_info_t *buf) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MEMINFO), "D"(buf)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _meminfo(system_memory_info_t *buf){
    return syscall_meminfo(buf);
}

static inline uint64_t syscall_taskcount(void) {
    uint64_t ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_TASKCOUNT)
        : "rcx", "r11", "memory"
    );
    return ret;
}

uint64_t _taskcount(void){
    return syscall_taskcount();
}

static inline uint64_t syscall_taskinfo(uint64_t pid, user_task_info_t *info) {
    uint64_t ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_TASKINFO), "D"(pid), "S"(info)
        : "rcx", "r11", "memory"
    );
    return ret;
}

uint64_t _taskinfo(uint64_t pid, user_task_info_t *info){
    return syscall_taskinfo(pid, info);
}

static inline long syscall_taskusage(uint64_t pid, user_task_usage_t *usage) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_TASKUSAGE), "D"(pid), "S"(usage)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _taskusage(uint64_t pid, user_task_usage_t *usage){
    return syscall_taskusage(pid, usage);
}

static inline int syscall_time(rtc_time_t *buf) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_TIME), "D"(buf)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _time(rtc_time_t *buf) {
    return syscall_time(buf);
}

static inline int syscall_unlink(const char *path) {
    int ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_UNLINK), "D"(path)
        : "rcx", "r11", "memory"
    );
    return ret;
}

int _unlink(const char *path) {
    return syscall_unlink(path);
}

static inline long syscall_waitpid(long pid){
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_WAITPID), "D"(pid)
        : "rcx", "r11", "memory"
    );

    return ret;
}

long _waitpid(long pid){
    return syscall_waitpid(pid);
}

static inline long sys_write(long n, long a1, long a2, long a3) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _write(int fd, const void *buf, long len) {
    return sys_write(SYS_WRITE, fd, (long)buf, len);
}

static inline void syscall_yield() {
    __asm__ volatile (
        "syscall"
        :
        : "a"(SYS_YIELD)
        : "rcx", "r11", "memory"
    );
}

void _yeild(){
    syscall_yield();
}

static inline long syscall_poll(struct pollfd *fds, unsigned long nfds, long timeout_ms) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_POLL), "D"(fds), "S"(nfds), "d"(timeout_ms)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _poll(struct pollfd *fds, unsigned long nfds, long timeout_ms) {
    return syscall_poll(fds, nfds, timeout_ms);
}

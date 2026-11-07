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

long _dup(int fd) {
    long ret;
    asm volatile("syscall" : "=a"(ret) : "a"(SYS_DUP), "D"((long)fd) : "rcx", "r11", "memory");
    return ret;
}

long _getppid(void) {
    long ret;
    asm volatile("syscall" : "=a"(ret) : "a"(SYS_GETPPID) : "rcx", "r11", "memory");
    return ret;
}

int _openat(int dirfd, const char *path, int flags) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_OPENAT), "D"((long)dirfd), "S"(path), "d"((long)flags)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

long _symlinkat(const char *target, int dirfd, const char *path) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_SYMLINKAT), "D"(target), "S"((long)dirfd), "d"(path)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _readlinkat(int dirfd, const char *path, char *buf, size_t size) {
    long ret;
    register long r10 asm("r10") = (long)size;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_READLINKAT), "D"((long)dirfd), "S"(path), "d"(buf), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _fstatat(int dirfd, const char *path, struct bleed_stat *st, int flags) {
    long ret;
    register long r10 asm("r10") = flags;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_FSTATAT), "D"((long)dirfd), "S"(path), "d"(st), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
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

long _execve(const char *path, const char *const argv[], uint64_t argc, const char *const envp[]) {
    long ret;
    register const char *const *r10 asm("r10") = envp;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_EXECVE), "D"(path), "S"(argv), "d"(argc), "r"(r10)
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

// the whole process, SYS_EXIT only ends the calling thread now
void _exit(int code) {
    sys_exit(SYS_EXIT_GROUP, code);
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

static inline long syscall_mmap(void *addr, size_t len, int prot, int flags, int fd, long off) {
    long ret;
    register long r10 asm("r10") = flags;
    register long r8 asm("r8") = fd;
    register long r9 asm("r9") = off;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MMAP), "D"(addr), "S"(len), "d"(prot), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _mmap_full(void *addr, size_t len, int prot, int flags, int fd, long off) {
    return syscall_mmap(addr, len, prot, flags, fd, off);
}

// zeroed read write pages, filled in as theyre touched
void* _mmap(unsigned long pages) {
    long ret = syscall_mmap(NULL, pages * 4096, 0x3, 0x22, -1, 0);
    return ret < 0 ? NULL : (void *)ret;
}

static inline long syscall_munmap(void* addr, size_t len) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MUNMAP), "D"(addr), "S"(len)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _munmap_range(void *addr, size_t len) {
    return syscall_munmap(addr, len);
}

// the whole mapping that starts at addr
void _munmap(void* addr) {
    syscall_munmap(addr, 0);
}

long _madvise(void *addr, size_t len, int advice) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MADVISE), "D"(addr), "S"(len), "d"(advice)
        : "rcx", "r11", "memory"
    );
    return ret;
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

static inline long syscall_pipe(int fds[2], int flags) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_PIPE), "D"(fds), "S"((long)flags)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _pipe(int fds[2]) {
    return syscall_pipe(fds, 0);
}

long _pipe2(int fds[2], int flags) {
    return syscall_pipe(fds, flags);
}

static inline long syscall3(long n, long a, long b, long c) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a), "S"(b), "d"(c)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _fcntl(int fd, int cmd, long arg) {
    return syscall3(SYS_FCNTL, fd, cmd, arg);
}

long _dup3(int oldfd, int newfd, int flags) {
    return syscall3(SYS_DUP3, oldfd, newfd, flags);
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

static inline long syscall_sigprocmask(int how, const void *set, void *oldset) {
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

long _sigprocmask(int how, const void *set, void *oldset) {
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

static inline long syscall_stat(const char *path, struct bleed_stat *st) {
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

long _stat(const char *path, struct bleed_stat *st) {
    return syscall_stat(path, st);
}

static inline long syscall_fstat(int fd, struct bleed_stat *st) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_FSTAT),
          "D"((long)fd),
          "S"(st)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _fstat(int fd, struct bleed_stat *st) {
    return syscall_fstat(fd, st);
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

long _mountinfo(uint64_t index, user_mount_info_t *info) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MOUNTINFO), "D"(index), "S"(info)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _cpuinfo(uint64_t index, user_cpu_info_t *info) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_CPUINFO), "D"(index), "S"(info)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _mountlabel(uint64_t index, char *buf, uint64_t size) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MOUNTLABEL), "D"(index), "S"(buf), "d"(size)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _taskpath(uint64_t pid, char *buf, uint64_t size) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_TASKPATH), "D"(pid), "S"(buf), "d"(size)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _mprotect(void *addr, unsigned long len, int prot) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_MPROTECT), "D"(addr), "S"(len), "d"((long)prot)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _arch_prctl(long code, uint64_t addr) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_ARCH_PRCTL), "D"(code), "S"(addr)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _waitopts(long pid, int *status, long options) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_WAITOPTS), "D"(pid), "S"(status), "d"(options)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _waitstatus(long pid, int *status) {
    long ret;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(SYS_WAITSTATUS), "D"(pid), "S"(status)
        : "rcx", "r11", "memory"
    );
    return ret;
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

long _sigaltstack(const stack_t *ss, stack_t *old) {
    return syscall3(SYS_SIGALTSTACK, (long)ss, (long)old, 0);
}

long _sched_getaffinity(long pid, size_t len, void *mask) {
    return syscall3(SYS_SCHED_GETAFFINITY, pid, (long)len, (long)mask);
}

static inline long syscall4(long n, long a, long b, long c, long d) {
    long ret;
    register long r10 __asm__("r10") = d;
    asm volatile(
        "syscall"
        : "=a"(ret)
        : "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

long _pread64(int fd, void *buf, size_t len, long offset) {
    return syscall4(SYS_PREAD64, fd, (long)buf, (long)len, offset);
}

long _pwrite64(int fd, const void *buf, size_t len, long offset) {
    return syscall4(SYS_PWRITE64, fd, (long)buf, (long)len, offset);
}

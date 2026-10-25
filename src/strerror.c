#include <errno.h>
#include <stdio.h>
#include <string.h>

char *strerror(int errnum) {
    switch (errnum) {
    case 0:
        return "Success";
    case EPERM:
        return "Operation not permitted";
    case ENOENT:
        return "No such file or directory";
    case ESRCH:
        return "No such process";
    case EINTR:
        return "Interrupted system call";
    case EIO:
        return "Input/output error";
    case ENXIO:
        return "No such device or address";
    case E2BIG:
        return "Argument list too long";
    case ENOEXEC:
        return "Exec format error";
    case EBADF:
        return "Bad file descriptor";
    case ECHILD:
        return "No child processes";
    case EAGAIN:
        return "Resource temporarily unavailable";
    case ENOMEM:
        return "Cannot allocate memory";
    case EACCES:
        return "Permission denied";
    case EFAULT:
        return "Bad address";
    case EBUSY:
        return "Device or resource busy";
    case EEXIST:
        return "File exists";
    case EXDEV:
        return "Invalid cross-device link";
    case ENODEV:
        return "No such device";
    case ENOTDIR:
        return "Not a directory";
    case EISDIR:
        return "Is a directory";
    case EINVAL:
        return "Invalid argument";
    case ENFILE:
        return "Too many open files in system";
    case EMFILE:
        return "Too many open files";
    case ENOTTY:
        return "Inappropriate ioctl for device";
    case ETXTBSY:
        return "Text file busy";
    case EFBIG:
        return "File too large";
    case ENOSPC:
        return "No space left on device";
    case ESPIPE:
        return "Illegal seek";
    case EROFS:
        return "Read-only file system";
    case EMLINK:
        return "Too many links";
    case EPIPE:
        return "Broken pipe";
    case EDOM:
        return "Numerical argument out of domain";
    case ERANGE:
        return "Numerical result out of range";
    case ENOSYS:
        return "Function not implemented";
    case ENOTEMPTY:
        return "Directory not empty";
    case ELOOP:
        return "Too many levels of symbolic links";
    case ENOTSOCK:
        return "Socket operation on non-socket";
    case EOPNOTSUPP:
        return "Operation not supported";
    case EAFNOSUPPORT:
        return "Address family not supported by protocol";
    case EADDRINUSE:
        return "Address already in use";
    case ENETDOWN:
        return "Network is down";
    case ENETUNREACH:
        return "Network is unreachable";
    case ECONNRESET:
        return "Connection reset by peer";
    case ENOTCONN:
        return "Transport endpoint is not connected";
    case ETIMEDOUT:
        return "Connection timed out";
    case ECONNREFUSED:
        return "Connection refused";
    case EHOSTUNREACH:
        return "No route to host";
    case EINPROGRESS:
        return "Operation now in progress";
    default: {
        static char unknown[48];
        (void)snprintf(unknown, sizeof(unknown), "Unknown error %d", errnum);
        return unknown;
    }
    }
}

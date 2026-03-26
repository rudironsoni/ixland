#include "exec.h"
#include "../../fs/fdtable.h"
#include "../signal/iox_signal.h"
#include "../../runtime/native/registry.h"
#include "iox/iox_types.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

iox_image_type_t iox_exec_classify(const char *path) {
    /* First check native registry - registered commands take precedence */
    if (iox_native_lookup(path)) {
        return IOX_IMAGE_NATIVE;
    }
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return IOX_IMAGE_NONE;
    }
    
    unsigned char magic[4];
    ssize_t n = read(fd, magic, 4);
    close(fd);
    
    if (n < 2) {
        return IOX_IMAGE_NONE;
    }
    
    /* WASM magic: \0asm (0x00 0x61 0x73 0x6d) */
    if (n >= 4 && magic[0] == 0x00 && magic[1] == 0x61 && magic[2] == 0x73 && magic[3] == 0x6d) {
        return IOX_IMAGE_WASI;
    }
    
    /* Script: #! */
    if (magic[0] == '#' && magic[1] == '!') {
        return IOX_IMAGE_SCRIPT;
    }
    
    return IOX_IMAGE_NONE;
}

int iox_exec_close_cloexec(iox_task_t *task) {
    if (!task || !task->files) {
        errno = EINVAL;
        return -1;
    }
    
    pthread_mutex_lock(&task->files->lock);
    for (size_t i = 0; i < task->files->max_fds; i++) {
        if (task->files->fd[i] && (task->files->fd[i]->flags & FD_CLOEXEC)) {
            iox_file_t *file = task->files->fd[i];
            task->files->fd[i] = NULL;
            iox_file_free(file);
        }
    }
    pthread_mutex_unlock(&task->files->lock);
    
    return 0;
}

void iox_exec_reset_signals(iox_sighand_t *sighand) {
    if (!sighand) return;
    
    for (int i = 0; i < IOX_NSIG; i++) {
        /* Reset caught signals to default */
        if (sighand->action[i].sa_handler != SIG_IGN) {
            sighand->action[i].sa_handler = SIG_DFL;
        }
    }
    
    /* Clear signal mask */
    sigemptyset(&sighand->blocked);
    
    /* Clear pending signals */
    sigemptyset(&sighand->pending);
}

/* Internal: Ensure task has an exec_image allocated */
static int iox_exec_image_ensure(iox_task_t *task) {
    if (!task) {
        errno = EINVAL;
        return -1;
    }
    
    if (task->exec_image) {
        return 0;  /* Already allocated */
    }
    
    task->exec_image = calloc(1, sizeof(iox_exec_image_t));
    if (!task->exec_image) {
        errno = ENOMEM;
        return -1;
    }
    
    return 0;
}

int iox_execve(const char *pathname, char *const argv[], char *const envp[]) {
    if (!pathname) {
        errno = EFAULT;
        return -1;
    }
    
    iox_task_t *task = iox_current_task();
    if (!task) {
        errno = ESRCH;
        return -1;
    }
    
    /* Classify image */
    iox_image_type_t type = iox_exec_classify(pathname);
    if (type == IOX_IMAGE_NONE) {
        errno = ENOENT;
        return -1;
    }
    
    /* Ensure exec_image is allocated before any modifications */
    if (iox_exec_image_ensure(task) < 0) {
        /* errno set by helper */
        return -1;
    }
    
    /* Close CLOEXEC fds */
    iox_exec_close_cloexec(task);
    
    /* Reset signal handlers */
    if (task->sighand) {
        iox_exec_reset_signals(task->sighand);
    }
    
    /* Count args */
    int argc = 0;
    if (argv) {
        while (argv[argc]) argc++;
    }
    
    /* Dispatch to handler */
    int ret;
    switch (type) {
    case IOX_IMAGE_NATIVE:
        ret = iox_exec_native(task, pathname, argc, (char **)argv, (char **)envp);
        break;
    case IOX_IMAGE_WASI:
        ret = iox_exec_wasi(task, pathname, argc, (char **)argv, (char **)envp);
        break;
    case IOX_IMAGE_SCRIPT:
        ret = iox_exec_script(task, pathname, argc, (char **)argv, (char **)envp);
        break;
    default:
        errno = ENOEXEC;
        ret = -1;
    }
    
    /* If exec fails, we return with exec_image still allocated (coherent) */
    return ret;
}

int iox_exec_native(iox_task_t *task, const char *path, int argc, char **argv, char **envp) {
    iox_native_entry_t entry = iox_native_lookup(path);
    if (!entry) {
        errno = ENOENT;
        return -1;
    }
    
    /* Update exec image metadata - by this point exec_image is guaranteed allocated */
    strncpy(task->exec_image->path, path, sizeof(task->exec_image->path) - 1);
    task->exec_image->path[sizeof(task->exec_image->path) - 1] = '\0';
    task->exec_image->type = IOX_IMAGE_NATIVE;
    
    /* Dispatch to native entry point */
    return entry(task, argc, argv, envp);
}

int iox_exec_wasi(iox_task_t *task, const char *path, int argc, char **argv, char **envp) {
    (void)task;
    (void)path;
    (void)argc;
    (void)argv;
    (void)envp;
    errno = ENOEXEC;
    return -1;
}

int iox_exec_script(iox_task_t *task, const char *path, int argc, char **argv, char **envp) {
    (void)task;
    (void)path;
    (void)argc;
    (void)argv;
    (void)envp;
    errno = ENOEXEC;
    return -1;
}

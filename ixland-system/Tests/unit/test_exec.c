#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../fs/fdtable.h"
#include "../../kernel/exec/exec.h"
#include "../../kernel/task/task.h"
#include "../../runtime/native/registry.h"
#include "../harness/iox_test.h"

/* Stub command capture structure */
typedef struct {
    iox_task_t *task;
    int argc;
    char **argv;
    char **envp;
    int called;
} stub_capture_t;

static stub_capture_t g_capture = {0};

/* Test stub native command */
static int test_stub_cmd(iox_task_t *task, int argc, char **argv, char **envp) {
    g_capture.task = task;
    g_capture.argc = argc;
    g_capture.argv = argv;
    g_capture.envp = envp;
    g_capture.called = 1;
    return 0;
}

IOX_TEST(exec_native_happy_path) {
    /* Initialize task system */
    IOX_ASSERT(iox_task_init() == 0);

    /* Register stub command */
    IOX_ASSERT(iox_native_register("/bin/testcmd", test_stub_cmd) == 0);

    /* Get current task */
    iox_task_t *task = iox_current_task();
    IOX_ASSERT_NOT_NULL(task);

    /* Capture pre-exec identity */
    pid_t pid_before = task->pid;
    pid_t pgid_before = task->pgid;
    pid_t sid_before = task->sid;

    /* Prepare args and env */
    char *argv[] = {"testcmd", "arg1", "arg2", NULL};
    char *envp[] = {"FOO=bar", NULL};

    /* Clear capture */
    memset(&g_capture, 0, sizeof(g_capture));

    /* Execute */
    int ret = iox_execve("/bin/testcmd", argv, envp);

    /* Verify exec succeeded */
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_EQ(g_capture.called, 1);

    /* Verify task identity preserved */
    IOX_ASSERT_EQ(g_capture.task, task);
    IOX_ASSERT_EQ(task->pid, pid_before);
    IOX_ASSERT_EQ(task->pgid, pgid_before);
    IOX_ASSERT_EQ(task->sid, sid_before);

    /* Verify exec image metadata updated */
    IOX_ASSERT_NOT_NULL(task->exec_image);
    IOX_ASSERT_EQ(task->exec_image->type, IOX_IMAGE_NATIVE);
    IOX_ASSERT_NOT_NULL(task->exec_image->path);
    IOX_ASSERT(strcmp(task->exec_image->path, "/bin/testcmd") == 0);

    /* Verify argv and env passed correctly */
    IOX_ASSERT_EQ(g_capture.argc, 3);
    IOX_ASSERT_NOT_NULL(g_capture.argv);
    IOX_ASSERT(strcmp(g_capture.argv[0], "testcmd") == 0);
    IOX_ASSERT(strcmp(g_capture.argv[1], "arg1") == 0);
    IOX_ASSERT(strcmp(g_capture.argv[2], "arg2") == 0);
    IOX_ASSERT_NOT_NULL(g_capture.envp);
    IOX_ASSERT(strcmp(g_capture.envp[0], "FOO=bar") == 0);

    /* Cleanup */
    iox_native_registry_clear();
    return true;
}

IOX_TEST(exec_command_not_found) {
    /* Initialize */
    IOX_ASSERT(iox_task_init() == 0);

    /* Try to execute unknown command */
    char *argv[] = {"unknown", NULL};
    int ret = iox_execve("/bin/unknown", argv, NULL);

    /* Should fail with ENOENT */
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT(errno == ENOENT);

    return true;
}

IOX_TEST(exec_invalid_path) {
    /* Initialize */
    IOX_ASSERT(iox_task_init() == 0);

    /* Try NULL path */
    int ret = iox_execve(NULL, NULL, NULL);
    IOX_ASSERT_EQ(ret, -1);
    IOX_ASSERT(errno == EFAULT);

    return true;
}

IOX_TEST(exec_cloexec_behavior) {
    /* Initialize */
    IOX_ASSERT(iox_task_init() == 0);
    iox_task_t *task = iox_current_task();
    IOX_ASSERT_NOT_NULL(task);
    IOX_ASSERT_NOT_NULL(task->files);

    /* Register stub command */
    IOX_ASSERT(iox_native_register("/bin/cloexec_test", test_stub_cmd) == 0);

    /* Create a pipe to get two FDs */
    int pipe_fds[2];
    IOX_ASSERT(pipe(pipe_fds) == 0);

    /* Add both FDs to task's file table */
    iox_file_t *file0 = iox_file_alloc();
    iox_file_t *file1 = iox_file_alloc();
    IOX_ASSERT_NOT_NULL(file0);
    IOX_ASSERT_NOT_NULL(file1);

    file0->fd = pipe_fds[0];
    file1->fd = pipe_fds[1];

    /* Mark file1 as CLOEXEC */
    file1->flags = FD_CLOEXEC;

    /* Allocate in table (use high FD numbers to avoid stdin/stdout/stderr) */
    int fd0 = 10;
    int fd1 = 11;

    pthread_mutex_lock(&task->files->lock);
    if (fd0 < (int)task->files->max_fds) {
        task->files->fd[fd0] = file0;
        atomic_fetch_add(&file0->refs, 1);
    }
    if (fd1 < (int)task->files->max_fds) {
        task->files->fd[fd1] = file1;
        atomic_fetch_add(&file1->refs, 1);
    }
    pthread_mutex_unlock(&task->files->lock);

    /* Verify both FDs exist */
    IOX_ASSERT(task->files->fd[fd0] != NULL);
    IOX_ASSERT(task->files->fd[fd1] != NULL);

    /* Clear capture */
    memset(&g_capture, 0, sizeof(g_capture));

    /* Execute */
    char *argv[] = {"cloexec_test", NULL};
    int ret = iox_execve("/bin/cloexec_test", argv, NULL);

    /* Verify exec succeeded */
    IOX_ASSERT_EQ(ret, 0);
    IOX_ASSERT_EQ(g_capture.called, 1);

    /* Verify non-CLOEXEC fd still exists */
    IOX_ASSERT(task->files->fd[fd0] != NULL);

    /* Verify CLOEXEC fd was closed */
    IOX_ASSERT(task->files->fd[fd1] == NULL);

    /* Cleanup */
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    iox_native_registry_clear();
    return true;
}

IOX_TEST(exec_image_classification_wasm) {
    /* Create temp file with WASM magic */
    char path[] = "iox_test_wasm_XXXXXX";
    int fd = mkstemp(path);
    IOX_ASSERT(fd >= 0);

    /* Write WASM magic: \0asm */
    unsigned char wasm_magic[] = {0x00, 0x61, 0x73, 0x6d};
    ssize_t written = write(fd, wasm_magic, 4);
    IOX_ASSERT(written == 4);

    /* Ensure data is written before classification reads it */
    fsync(fd);
    close(fd);

    /* Clear registry first so file magic takes precedence */
    iox_native_registry_clear();

    /* Classify */
    iox_image_type_t type = iox_exec_classify(path);
    IOX_ASSERT_EQ(type, IOX_IMAGE_WASI);

    /* Cleanup */
    unlink(path);
    return true;
}

IOX_TEST(exec_image_classification_script) {
    /* Create temp file with shebang - use CWD for iOS simulator compatibility */
    char path[] = "iox_test_script_XXXXXX";
    int fd = mkstemp(path);
    IOX_ASSERT(fd >= 0);

    IOX_ASSERT(write(fd, "#!/bin/sh\n", 10) == 10);
    fsync(fd);
    close(fd);

    /* Classify */
    iox_image_type_t type = iox_exec_classify(path);
    IOX_ASSERT_EQ(type, IOX_IMAGE_SCRIPT);

    /* Cleanup */
    unlink(path);
    return true;
}

IOX_TEST(exec_image_classification_native) {
    /* Initialize */
    IOX_ASSERT(iox_task_init() == 0);

    /* Register a command */
    IOX_ASSERT(iox_native_register("/bin/nativetest", test_stub_cmd) == 0);

    /* Create temp file (no special magic) */
    char path[] = "/tmp/iox_test_native_XXXXXX";
    int fd = mkstemp(path);
    IOX_ASSERT(fd >= 0);
    IOX_ASSERT(write(fd, "binary content\n", 15) == 15);
    close(fd);

    /* Register this path */
    IOX_ASSERT(iox_native_register(path, test_stub_cmd) == 0);

    /* Classify - should find in registry */
    iox_image_type_t type = iox_exec_classify(path);
    IOX_ASSERT_EQ(type, IOX_IMAGE_NATIVE);

    /* Cleanup */
    unlink(path);
    iox_native_registry_clear();
    return true;
}

IOX_TEST(exec_image_classification_none) {
    /* Try non-existent file */
    iox_image_type_t type = iox_exec_classify("/nonexistent/path");
    IOX_ASSERT_EQ(type, IOX_IMAGE_NONE);

    return true;
}

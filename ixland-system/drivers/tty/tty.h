#ifndef IOX_TTY_H
#define IOX_TTY_H

#include <pthread.h>
#include <stdbool.h>
#include <termios.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IOX_MAX_PTS 64

typedef struct iox_tty iox_tty_t;
typedef struct iox_pty iox_pty_t;

struct iox_tty {
    int tty_id;
    pid_t foreground_pgrp;
    struct termios termios;
    struct winsize winsize;
    bool is_session_leader;
    atomic_int refs;
    pthread_mutex_t lock;
};

struct iox_pty {
    int master_fd;
    int slave_fd;
    char slave_name[64];
    pid_t session_leader;
    iox_tty_t *tty;
    atomic_int refs;
};

iox_tty_t *iox_tty_alloc(int id);
void iox_tty_free(iox_tty_t *tty);
int iox_tty_set_foreground(iox_tty_t *tty, pid_t pgrp);
pid_t iox_tty_get_foreground(iox_tty_t *tty);

iox_pty_t *iox_pty_open(void);
void iox_pty_close(iox_pty_t *pty);
int iox_pty_master_read(iox_pty_t *pty, void *buf, size_t count);
int iox_pty_master_write(iox_pty_t *pty, const void *buf, size_t count);
int iox_pty_slave_read(iox_pty_t *pty, void *buf, size_t count);
int iox_pty_slave_write(iox_pty_t *pty, const void *buf, size_t count);
int iox_pty_set_size(iox_pty_t *pty, int rows, int cols);

int iox_tcgetattr(int fd, struct termios *termios_p);
int iox_tcsetattr(int fd, int optional_actions, const struct termios *termios_p);
int iox_tcgetpgrp(int fd);
int iox_tcsetpgrp(int fd, pid_t pgrp);
int iox_tcflush(int fd, int queue_selector);
int iox_tcdrain(int fd);
int iox_tcsendbreak(int fd, int duration);
int iox_tcdrain(int fd);

int iox_tty_init(void);
void iox_tty_deinit(void);

#ifdef __cplusplus
}
#endif

#endif

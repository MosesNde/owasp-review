#include <stdlib.h>
#include <pthread.h>

static pthread_mutex_t extras_mutex = PTHREAD_MUTEX_INITIALIZER;

void ap_register_extra_mpm_process(pid_t pid, uid_t current_user_uid, uid_t allowed_uid)
{
    if (current_user_uid != allowed_uid) {
        return;
    }
    extra_process_t *p = (extra_process_t *)malloc(sizeof(extra_process_t));
    if (!p) {
        return;
    }
    p->pid = pid;
    pthread_mutex_lock(&extras_mutex);
    p->next = extras;
    extras = p;
    pthread_mutex_unlock(&extras_mutex);
}
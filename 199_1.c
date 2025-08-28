#include <sys/shm.h>
#include <sys/ipc.h>
#include <errno.h>

static apr_status_t unixd_set_shm_perms(const char *fname)
{
#ifdef AP_NEED_SET_MUTEX_PERMS
#if APR_USE_SHMEM_SHMGET || APR_USE_SHMEM_SHMGET_ANON
    struct shmid_ds shmbuf;
    key_t shmkey;
    int shmid;
    shmkey = ftok(fname, 1);
    if (shmkey == (key_t)-1) {
        return errno;
    }
    if ((shmid = shmget(shmkey, 0, SHM_R | SHM_W)) == -1) {
        return errno;
    }
#if MODULE_MAGIC_NUMBER_MAJOR <= 20081212
#define ap_unixd_config unixd_config
#endif
    shmbuf.shm_perm.uid  = ap_unixd_config.user_id;
    shmbuf.shm_perm.gid  = ap_unixd_config.group_id;
    shmbuf.shm_perm.mode = 0600;
    if (shmctl(shmid, IPC_SET, &shmbuf) == -1) {
        return errno;
    }
    return APR_SUCCESS;
#else
    return APR_ENOTIMPL;
#endif
#else
    return APR_ENOTIMPL;
#endif
}
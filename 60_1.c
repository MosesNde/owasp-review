#include <stdlib.h>

void ap_register_extra_mpm_process(pid_t pid)
{
    extra_process_t *p = (extra_process_t *)malloc(sizeof(extra_process_t));
    p->next = extras;
    p->pid = pid;
    extras = p;
}
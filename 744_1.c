#include <string.h>

typedef struct {
    const char *t_name;
    int t_val;
} priority_t;

priority_t priorities[] = {
    {"emerg", 0}, {"alert", 1}, {"crit", 2}, {"error", 3}, {"warn", 4},
    {"notice", 5}, {"info", 6}, {"debug", 7}, {"trace1", 8}, {"trace2", 9},
    {"trace3", 10}, {"trace4", 11}, {"trace5", 12}, {"trace6", 13}, {"trace7", 14}, {"trace8", 15},
    {NULL, -1}
};

const char *ap_parse_log_level(const char *str, int *val) {
    char *err = "Log level keyword must be one of emerg/alert/crit/error/warn/notice/info/debug/trace1/.../trace8";
    int i = 0;
    if (str == NULL)
        return err;
    while (priorities[i].t_name != NULL) {
        if (!strcasecmp(str, priorities[i].t_name)) {
            *val = priorities[i].t_val;
            return NULL;
        }
        i++;
    }
    return err;
}
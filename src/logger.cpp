#include "logger.h"
#include "passthrough.h"
#include "config.h"

#include <time.h>
#include <string.h>
#include <stdlib.h>

const char *get_lvlname(int lvl)
{
    switch (lvl)
    {
    case 1:
        return "ERROR";
    case 2:
        return "WARNING";
    case 3:
        return "INFO";
    default:
        return "DEBUG";
    }
}

int log_msg(int lvl, const char *msg, ...)
{
    config sea_config = get_sea_config();
    char *log_fn = sea_config.log_file;
    int debug_lvl = sea_config.log_level;
    if (lvl > debug_lvl)
        return 0;

    //get current time
    time_t t;
    time(&t);

    //format input string
    char fmsg[MAX_LOG];
    va_list arglist;
    va_start(arglist, msg);
    vsprintf(fmsg, msg, arglist);
    va_end(arglist);

    if (LOG_FOREGROUND)
        fprintf(stderr, "%ld: %s: %s\n", t, get_lvlname(lvl), fmsg);

    else
    {
        FILE *logs = ((funcptr_fopen)libc_fopen)(log_fn, "a+");
        // write complete log string to file
        if (logs == NULL)
        {
            xprintf("WARNING: Cannot write to log file: %s (%s)\n", fmsg, get_lvlname(lvl));
            return 1;
        }
        fprintf(logs, "%ld: %s: %s\n", t, get_lvlname(lvl), fmsg);
        fclose(logs);
    }
    return 0;
}

//FILE* log_fopen(const char *path, const char *mode){
//    if(libc_fopen)
//        return ((funcptr_fopen)libc_fopen)(path, mode);
//    return NULL;
//}

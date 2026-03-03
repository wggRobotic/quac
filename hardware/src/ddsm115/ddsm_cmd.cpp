#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ddsm115/DDSM115CMD.h>

int in_list(char* str, ...)
{
    int ret = 0;

    va_list args;
    va_start(args, str);

    for (char* s = va_arg(args, char*); s != NULL; s = va_arg(args, char*))
        if (strcmp(str, s) == 0) { ret = 1; break;}

    va_end(args);

    return ret;
}

int main(int argc, char *argv[])
{
    if (argc != 2)  { printf("Usage: ros2 run quac ddsm_cmd <cmd> [<options>]\n"); return 0; }

    if (in_list(argv[1], "-h", "--help", NULL))
    {
        printf(
            "Usage: ros2 run quac ddsm_cmd\n"
            "  setid <id: int>\n"
            "  vc <id: int> <velocity: int>\n"
            "  info <id: int>\n"
            "\n"
        );
        return 0;
    }


}
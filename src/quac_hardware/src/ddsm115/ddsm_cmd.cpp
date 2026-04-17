#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ddsm115/DDSM115CMD.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    if (argc < 2)  { printf("Usage: ros2 run quac ddsm_cmd <cmd> [<options>]\n"); return 0; }

    DDSM115CMD cmd;
    if (cmd.connect("/dev/ttyTHS1") == false) {printf("error connecting\n"); return 1;}
    

    if (strcmp(argv[1], "cmd") ==0)
    {
        uint8_t id, mode, err;
        double cur, vel, pos;
        cmd.drive(4, 0, 1, 0);
        sleep(1);
        cmd.drive_feedback(&id, &mode, &cur, &vel, &pos, &err);
        printf("cur %f vel %f pos %f\n", cur, vel ,pos);
    }

    cmd.disconnect();
}
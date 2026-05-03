#include <cstddef>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ddsm115/DDSM115CMD.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2)  { printf("Usage: ros2 run quac ddsm_cmd <cmd> [<options>]\n"); return 0; }

    DDSM115CMD cmd;
    if (cmd.connect("/dev/ttyTCU0") == false) {printf("error connecting\n"); return 1;}
    

    if (strcmp(argv[1], "drive") == 0)
    {
        if (argc < 4) printf("Usage: ros2 run quac ddsm_cmd drive <id> <rad/s>\n");
        else
        {
            uint8_t id, mode, err;
            double cur, vel, pos;
            cmd.drive(std::stoi(argv[2]), std::stof(argv[3]), 1, 0);
            usleep(10000);
            uint8_t buffer[20];
            int ret = cmd.read_bytes(buffer,12);
            for (int i = 0; i < ret; i++) printf("0x%x ", buffer[i]);
            printf("\n");
            
            //cmd.drive_feedback(&id, &mode, &cur, &vel, &pos, &err);
            //printf("cur %f vel %f pos %f\n", cur, vel ,pos);
        }
    }

    else if (strcmp(argv[1], "set_mode") == 0)
    {
        if (argc < 4) printf("Usage: ros2 run quac ddsm_cmd set_mode <id> <mode ( cur | vel | pos )>\n");
        else
        {
            int mode = 0;
            if (strcmp(argv[3], "cur") == 0) mode = DRIVE_MODE_CURRENT;
            if (strcmp(argv[3], "vel") == 0) mode = DRIVE_MODE_VELOCITY;
            if (strcmp(argv[3], "pos") == 0) mode = DRIVE_MODE_POSITION;
            if (mode != 0) cmd.set_mode(std::stoi(argv[2]), mode);
            else
            {
                printf("Invalid argument '%s'\n", argv[3]);
                printf("Usage: ros2 run quac ddsm_cmd set_mode <id> <mode ( cur | vel | pos )>\n");
            }
        }
    }
    else
    {
        printf("Usage: ros2 run quac ddsm_cmd <cmd> [<options>]\n");
    }

    cmd.disconnect();
}
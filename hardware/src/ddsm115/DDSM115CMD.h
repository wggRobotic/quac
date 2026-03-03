#include <string>
#include <stdbool.h>
#include <stdint.h>

#define M_PI 3.14159265358979323846

class DDSM115CMD
{
public:
    bool connect(const std::string& port);
    void disconnect();
    const char* get_error();

    bool drive(uint8_t id, double value, uint8_t act, uint8_t brake);
    bool drive_feedback(uint8_t* id, uint8_t* mode, double* current, double* velocity, double* position, uint8_t error_code);
    bool feedback(uint8_t id, int value, int act, int brake);

private:
    bool send_msg(uint8_t* msg);
    bool rec_msg(uint8_t* msg);
    void set_error(char* str, ...);

    char m_Error[64];
    int m_SerialFD;
};
#include "diffdrive_ddsm115/diffdrive_ddsm115.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <thread>

#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

namespace quac_hardware
{

DiffDriveDDSM115::DiffDriveDDSM115() : m_Logger(rclcpp::get_logger("DiffDriveDDSM115"))
{

}

hardware_interface::CallbackReturn DiffDriveDDSM115::on_init(const hardware_interface::HardwareInfo &info)
{
  if (hardware_interface::SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS) return hardware_interface::CallbackReturn::ERROR;

  RCLCPP_INFO(m_Logger, "Hardware declaration:");

  m_Port = info.hardware_parameters.at("port");
  m_EncoderResolution = std::stoi(info.hardware_parameters.at("encoder_resolution"));
  m_Act = std::stoi(info.hardware_parameters.at("act"));
  m_UseEsp32 = (info.hardware_parameters.at("use_esp32") == "true");

  RCLCPP_INFO(m_Logger, "  port: '%s'", m_Port.c_str());
  RCLCPP_INFO(m_Logger, "  encoder_resolution: %d", m_EncoderResolution);
  RCLCPP_INFO(m_Logger, "  act: %d", m_Act);
  RCLCPP_INFO(m_Logger, "  use_esp32: %s", m_UseEsp32 ? "true" : "false");

  m_Wheels.resize(0);

  for (size_t i = 0; i < info.joints.size(); i++)
  {
    const auto &joint = info_.joints[i];

    diffdrive_wheel w;
    w.name = joint.name;
    w.command_velocity = 0.0;
    w.state_position = 0.0;
    w.state_velocity = 0.0;
    w.id = 0;
    w.scalar = 1;

    for (const auto &p : joint.parameters) if (p.first == "id") w.id = std::stoi(p.second);
    for (const auto &p : joint.parameters) if (p.first == "scalar") w.scalar = std::stoi(p.second);

    RCLCPP_INFO(m_Logger, "  Joint %s:", joint.name.c_str());
    RCLCPP_INFO(m_Logger, "    id: %d", w.id);
    RCLCPP_INFO(m_Logger, "    scalar: %d", w.scalar);
    m_Wheels.push_back(w);
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface> DiffDriveDDSM115::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  for (size_t i = 0; i < m_Wheels.size(); i++)
  {
    state_interfaces.emplace_back(hardware_interface::StateInterface(m_Wheels[i].name, hardware_interface::HW_IF_POSITION, &m_Wheels[i].state_position));
    state_interfaces.emplace_back(hardware_interface::StateInterface(m_Wheels[i].name, hardware_interface::HW_IF_VELOCITY, &m_Wheels[i].state_velocity));
  }

  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> DiffDriveDDSM115::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  for (size_t i = 0; i < m_Wheels.size(); i++)
  {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(m_Wheels[i].name, hardware_interface::HW_IF_VELOCITY, &m_Wheels[i].command_velocity));
  }
  
  return command_interfaces;
}

hardware_interface::CallbackReturn DiffDriveDDSM115::on_configure(const rclcpp_lifecycle::State & previous_state)
{
  RCLCPP_INFO(m_Logger, "Configuring ...please wait...");

  if ((m_SerialFD = open(m_Port.c_str(), O_RDWR | O_NOCTTY)) < 0) {
    RCLCPP_INFO(m_Logger, "Error opening serial port");
    return hardware_interface::CallbackReturn::ERROR;
  }

  struct termios tty;
  if (tcgetattr(m_SerialFD, &tty) != 0)
  {
    RCLCPP_INFO(m_Logger, "Error from tcgetattr");

    close(m_SerialFD);
    return hardware_interface::CallbackReturn::ERROR;
  }

  cfsetospeed(&tty, B115200);
  cfsetispeed(&tty, B115200);

  if (m_UseEsp32)
  {
    tty.c_cflag &= ~PARENB; // no parity
    tty.c_cflag &= ~CSTOPB; // one stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;     // 8 bits per byte
    tty.c_cflag &= ~CRTSCTS;// no flow control
    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ISIG;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(ICRNL | INLCR);

    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;
  }
  else
  {
    tty.c_cflag &= ~PARENB; // No parity
    tty.c_cflag &= ~CSTOPB; // 1 stop bit
    tty.c_cflag &= ~CSIZE; // Clear byte size bits
    tty.c_cflag |= CS8; // 8 bits per byte
    tty.c_cflag &= ~CRTSCTS; // Disable CTS/RTS
    tty.c_lflag = 0; // Make tty raw
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    tty.c_cc[VTIME] = 0;  // Read timeout
    tty.c_cc[VMIN] = 0;
  }

  if (tcsetattr(m_SerialFD, TCSANOW, &tty) != 0)
  {
    RCLCPP_INFO(m_Logger, "Error from tcsetattr");

    close(m_SerialFD);
    return hardware_interface::CallbackReturn::ERROR;
  }

  RCLCPP_INFO(m_Logger, "Successfully configured!");

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn DiffDriveDDSM115::on_cleanup(const rclcpp_lifecycle::State & previous_state)
{
  RCLCPP_INFO(m_Logger, "Cleaning up ...please wait...");
  
  close(m_SerialFD);

  RCLCPP_INFO(m_Logger, "Successfully cleaned up!");

  return hardware_interface::CallbackReturn::SUCCESS;
}


hardware_interface::return_type DiffDriveDDSM115::read(const rclcpp::Time & time, const rclcpp::Duration & period)
{
  double dt = period.seconds();

  for (size_t i = 0; i < m_Wheels.size(); i++)
  {
    m_Wheels[i].state_velocity = m_Wheels[i].command_velocity;
    m_Wheels[i].state_position += m_Wheels[i].state_velocity * dt;
  }

  return hardware_interface::return_type::OK;
}

uint8_t maximCrc8(uint8_t* data, const unsigned int size)
{
  uint8_t crc = 0;
  for (unsigned int i = 0; i < size; ++i)
  {
    uint8_t inbyte = data[i];
    for (unsigned char j = 0; j < 8; ++j)
    {
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix)
        crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}

hardware_interface::return_type DiffDriveDDSM115::write(const rclcpp::Time & time, const rclcpp::Duration & period)
{
  uint8_t cmd_buffer[256];
  int cmd_length;

  for (size_t i = 0; i < m_Wheels.size(); i++)
  {
    int rpm = static_cast<int>(std::round(m_Wheels[i].command_velocity * (double)m_Wheels[i].scalar / (2 * M_PI) * 60.0));

    if (m_UseEsp32)
      cmd_length = snprintf((char*)cmd_buffer, sizeof(cmd_buffer), "{\"T\": 10010,\"id\":%d,\"cmd\":%d,\"act\":%d}\n", m_Wheels[i].id, rpm, m_Act);
    else
    {
      int16_t rpm_value = (int16_t)rpm;
      // TODO: this implementation of data encoding is not endian safe
      uint8_t cmd[] = { (uint8_t) m_Wheels[i].id,
                              100,
                              (uint8_t)((rpm_value >> 8) & 0xFF),
                              (uint8_t)(rpm_value & 0xFF),
                              0x00,
                              0x00,
                              0x00,
                              0x00,
                              0x00,
                              0x00 
                            };
      cmd[9] = maximCrc8(cmd, 9);

      memcpy(cmd_buffer, cmd, sizeof(cmd));
      cmd_length = sizeof(cmd);
    }

    ::write(m_SerialFD, cmd_buffer, cmd_length);

    if (m_UseEsp32 == false)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));

      uint8_t drive_response[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
      int total_num_bytes = 0;
      int num_bytes = 0;
      for (int i = 0; i < sizeof(drive_response); i++) {
        num_bytes = ::read(m_SerialFD, &drive_response[i], 1);
        if (num_bytes <= 0) {
          break;
        }
        total_num_bytes += num_bytes;
      }
      if (1==1);
      else if (num_bytes < 0) printf("Error reading DDSM115 response for wheel id %d", m_Wheels[i].id);
      else if (total_num_bytes < 10)
      {
        // ROS_WARN("Timeout reading DDSM115 response for wheel id %d", wheel_id);
        // ROS_INFO("Received %d bytes", total_num_bytes);
        // for (int i = 0; i < 10; i++) {
        //   ROS_INFO("%02x", drive_response[i]);
        // }
      }
      else if (drive_response[0] != m_Wheels[i].id) printf("Received response for wheel %d instead of %d", drive_response[0], m_Wheels[i].id);
      if (drive_response[9] != maximCrc8(drive_response, 9)) printf("CRC error in response from wheel id %d", m_Wheels[i].id);
      else
      {
        // TODO: this implementation of data decoding is not endian safe
        int16_t drive_current = 0;
        int16_t drive_velocity = 0;
        uint16_t drive_position = 0;
        uint8_t swap;
        swap = drive_response[4];
        drive_response[4] = drive_response[5];
        drive_response[5] = swap;
        drive_current = (drive_response[2] << 8) + drive_response[3];
        drive_velocity = (drive_response[4] << 8) + drive_response[5];
        drive_position = (drive_response[6] << 8) + drive_response[7];
      }
      
    }

  }

  return hardware_interface::return_type::OK;
}

}  // namespace diffdrive_arduino

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(quac_hardware::DiffDriveDDSM115, hardware_interface::SystemInterface)
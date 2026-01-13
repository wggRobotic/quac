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
  m_Act = std::stoi(info.hardware_parameters.at("act"));
  m_Feedback = (info.hardware_parameters.at("feedback") == "true");
  m_Counter = 0;

  RCLCPP_INFO(m_Logger, "  port: '%s'", m_Port.c_str());
  RCLCPP_INFO(m_Logger, "  act: %d", m_Act);
  RCLCPP_INFO(m_Logger, "  feedback: %s", m_Feedback ? "true" : "false");

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
    w.last_position = 0;
    w.read = false;

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
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN] = 0;

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

hardware_interface::return_type DiffDriveDDSM115::read(const rclcpp::Time & time, const rclcpp::Duration & period)
{
  double dt = period.seconds();

  if (m_Counter == 0) RCLCPP_INFO(m_Logger, "Info:");

  for (size_t i = 0; i < m_Wheels.size(); i++)
  {
    double vel = m_Wheels[i].command_velocity, pos = m_Wheels[i].state_position + m_Wheels[i].state_velocity * dt;

    uint8_t drive_response[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    int total_num_bytes = 0;
    int num_bytes = 0;
    for (int j = 0; j < sizeof(drive_response); j++) {
      num_bytes = ::read(m_SerialFD, &drive_response[j], 1);
      if (num_bytes <= 0) {
        break;
      }
      total_num_bytes += num_bytes;
    }

    if (m_Feedback)
    {
      if (num_bytes < 0) RCLCPP_INFO(m_Logger, "Error reading DDSM115 response for wheel id %d", m_Wheels[i].id);

      else if (total_num_bytes < 10)
      {
        RCLCPP_INFO(m_Logger, "Timeout reading DDSM115 response for wheel id %d", m_Wheels[i].id);
        RCLCPP_INFO(m_Logger, "Received %d bytes", total_num_bytes);
        for (int j = 0; j < total_num_bytes; j++) RCLCPP_INFO(m_Logger, "%02x", drive_response[j]);
      }

      else if (drive_response[9] != maximCrc8(drive_response, 9)) RCLCPP_INFO(m_Logger, "CRC error in response from wheel %d", m_Wheels[i].id);
      else if (drive_response[0] != m_Wheels[i].id) RCLCPP_INFO(m_Logger, "Received response for wheel %d instead of %d", drive_response[0], m_Wheels[i].id);
      else
      {
        int16_t drive_current = (drive_response[2] << 8) + drive_response[3];
        int16_t drive_velocity = (drive_response[4] << 8) + drive_response[5];
        uint16_t drive_position = (drive_response[6] << 8) + drive_response[7];

        if (m_Wheels[i].read == false)
        {
          m_Wheels[i].last_position = drive_position;
          m_Wheels[i].read = true;
        }

        vel = (double)(m_Wheels[i].scalar * (int)drive_velocity) / 60.0 * 2.0 * M_PI;

        double delta = ((double)drive_position - (double)m_Wheels[i].last_position) / 32768.0;
        m_Wheels[i].last_position = drive_position;

        if (delta > 0.5) delta -= 1.0;
        else if (delta < -0.5) delta += 1.0;

        pos = m_Wheels[i].state_position - (double)m_Wheels[i].scalar * delta * 2.0 * M_PI;

        if (m_Counter == 0) RCLCPP_INFO(m_Logger, "  %-23s:  - vel %3d rpm   - pos %.4f rad   - curr %4.1f mA", m_Wheels[i].name.c_str(), drive_velocity, pos, (double)drive_current / 32768.0*8000.0);
      }
    }

    m_Wheels[i].state_velocity = vel;
    m_Wheels[i].state_position = pos;
  }

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type DiffDriveDDSM115::write(const rclcpp::Time & time, const rclcpp::Duration & period)
{
  for (size_t i = 0; i < m_Wheels.size(); i++)
  {
    int16_t rpm = static_cast<int16_t>(std::round(m_Wheels[i].command_velocity * (double)m_Wheels[i].scalar / (2.0 * M_PI) * 60.0));

    uint8_t cmd[] = 
    {
      (uint8_t) m_Wheels[i].id,
      100,
      (uint8_t)((rpm >> 8) & 0xFF),
      (uint8_t)(rpm & 0xFF),
      0x00,
      0x00,
      m_Act,
      0x00,
      0x00,
      0x00 
    };

    cmd[9] = maximCrc8(cmd, 9);

    ::write(m_SerialFD, cmd, sizeof(cmd));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    tcdrain(m_SerialFD);
  }

  m_Counter = (m_Counter+1)%5;
  return hardware_interface::return_type::OK;
}

}

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(quac_hardware::DiffDriveDDSM115, hardware_interface::SystemInterface)
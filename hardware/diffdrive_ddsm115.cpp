#include "diffdrive_ddsm115/diffdrive_ddsm115.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

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

  RCLCPP_INFO(m_Logger, "  port: '%s'", m_Port.c_str());
  RCLCPP_INFO(m_Logger, "  encoder_resolution: %d", m_EncoderResolution);
  RCLCPP_INFO(m_Logger, "  act: %d", m_Act);

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
    w.invert = 1;

    for (const auto &p : joint.parameters) if (p.first == "id") w.id = std::stoi(p.second);
    for (const auto &p : joint.parameters) if (p.first == "invert") w.invert = std::stoi(p.second);

    RCLCPP_INFO(m_Logger, "  Joint %s:", joint.name.c_str());
    RCLCPP_INFO(m_Logger, "    id: %d", w.id);
    RCLCPP_INFO(m_Logger, "    invert: %d", w.invert);
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

hardware_interface::return_type DiffDriveDDSM115::write(const rclcpp::Time & time, const rclcpp::Duration & period)
{
  for (size_t i = 0; i < m_Wheels.size(); i++)
  {
    std::stringstream cmd_stream;
    cmd_stream << "{\"T\": 10010,\"id\":" << m_Wheels[i].id << ",\"cmd\":" << static_cast<int>(std::round(m_Wheels[i].command_velocity * (double)m_Wheels[i].invert / (2 * M_PI) * 60.0)) << ",\"act\":" << m_Act << "}\n";

    std::string cmd = cmd_stream.str();
    ::write(m_SerialFD, cmd.c_str(), cmd.size());
  }

  return hardware_interface::return_type::OK;
}

}  // namespace diffdrive_arduino

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(quac_hardware::DiffDriveDDSM115, hardware_interface::SystemInterface)
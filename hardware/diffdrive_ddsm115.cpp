#include "diffdrive_ddsm115/diffdrive_ddsm115.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <memory>
#include <vector>

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
    w.encoder_resolution = 1;

    for (const auto &p : joint.parameters) if (p.first == "id") w.id = std::stoi(p.second);

    RCLCPP_INFO(m_Logger, "Joint %s: 'id':%d", joint.name.c_str(), w.id, w.encoder_resolution);
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

  /*Implement actual hardware stuff*/

  RCLCPP_INFO(m_Logger, "Successfully configured!");

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn DiffDriveDDSM115::on_cleanup(const rclcpp_lifecycle::State & previous_state)
{
  RCLCPP_INFO(m_Logger, "Cleaning up ...please wait...");
  
  /*Implement actual hardware stuff*/

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

hardware_interface::return_type DiffDriveDDSM115::write(const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  return hardware_interface::return_type::OK;
}

}  // namespace diffdrive_arduino

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(quac_hardware::DiffDriveDDSM115, hardware_interface::SystemInterface)
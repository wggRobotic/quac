#include "quac_hardware/ddsm_hardware.hpp"
#include "hardware_interface/handle.hpp"
#include "rclcpp/rclcpp.hpp"

namespace quac_hardware
{

hardware_interface::return_type DDSMHardware::configure(
  const hardware_interface::HardwareInfo & info)
{
  if (SystemInterface::configure(info) != hardware_interface::return_type::OK)
  {
    return hardware_interface::return_type::ERROR;
  }

  // Resize vectors to number of joints
  const size_t num_joints = info.joints.size();
  positions_.assign(num_joints, 0.0);
  velocities_.assign(num_joints, 0.0);
  efforts_.assign(num_joints, 0.0);
  commands_.assign(num_joints, 0.0);
  joint_names_.clear();

  for (const auto & joint : info.joints)
  {
    joint_names_.push_back(joint.name);
    RCLCPP_INFO(rclcpp::get_logger("DDSMHardware"),
                "Found joint %s", joint.name.c_str());
  }

  // Init your hardware interface
  if (!init_hardware_interface())
  {
    RCLCPP_ERROR(rclcpp::get_logger("DDSMHardware"),
                 "Failed to initialize hardware interface");
    return hardware_interface::return_type::ERROR;
  }

  return hardware_interface::return_type::OK;
}

std::vector<hardware_interface::StateInterface> DDSMHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
  for (size_t i = 0; i < joint_names_.size(); ++i)
  {
    state_interfaces.emplace_back(
      hardware_interface::StateInterface(joint_names_[i], "position", &positions_[i]));
    state_interfaces.emplace_back(
      hardware_interface::StateInterface(joint_names_[i], "velocity", &velocities_[i]));
    state_interfaces.emplace_back(
      hardware_interface::StateInterface(joint_names_[i], "effort", &efforts_[i]));
  }
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface> DDSMHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  for (size_t i = 0; i < joint_names_.size(); ++i)
  {
    command_interfaces.emplace_back(
      hardware_interface::CommandInterface(joint_names_[i], "velocity", &commands_[i]));
  }
  return command_interfaces;
}

hardware_interface::return_type DDSMHardware::start()
{
  RCLCPP_INFO(rclcpp::get_logger("DDSMHardware"), "Starting hardware interface");
  // e.g., enable motors
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type DDSMHardware::stop()
{
  RCLCPP_INFO(rclcpp::get_logger("DDSMHardware"), "Stopping hardware interface");
  // e.g., disable motors
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type DDSMHardware::read()
{
  // Read each motor/joint
  for (size_t i = 0; i < joint_names_.size(); ++i)
  {
    double pos = 0.0, vel = 0.0, eff = 0.0;
    if (!read_motor_state(joint_names_[i], pos, vel, eff))
    {
      RCLCPP_ERROR(rclcpp::get_logger("DDSMHardware"),
                   "Failed to read state for joint %s", joint_names_[i].c_str());
      return hardware_interface::return_type::ERROR;
    }
    positions_[i] = pos;
    velocities_[i] = vel;
    efforts_[i] = eff;
  }
  return hardware_interface::return_type::OK;
}

hardware_interface::return_type DDSMHardware::write()
{
  // Write velocity commands
  for (size_t i = 0; i < joint_names_.size(); ++i)
  {
    if (!set_motor_velocity(joint_names_[i], commands_[i]))
    {
      RCLCPP_ERROR(rclcpp::get_logger("DDSMHardware"),
                   "Failed to set velocity for joint %s", joint_names_[i].c_str());
      return hardware_interface::return_type::ERROR;
    }
  }
  return hardware_interface::return_type::OK;
}

// Placeholder implementations – you must implement for your hardware:
bool DDSMHardware::init_hardware_interface()
{
  // e.g., open serial port, initialize RS485 driver, configure DDSM115
  return true;
}
bool DDSMHardware::set_motor_velocity(const std::string & joint_name, double velocity)
{
  // Map the velocity (rad/s) to motor command (RPM or value) as per DDSM115 interface
  // For example convert rad/s to RPM: RPM = velocity_rad_s * (60/(2*pi))
  // Then send via RS485: set velocity command to motor driver.
  return true;
}
bool DDSMHardware::read_motor_state(const std::string & joint_name,
                                      double & position,
                                      double & velocity,
                                      double & effort)
{
  // Query driver for the joint_name, get encoder counts,
  // convert to rad, rad/s, current, etc.
  position = 0.0;
  velocity = 0.0;
  effort   = 0.0;
  return true;
}

} // namespace my_robot_hardware

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(
  quac_hardware::DDSMHardware,
  hardware_interface::SystemInterface
)

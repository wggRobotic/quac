#pragma once

#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include <vector>
#include <string>

namespace quac_hardware
{

class DDSMHardware : public hardware_interface::SystemInterface
{
public:
  hardware_interface::return_type configure(
    const hardware_interface::HardwareInfo & info) override;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::return_type start() override;
  hardware_interface::return_type stop() override;

  hardware_interface::return_type read() override;
  hardware_interface::return_type write() override;

private:
  // Storage for each joint/motor state and command
  std::vector<double> positions_;      // rad or m (depending on joint)
  std::vector<double> velocities_;     // rad/s or m/s
  std::vector<double> efforts_;        // e.g., current (optional)
  std::vector<double> commands_;       // desired velocity command

  // Joint names (populated from info)
  std::vector<std::string> joint_names_;

  // Hardware/driver interface specifics
  // E.g., a RS485 interface, or serial, or CAN. Here placeholders:
  bool init_hardware_interface();      // set up connection to motor controllers
  bool set_motor_velocity(const std::string & joint_name, double velocity);
  bool read_motor_state(const std::string & joint_name,
                        double & position,
                        double & velocity,
                        double & effort);

  // Limits, maybe stored from info
};

} // namespace my_robot_hardware

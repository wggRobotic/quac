#pragma once

#ifndef DIFFDRIVE_DDSM115_H
#define DIFFDRIVE_DDSM115_H

#include <cstring>
#include "rclcpp/rclcpp.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "stdbool.h"

#define M_PI 3.14159265358979323846

namespace quac_hardware
{

struct diffdrive_wheel
{
  std::string name;
  double command_velocity;
  double state_position;
  double state_velocity;
  int id;
  int scalar;
};

class DiffDriveDDSM115 : public hardware_interface::SystemInterface
{

public:

  //RCLCPP_SHARED_PTR_DEFINITIONS(DiffDriveDDSM115)

  DiffDriveDDSM115();

  hardware_interface::CallbackReturn on_init(const hardware_interface::HardwareInfo & info) override;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::CallbackReturn on_cleanup(const rclcpp_lifecycle::State & previous_state) override;

  hardware_interface::return_type read(const rclcpp::Time & time, const rclcpp::Duration & period) override;

  hardware_interface::return_type write(const rclcpp::Time & time, const rclcpp::Duration & period) override;

private:

  rclcpp::Logger m_Logger;
  std::chrono::time_point<std::chrono::system_clock> m_Time;
  std::vector<diffdrive_wheel> m_Wheels;

  int m_SerialFD;
  std::string m_Port;
  int m_EncoderResolution;
  int m_Act;
  bool m_UseEsp32;
};

}

#endif
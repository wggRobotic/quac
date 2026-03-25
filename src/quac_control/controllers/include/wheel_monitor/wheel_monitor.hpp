#pragma once

#ifndef WHEEL_MONITOR__WHEEL_MONITOR_HPP_
#define WHEEL_MONITOR__WHEEL_MONITOR_HPP_

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "std_msgs/msg/float64_multi_array.hpp"

#include "controller_interface/controller_interface.hpp"
#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/subscription.hpp"
#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/state.hpp"

typedef struct wheel_interface
{
  int id;
  double value;
} wheel_interface;

typedef struct wheel
{
  std::string name;
  wheel_interface pos;
  wheel_interface vel;
  wheel_interface cur;
} wheel;

namespace quac_controllers
{

class WheelMonitor : public controller_interface::ControllerInterface
{
public:
  WheelMonitor();

  controller_interface::InterfaceConfiguration state_interface_configuration() const override;

  controller_interface::InterfaceConfiguration command_interface_configuration() const override;

  controller_interface::return_type update(const rclcpp::Time& time, const rclcpp::Duration& period) override;

  controller_interface::CallbackReturn on_init() override;

  controller_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State& previous_state) override;

  controller_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State& previous_state) override;

private:
  int get_joint_interface_id(const std::string& joint_name, const std::string& interface_name);

  std::vector<wheel> m_Wheels;
  std::shared_ptr<rclcpp::Publisher<std_msgs::msg::Float64MultiArray>> m_InfoPublisher;
  std_msgs::msg::Float64MultiArray m_InfoMessage;
};

}  // namespace quac_controllers

#endif  // WHEEL_MONITOR__WHEEL_MONITOR_HPP_
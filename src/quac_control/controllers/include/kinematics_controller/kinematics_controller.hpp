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
#include "rclcpp/time.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include <geometry_msgs/msg/pose_stamped.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64.hpp>

namespace quac_controllers
{

class KinematicsController : public controller_interface::ControllerInterface
{
public:
  KinematicsController();

  controller_interface::InterfaceConfiguration state_interface_configuration() const override;

  controller_interface::InterfaceConfiguration command_interface_configuration() const override;

  controller_interface::return_type update(const rclcpp::Time& time, const rclcpp::Duration& period) override;

  controller_interface::CallbackReturn on_init() override;

  controller_interface::CallbackReturn on_configure(const rclcpp_lifecycle::State& previous_state) override;

  controller_interface::CallbackReturn on_activate(const rclcpp_lifecycle::State& previous_state) override;

private:
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr ee_pos_sub_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr gripper_sub_;
};

}  // namespace quac_controllers

#endif  // WHEEL_MONITOR__WHEEL_MONITOR_HPP_
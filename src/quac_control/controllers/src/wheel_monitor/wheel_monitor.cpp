#include "wheel_monitor/wheel_monitor.hpp"
#include <pluginlib/class_list_macros.hpp>

namespace quac_controllers
{

WheelMonitor::WheelMonitor() {}

controller_interface::InterfaceConfiguration WheelMonitor::state_interface_configuration() const
{
  controller_interface::InterfaceConfiguration conf;
  conf.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  conf.names.reserve(m_Wheels.size()*3);
  
  for (int i = 0; i < m_Wheels.size(); i++)
  {
    conf.names.push_back(m_Wheels[i].name + "/position");
    conf.names.push_back(m_Wheels[i].name + "/velocity");
    conf.names.push_back(m_Wheels[i].name + "/current");
  }
  return conf;
}

controller_interface::InterfaceConfiguration WheelMonitor::command_interface_configuration() const
{
  return {controller_interface::interface_configuration_type::NONE};
}

controller_interface::CallbackReturn WheelMonitor::on_init()
{
  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn WheelMonitor::on_configure(const rclcpp_lifecycle::State & previous_state)
{
  //get_node()->declare_parameter("wheel_names", std::vector<std::string>{"wheel_joint"});
  auto wheel_names = get_node()->get_parameter("wheel_names").as_string_array();
  m_Wheels.resize(wheel_names.size());
  for (int i = 0; i < wheel_names.size(); i++) m_Wheels[i].name = wheel_names[i];
  m_InfoMessage.data.resize(wheel_names.size()*3);

  m_InfoPublisher = get_node()->create_publisher<std_msgs::msg::Float64MultiArray>("wheel_info", 10);

  return controller_interface::CallbackReturn::SUCCESS;
}

int WheelMonitor::get_joint_interface_id(const std::string& joint_name, const std::string& interface_name)
{
  for (int i = 0; i < state_interfaces_.size(); i++) if (
    state_interfaces_[i].get_prefix_name() == joint_name &&
    state_interfaces_[i].get_interface_name() == interface_name
  ) return i;

  return -1;
}

controller_interface::CallbackReturn WheelMonitor::on_activate(const rclcpp_lifecycle::State & previous_state)
{
  for (int i = 0; i < m_Wheels.size(); i++)
  {
    if ((m_Wheels[i].pos.id = get_joint_interface_id(m_Wheels[i].name, "position")) == -1)
    {
      RCLCPP_ERROR(get_node()->get_logger(), "position interface for wheel %d not found", i);
      return controller_interface::CallbackReturn::ERROR;
    }

    if ((m_Wheels[i].vel.id = get_joint_interface_id(m_Wheels[i].name, "velocity")) == -1)
    {
      RCLCPP_ERROR(get_node()->get_logger(), "velocity interface for wheel %d not found", i);
      return controller_interface::CallbackReturn::ERROR;
    }

    if ((m_Wheels[i].cur.id = get_joint_interface_id(m_Wheels[i].name, "current")) == -1)
    {
      RCLCPP_ERROR(get_node()->get_logger(), "current interface for wheel %d not found", i);
      return controller_interface::CallbackReturn::ERROR;
    }
  }

  return controller_interface::CallbackReturn::SUCCESS;
}

controller_interface::return_type WheelMonitor::update(const rclcpp::Time & time, const rclcpp::Duration & period)
{
  for (int i = 0; i < m_Wheels.size(); i++)
  {
    m_InfoMessage.data[3*i+0] = state_interfaces_[m_Wheels[i].pos.id].get_value();
    m_InfoMessage.data[3*i+1] = state_interfaces_[m_Wheels[i].vel.id].get_value();
    m_InfoMessage.data[3*i+2] = state_interfaces_[m_Wheels[i].cur.id].get_value();
  }

  m_InfoPublisher->publish(m_InfoMessage);

  return controller_interface::return_type::OK;
}

}

PLUGINLIB_EXPORT_CLASS(quac_controllers::WheelMonitor, controller_interface::ControllerInterface)
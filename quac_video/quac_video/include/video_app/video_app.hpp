#include <rclcpp/rclcpp.hpp>
#include "ament_index_cpp/get_package_share_directory.hpp"
#include <librealsense2/rs.hpp>

#include "gst_app.hpp"

class VideoApp : public rclcpp::Node
{
public:
  VideoApp();

  void run();

  rs2::config cfg;
  rs2::pipeline pipeline;
};
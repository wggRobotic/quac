#include <rclcpp/rclcpp.hpp>
#include "ament_index_cpp/get_package_share_directory.hpp"
#include <librealsense2/rs.hpp>

#include "gst_app.hpp"

enum cam_pos
{
  CAM_FRONT,
  CAM_BACK
};

class VideoApp : public rclcpp::Node
{
public:
  VideoApp();

  void run();

  std::string nvinfer_parse_folder;

  std::string ip;

  struct
  {
    int port;
    std::string serial_number;
  } cams[2];

  int width, height, fps;

  rs2::config cfg;
  rs2::pipeline pipeline;
};
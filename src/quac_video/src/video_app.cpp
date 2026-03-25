#include "video_app/video_app.hpp"
#include <memory>
#include <fstream>
#include <filesystem>
#include <rclcpp/executors.hpp>

void on_detection(void* data)
{
  VideoApp* app = (VideoApp*)data;
}

const void* on_get_frame(void* data)
{
  VideoApp* app = (VideoApp*)data;

  rs2::frameset frames = app->pipeline.wait_for_frames();
  rs2::video_frame color_frame = frames.get_color_frame();
  return color_frame.get_data();
}

VideoApp::VideoApp() : Node(
  "video_app",                        // node name
  rclcpp::NodeOptions()
    .allow_undeclared_parameters(true) // <-- this enables setting parameters that aren't declared
    .automatically_declare_parameters_from_overrides(true) // optional
)
{
  {
    std::filesystem::create_directories(".nvinfer_config");
    std::ofstream file(".nvinfer_config/hazmat_config.txt", std::ios::trunc);
    
    file << "[property]\n";
    auto property_parameters = list_parameters({"hazmat.property_parameters"}, 10);

    for (const auto & name : property_parameters.names)
      file << &(name.c_str()[sizeof("hazmat.property_parameters")]) << "=" << get_parameter(name).as_string() << "\n";    
    
    file << "\n[class-attrs-all]\n";
    auto class_parameters = list_parameters({"hazmat.class_parameters"}, 10);

    for (const auto & name : class_parameters.names)
      file << &(name.c_str()[sizeof("hazmat.class_parameters")]) << "=" << get_parameter(name).as_string() << "\n";    
    

    file.close();
  }

  {
    std::ofstream file(".nvinfer_config/hazmat_labels.txt", std::ios::trunc);

    auto labels = get_parameter("hazmat.labels").as_string_array();
    for (int i = 0; i < labels.size(); i++) file << labels[i] << "\n";

    file.close();
  }

  
}

void VideoApp::run()
{
  int width = 640, height = 480, fps = 30;

  cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_RGBA8, fps);
  pipeline.start(cfg);

  gst_app_interface interface;
  interface.on_detection = on_detection;
  interface.on_get_frame = on_get_frame;
  interface.user_data = this;

  std::string pkg_share_dir = ament_index_cpp::get_package_share_directory("quac_video");
  std::string config_path = pkg_share_dir + "/config/hazmat_config.txt";

  gst_app_run(width, height, fps, ".nvinfer_config/hazmat_config.txt", false, "192.168.137.26", &interface);

  pipeline.stop();
}

int main (int argc, char *argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<VideoApp>();
  node->run();
  //rclcpp::spin(node);
  rclcpp::shutdown();
}


  
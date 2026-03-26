#include "video_app/video_app.hpp"
#include <memory>
#include <fstream>
#include <filesystem>
#include <cstdlib>
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
  nvinfer_parse_folder = get_parameter("nvinfer_parse_folder").as_string();

  ip = get_parameter("ip").as_string();

  cams[CAM_FRONT].port = get_parameter("front.port").as_int();
  cams[CAM_FRONT].serial_number = get_parameter("front.serial_number").as_string();

  cams[CAM_BACK].port = get_parameter("back.port").as_int();
  cams[CAM_BACK].serial_number = get_parameter("back.serial_number").as_string();

  width = get_parameter("width").as_int();
  height = get_parameter("height").as_int();
  fps = get_parameter("fps").as_int();

  {
    std::filesystem::create_directories(nvinfer_parse_folder);
    std::ofstream file(nvinfer_parse_folder + "hazmat_config.txt", std::ios::trunc);
    
    file << "[property]\n";

    for (const auto & name : list_parameters({"hazmat.property_parameters"}, 10).names)
      file << &(name.c_str()[sizeof("hazmat.property_parameters")]) << "=" << get_parameter(name).as_string() << "\n";    
    
    file << "\n[class-attrs-all]\n";

    for (const auto & name : list_parameters({"hazmat.class_parameters"}, 10).names)
      file << &(name.c_str()[sizeof("hazmat.class_parameters")]) << "=" << get_parameter(name).as_string() << "\n";    
    

    file.close();
  }

  {
    std::ofstream file(nvinfer_parse_folder + get_parameter("hazmat.property_parameters.labelfile-path").as_string(), std::ios::trunc);

    for (const auto & label : get_parameter("hazmat.labels").as_string_array()) file << label << "\n";

    file.close();
  }

  {
    std::string engine_path = nvinfer_parse_folder + get_parameter("hazmat.property_parameters.model-engine-file").as_string();
    if (std::filesystem::exists(engine_path) == false)
    {
      std::string engine_cmd = "/usr/src/tensorrt/bin/trtexec "
        "--onnx=" + get_parameter("hazmat.model").as_string() + " "
        "--saveEngine=" + engine_path + " "
        "--verbose "
        "--skipInference";

      RCLCPP_INFO(get_logger(), "%s doesn't exist. Generating ...", engine_path);
      RCLCPP_INFO(get_logger(), engine_cmd.c_str());
      system(engine_cmd.c_str());
    }
  }

  
}

void VideoApp::run()
{
  cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_RGBA8, fps);
  pipeline.start(cfg);

  gst_app_interface interface;
  interface.on_detection = on_detection;
  interface.on_get_frame = on_get_frame;
  interface.user_data = this;

  std::string pkg_share_dir = ament_index_cpp::get_package_share_directory("quac_video");
  std::string config_path = pkg_share_dir + "/config/hazmat_config.txt";

  gst_app_run(width, height, fps, nvinfer_parse_folder + "hazmat_config.txt", false, ip, &interface);

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


  
#include "camera_app/camera_app.hpp"

CameraApp::CameraApp() : Node("camera_app")
{
  declare_parameter<int>("port", 5000);
  port = get_parameter("port").as_int();

  declare_parameter<std::string>("serial_number", "12345");
  serial_number = get_parameter("serial_number").as_string();

  declare_parameter<int>("width", 640);
  width = get_parameter("width").as_int();

  declare_parameter<int>("height", 480);
  height = get_parameter("height").as_int();

  declare_parameter<int>("fps", 30);
  fps = get_parameter("fps").as_int();

  declare_parameter<std::vector<std::string>>("hazmat.labels", {"label0"});
  for (const auto & label : get_parameter("hazmat.labels").as_string_array()) hazmat_labels.push_back(label);

  ip_set = false;
}

int CameraApp::init(image_transport::ImageTransport &it)
{
  ip_subscriber = create_subscription<std_msgs::msg::String>("target_ip", 10, std::bind(&CameraApp::ip_callback, this, std::placeholders::_1));

  while (true)
  {
    rclcpp::spin_some(shared_from_this());

    if (!rclcpp::ok()) return -1;
    if (ip_set) break;
    RCLCPP_INFO(get_logger(), "waiting for ip ...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  ip_subscriber.reset();

  return 0;
}

void CameraApp::ip_callback(const std_msgs::msg::String::SharedPtr msg)
{
  ip = msg->data;
  ip_set = true;
}

void CameraApp::send_gst_frame(const void* data)
{
  int frame_size = 3 * width * height;

  GstBuffer *buffer = gst_buffer_new_allocate(nullptr, frame_size, nullptr);
  if (!buffer) {
    RCLCPP_INFO(get_logger(), "gst_buffer_new_allocate failed");
    return;
  }

  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {
      RCLCPP_INFO(get_logger(), "gst_buffer_map failed");
      gst_buffer_unref(buffer);
      return;
  }
  memcpy(map.data, data, frame_size);

  gst_buffer_unmap(buffer, &map);

  GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(gst_appsrc), buffer);
  if (ret != GST_FLOW_OK)
  {
    RCLCPP_INFO(get_logger(), "gst_app_src_push_buffer failed");
    gst_buffer_unref(buffer);
  }

}

void CameraApp::run()
{
  gst_init(NULL, NULL);

  std::string gst_pipeline_desc = 
    "appsrc name=appsrc format=time "
    "caps=video/x-raw,format=RGB,width=" + std::to_string(width) + 
    ",height=" + std::to_string(height) + ",framerate=" + std::to_string(fps) + "/1 "
    "! videoconvert ! x264enc speed-preset=ultrafast tune=zerolatency "
    "! rtph264pay config-interval=1 ! udpsink host=" + ip + " port=" + std::to_string(port) + " sync=false";


  GError *error = nullptr;
  gst_pipeline = gst_parse_launch(gst_pipeline_desc.c_str(), &error);

  if (!gst_pipeline || error) {
    RCLCPP_INFO(get_logger(), "Failed to create Gstreamer pipeline");
    return;
  }

  gst_appsrc = gst_bin_get_by_name(GST_BIN(gst_pipeline), "appsrc");
  gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);

  rs_cfg.enable_device(serial_number);
  rs_cfg.enable_stream(RS2_STREAM_COLOR, width, height, RS2_FORMAT_BGR8, fps);
  rs_cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, fps);

  rs_pipeline.start(rs_cfg);
  RCLCPP_INFO(get_logger(), "Streaming camera on %s:%d", ip.c_str(), port);

  while (rclcpp::ok())
  {
    rs2::frameset frames = rs_pipeline.wait_for_frames();
    rs2::video_frame color_frame = frames.get_color_frame();
    rs2::depth_frame depth_frame = frames.get_depth_frame();

    send_gst_frame(color_frame.get_data());
  }
 
  rs_pipeline.stop();
  RCLCPP_INFO(get_logger(), "Closed camera");

  gst_element_set_state(gst_pipeline, GST_STATE_NULL);
  gst_object_unref(gst_pipeline);
}

int main (int argc, char *argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<CameraApp>();

  image_transport::ImageTransport it(node);
  node->init(it);
  node->run();
  rclcpp::shutdown();
}


  
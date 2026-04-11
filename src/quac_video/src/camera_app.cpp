#include "camera_app/camera_app.hpp"
#include "yolos/core/version.hpp"
#include <atomic>
#include <csignal>
#include <rclcpp/time.hpp>
#include <unistd.h>

std::atomic<bool> keep_running{true};

void handle_signal(int signum) {
  keep_running.store(false);
}

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

  declare_parameter<int>("bitrate", 1000);
  bitrate = get_parameter("bitrate").as_int();

  declare_parameter<int>("key_int_max", 60);
  key_int_max = get_parameter("key_int_max").as_int();

  declare_parameter<std::string>("engines_dir", "/null/");
  engines_dir = get_parameter("engines_dir").as_string();

  declare_parameter<std::string>("models_dir", "/null/");
  models_dir = get_parameter("models_dir").as_string();

  init_detection_state(&hazmat_info, "hazmat");
  init_detection_state(&paintroller_info, "paintroller");

  ip_set = false;
}

void CameraApp::init_detection_state(struct detection_state* info, const std::string& prefix)
{
  declare_parameter<bool>(prefix + ".enabled", false);
  info->enabled = get_parameter(prefix + ".enabled").as_bool();
  RCLCPP_INFO(get_logger(), "%s enabled: %s", prefix.c_str(), info->enabled ? "true" : "false");

  declare_parameter<std::string>(prefix + ".model", "none");
  info->model = get_parameter(prefix + ".model").as_string();

  info->model_path = models_dir + info->model + ".onnx";
  info->engine_path = engines_dir + info->model + ".engine";
}

int CameraApp::model_init(struct detection_state* info)
{
  if (std::filesystem::exists(info->engine_path) == false)
  {
    RCLCPP_INFO(get_logger(), "%s doesn't exist. Generating ...", info->engine_path.c_str());

    std::string engine_cmd = 
      "/usr/src/tensorrt/bin/trtexec "
      "--onnx=" + info->model_path + " "
      "--saveEngine=" + info->engine_path + " "
      "--verbose "
      "--skipInference "
      "--fp16 ";

    ::system(engine_cmd.c_str());
  }

  if (std::filesystem::exists(engines_dir + info->model + ".engine") == false) return -1;
}

bool camera_is_connected(const std::string& serial_number) {
  rs2::context ctx;
  rs2::device_list devices = ctx.query_devices();

  for (auto&& dev : devices)
    if (dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER) == serial_number)
      return true;
    
  return false;
}

int CameraApp::init(image_transport::ImageTransport &it)
{
  if (camera_is_connected(serial_number) == false)
  {
    RCLCPP_ERROR(get_logger(), "camera with serial %s not connected", serial_number.c_str());
    return -1;
  }

  if (hazmat_info.enabled || paintroller_info.enabled)
  {
    if (engines_dir == "/null/")
    {
      RCLCPP_ERROR(get_logger(), "invalid engine directory");
      return -1;
    }
    std::filesystem::create_directories(engines_dir);
  }

  if (hazmat_info.enabled) if (model_init(&hazmat_info) == -1) return -1;
  if (paintroller_info.enabled) if (model_init(&paintroller_info) == -1) return -1;

  ip_subscriber = create_subscription<std_msgs::msg::String>("video_target_ip", 10, std::bind(&CameraApp::ip_callback, this, std::placeholders::_1));

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

void CameraApp::qrcode_thread_function()
{
  qr_code_state.quirc_instance = quirc_new();
  cv::Mat grayscale;

  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr qr_code_publisher = 
    create_publisher<sensor_msgs::msg::JointState>("qr_code_detections", 10);

  while (keep_running.load())
  {
    bool frame = false;
    {
      std::lock_guard<std::mutex> lock(qr_code_state.mutex);
      frame = qr_code_state.queued;
    }
    if (frame == false)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    cv::Mat bgr_image(height, width, CV_8UC3, qr_code_state.bgr_data);
    cv::cvtColor(bgr_image, grayscale, cv::COLOR_BGR2GRAY);

    quirc_resize(qr_code_state.quirc_instance, width, height);

    uint8_t *qr_image = quirc_begin(qr_code_state.quirc_instance, &width, &height);
    memcpy(qr_image, grayscale.data, width * height);
    quirc_end(qr_code_state.quirc_instance);

    int count = quirc_count(qr_code_state.quirc_instance);
    int actual_count = 0;
    auto message = sensor_msgs::msg::JointState();
    message.header.stamp = this->now();
    message.header.frame_id = std::string(get_name());

    for (int i = 0; i < count; i++)
    {
      struct quirc_code code;
      struct quirc_data data;

      quirc_extract(qr_code_state.quirc_instance, i, &code);

      if (quirc_decode(&code, &data) == QUIRC_SUCCESS)
      {
        actual_count++;

        int mid_x = (code.corners[0].x + code.corners[1].x + code.corners[2].x + code.corners[3].x) / 4;
        int mid_y = (code.corners[0].y + code.corners[1].y + code.corners[2].y + code.corners[3].y) / 4;

        double depth;

        const uint16_t* depth_data = (const uint16_t*)qr_code_state.depth_data;

        #define AVERAGE_RANGE 6

        for (int x = -AVERAGE_RANGE/2; x < AVERAGE_RANGE/2; x++)
          for (int y = -AVERAGE_RANGE/2; y < AVERAGE_RANGE/2; y++)
            depth += (double)depth_data[mid_x + x + (mid_y + y) * width];

        depth = depth / (double)(AVERAGE_RANGE * AVERAGE_RANGE) * qr_code_state.depth_units;

        message.name.push_back(std::string((char*)data.payload));
        message.position.push_back(depth);
      }
    }

    if (actual_count > 0) qr_code_publisher->publish(message);

    {
      std::lock_guard<std::mutex> lock(qr_code_state.mutex);
      qr_code_state.queued = false;
    }
  }

  qr_code_publisher.reset();
  quirc_destroy(qr_code_state.quirc_instance);
}

void CameraApp::run()
{
  gst_init(NULL, NULL);

  yolos::det::YOLODetector* hazmat_detector;
  yolos::det::YOLODetector* paintroller_detector;

  if (hazmat_info.enabled)
  {
    hazmat_detector = new yolos::det::YOLODetector(
      engines_dir + hazmat_info.model + ".engine", 
      models_dir  + hazmat_info.model + ".labels.txt"
    );
  }
  if (paintroller_info.enabled)
  {
    paintroller_detector = new yolos::det::YOLODetector(
      engines_dir + paintroller_info.model + ".engine", 
      models_dir  + paintroller_info.model + ".labels.txt"
    );
  }

  qr_code_state.queued = false;
  qr_code_state.bgr_data = malloc(width * height * 3);
  qr_code_state.depth_data = malloc(width * height * 2);
  std::thread qr_code_thread(&CameraApp::qrcode_thread_function, this);

  std::string gst_pipeline_desc = 
    "appsrc name=appsrc format=time "
    "caps=video/x-raw,format=BGR,width=" + std::to_string(width) + 
    ",height=" + std::to_string(height) + ",framerate=" + std::to_string(fps) + "/1 "
    "! videoconvert ! x264enc speed-preset=ultrafast tune=zerolatency bitrate=" + std::to_string(bitrate) + " key-int-max=" + std::to_string(key_int_max) + " "
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

  signal(SIGTERM, handle_signal);
  signal(SIGINT, handle_signal);

  while (keep_running.load())
  {
    rs2::frameset frames = rs_pipeline.wait_for_frames();
    rclcpp::Time time = now();
    rs2::video_frame color_frame = frames.get_color_frame();
    rs2::depth_frame depth_frame = frames.get_depth_frame();

    cv::Mat image(height, width, CV_8UC3, (void*)color_frame.get_data());

    std::vector<yolos::det::Detection> hazmat_results, paintroller_results;

    if (hazmat_info.enabled) hazmat_results = hazmat_detector->detect(image);
    if (paintroller_info.enabled) paintroller_results = paintroller_detector->detect(image);

    GstBuffer *buffer = gst_buffer_new_allocate(nullptr, 3 * width * height, nullptr);
    GstMapInfo map;
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, image.data, 3 * width * height);

    cv::Mat mapped_image(height, width, CV_8UC3, (void*)map.data);
    if (hazmat_info.enabled) hazmat_detector->drawDetections(mapped_image, hazmat_results);
    if (paintroller_info.enabled) paintroller_detector->drawDetections(mapped_image, paintroller_results);

    gst_buffer_unmap(buffer, &map);
    gst_app_src_push_buffer(GST_APP_SRC(gst_appsrc), buffer);

    {
      std::lock_guard<std::mutex> lock(qr_code_state.mutex);

      if (qr_code_state.queued == false)
      {
        memcpy(qr_code_state.bgr_data, color_frame.get_data(), 3 * width * height);
        memcpy(qr_code_state.depth_data, depth_frame.get_data(), 2 * width * height);
        qr_code_state.depth_units = depth_frame.get_units();
        qr_code_state.queued = true;
      }
    }
  }
 
  rs_pipeline.stop();
  RCLCPP_INFO(get_logger(), "Closed camera");

  gst_element_set_state(gst_pipeline, GST_STATE_NULL);
  gst_object_unref(gst_pipeline);

  qr_code_thread.join();
  free(qr_code_state.bgr_data);
  free(qr_code_state.depth_data);

  if (hazmat_info.enabled) delete hazmat_detector;
  if (paintroller_info.enabled) delete paintroller_detector;
}

int main (int argc, char *argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<CameraApp>();

  image_transport::ImageTransport it(node);
  if (node->init(it) == 0) node->run();
  
  rclcpp::shutdown();
}


  
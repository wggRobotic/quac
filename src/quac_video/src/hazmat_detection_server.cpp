#include "hazmat_detection_server/hazmat_detection_server.hpp"
#include <mutex>

HazmatDetectionServer::HazmatDetectionServer() : Node("hazmat_detection_server"), tf_buffer(this->get_clock()), tf_listener(tf_buffer)
{
  declare_parameter<std::vector<std::string>>("topics", std::vector<std::string>{"cam/bgrd"});
  topics = get_parameter("topics").as_string_array();

  declare_parameter<std::vector<std::string>>("camera_frames", std::vector<std::string>{"cam"});
  camera_frames = get_parameter("camera_frames").as_string_array();

  declare_parameter<std::string>("model_path", "model");
  model_path = get_parameter("model_path").as_string();

  declare_parameter<std::string>("engine_path", "engine");
  engine_path = get_parameter("engine_path").as_string();

  declare_parameter<std::string>("labels_path", "labels");
  labels_path = get_parameter("labels_path").as_string();

  declare_parameter<std::string>("reference_frame", "map");
  mapping.reference_frame = get_parameter("reference_frame").as_string();

  declare_parameter<int>("publish_rate", 10);
  int publish_rate = get_parameter("publish_rate").as_int();

  callback_group = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  mapping.timer = create_wall_timer(std::chrono::milliseconds((int)(1000.f/(float)publish_rate)), std::bind(&HazmatDetectionServer::publish_callback, this), callback_group);

  mapping.object_publisher = create_publisher<quac_interfaces::msg::DetectedObjectArray>("hazmat_signs", 10);
}

void HazmatDetectionServer::publish_callback()
{
  return;
  std::lock_guard<std::mutex> lock(mapping.detections_mutex);

  for (int i = 0; i < mapping.detections.size(); i++)
  {
    geometry_msgs::msg::TransformStamped t;

    t.header.stamp = now();
    t.header.frame_id = mapping.reference_frame;
    t.child_frame_id = mapping.detections[i].name;

    t.transform.translation.z = mapping.detections[i].x;
    t.transform.translation.x = mapping.detections[i].y;
    t.transform.translation.y = mapping.detections[i].z;
    
    t.transform.rotation.x = 0.0;
    t.transform.rotation.y = 0.0;
    t.transform.rotation.z = 0.0;
    t.transform.rotation.w = 1.0;

    //mapping.tf_broadcaster->sendTransform(t);
  }
  
}

void HazmatDetectionServer::image_callback(quac_interfaces::msg::ImageBGRD::SharedPtr msg, int i)
{
  cv::Mat image(msg->height, msg->width, CV_8UC3, (void*)msg->bgr_data.data());
  std::vector<yolos::det::Detection> results = topic_handlers[i]->detector->detect(image);
  topic_handlers[i]->detector->drawDetections(image, results);

  quac_interfaces::msg::DetectedObjectArray object_msg;
  object_msg.header.stamp = now();
  object_msg.header.frame_id = camera_frames[i];

  std::lock_guard<std::mutex> lock(mapping.detections_mutex);

  for (int j = 0; j < results.size(); j++)
  {
    quac_interfaces::msg::DetectedObject object;
    object.name = "detection_" + std::to_string(j);
    object.type = topic_handlers[i]->detector->getClassNames()[results[j].classId];

    object.pose.orientation.x = 0.0;
    object.pose.orientation.y = 0.0;
    object.pose.orientation.z = 0.0;
    object.pose.orientation.w = 1.0;

    int u = results[j].box.x + results[j].box.width / 2;
    int v = results[j].box.y + results[j].box.height / 2;

    object.pose.position.z = ((float)msg->depth_data[v * msg->width + u]) * msg->depth_scale;
    object.pose.position.x = (u - msg->ppx) * object.pose.position.z / msg->fx;
    object.pose.position.y = (v - msg->ppy) * object.pose.position.z / msg->fy;

    object_msg.objects.push_back(object);
  }

  if (object_msg.objects.size() > 0) topic_handlers[i]->object_publisher->publish(object_msg);
}

void HazmatDetectionServer::run()
{
  if (topics.size() != camera_frames.size()) return;

  if (std::filesystem::exists(engine_path) == false)
  {
    RCLCPP_INFO(get_logger(), "%s doesn't exist. Generating ...", engine_path.c_str());

    std::string engine_cmd = 
      "/usr/src/tensorrt/bin/trtexec "
      "--onnx=" + model_path + " "
      "--saveEngine=" + engine_path + " "
      "--verbose "
      "--skipInference "
      "--fp16 ";

    ::system(engine_cmd.c_str());
  }

  if (std::filesystem::exists(engine_path) == false) return;

  rclcpp::SubscriptionOptions options;
  options.callback_group = callback_group;
  topic_handlers.resize(topics.size());
  for (int i = 0; i < topic_handlers.size(); i++)
  {
    topic_handlers[i] = std::make_unique<TopicHandler>();
    topic_handlers[i]->subscriber = create_subscription<quac_interfaces::msg::ImageBGRD>(topics[i],10,[this, i](quac_interfaces::msg::ImageBGRD::SharedPtr msg) {image_callback(msg, i);}, options);
    topic_handlers[i]->object_publisher = create_publisher<quac_interfaces::msg::DetectedObjectArray>(topics[i] + "/hazmat_signs", 10);
    topic_handlers[i]->detector = std::make_shared<yolos::det::YOLODetector>(engine_path, labels_path);
  }

  rclcpp::spin(shared_from_this());

  for (int i = 0; i < topic_handlers.size(); i++)
  {
    topic_handlers[i]->subscriber.reset();
    topic_handlers[i]->detector.reset();
  }
}

int main (int argc, char *argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<HazmatDetectionServer>();
  node->run();
  
  rclcpp::shutdown();
}
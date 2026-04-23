#include "detection_server/detection_server.hpp"

DetectionServer::DetectionServer(const std::string& name, DetectionCallback callback) : Node(name), tf_buffer(this->get_clock()), tf_listener(tf_buffer), detection_callback(callback)
{
  declare_parameter<std::vector<std::string>>("topics", std::vector<std::string>{"cam/bgrd"});
  topics = get_parameter("topics").as_string_array();

  declare_parameter<std::vector<std::string>>("camera_frames", std::vector<std::string>{"cam"});
  camera_frames = get_parameter("camera_frames").as_string_array();

  declare_parameter<std::string>("reference_frame", "map");
  mapping.reference_frame = get_parameter("reference_frame").as_string();

  declare_parameter<int>("publish_rate", 10);
  int publish_rate = get_parameter("publish_rate").as_int();

  callback_group = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  mapping.timer = create_wall_timer(std::chrono::milliseconds((int)(1000.f/(float)publish_rate)), std::bind(&HazmatDetectionServer::publish_callback, this), callback_group);

  mapping.object_publisher = create_publisher<quac_interfaces::msg::DetectedObjectArray>("hazmat_signs", 10);

  topic_handlers.resize(std::min(topics.size(), camera_frames.size()));

  rclcpp::SubscriptionOptions options;
  options.callback_group = callback_group;
  
  for (int i = 0; i < topic_handlers.size(); i++)
  {
    topic_handlers[i] = std::make_unique<TopicHandler>();
    topic_handlers[i]->subscriber = create_subscription<quac_interfaces::msg::ImageBGRD>(topics[i],10,[this, i](quac_interfaces::msg::ImageBGRD::SharedPtr msg) {image_callback(msg, i);}, options);
    topic_handlers[i]->object_publisher = create_publisher<quac_interfaces::msg::DetectedObjectArray>(topics[i] + "/hazmat_signs", 10);
  }
}

void DetectionServer::publish_callback()
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

void DetectionServer::image_callback(quac_interfaces::msg::ImageBGRD::SharedPtr msg, int i)
{
  std::vector<detection> detections;

  detection_callback(msg, i, detections);
  if (detections.size() == 0) return;

  quac_interfaces::msg::DetectedObjectArray object_msg;
  object_msg.header.stamp = now();
  object_msg.header.frame_id = camera_frames[i];

  std::lock_guard<std::mutex> lock(mapping.detections_mutex);

  for (int j = 0; j < detections.size(); j++)
  {
    quac_interfaces::msg::DetectedObject object;
    object.name = "detection_" + std::to_string(j);
    object.type = detections[j].data;

    object.pose.orientation.x = 0.0;
    object.pose.orientation.y = 0.0;
    object.pose.orientation.z = 0.0;
    object.pose.orientation.w = 1.0;

    int u = detections[j].x + detections[j].width / 2;
    int v = detections[j].y + detections[j].height / 2;

    object.pose.position.x = ((float)msg->depth_data[v * msg->width + u]) * msg->depth_scale;
    object.pose.position.y = (u - msg->ppx) * object.pose.position.x / msg->fx;
    object.pose.position.z = (v - msg->ppy) * object.pose.position.x / msg->fy;

    object_msg.objects.push_back(object);
  }

  topic_handlers[i]->object_publisher->publish(object_msg);
}
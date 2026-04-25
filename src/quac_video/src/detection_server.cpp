#include "detection_server/detection_server.hpp"

DetectionServer::DetectionServer(const std::string& name, DetectionCallback callback) : Node(name), tf_buffer(this->get_clock()), tf_listener(tf_buffer), detection_callback(callback)
{
  declare_parameter<std::vector<std::string>>("camera_names", std::vector<std::string>{"cam"});
  camera_names = get_parameter("camera_names").as_string_array();

  declare_parameter<std::string>("reference_frame", "map");
  mapping.reference_frame = get_parameter("reference_frame").as_string();

  declare_parameter<int>("publish_rate", 10);
  int publish_rate = get_parameter("publish_rate").as_int();

  declare_parameter<std::string>("topic_name", "objects");
  topic_name = get_parameter("topic_name").as_string();

  callback_group = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  mapping.timer = create_wall_timer(std::chrono::milliseconds((int)(1000.f/(float)publish_rate)), std::bind(&DetectionServer::publish_callback, this), callback_group);

  mapping.object_publisher = create_publisher<quac_interfaces::msg::DetectedObjectArray>(topic_name, 10);

  topic_handlers.resize(camera_names.size());

  rclcpp::SubscriptionOptions options;
  options.callback_group = callback_group;
  
  for (int i = 0; i < topic_handlers.size(); i++)
  {
    topic_handlers[i] = std::make_unique<TopicHandler>();
    topic_handlers[i]->subscriber = create_subscription<quac_interfaces::msg::ImageBGRD>(camera_names[i] + "/bgrd",10,[this, i](quac_interfaces::msg::ImageBGRD::SharedPtr msg) {image_callback(msg, i);}, options);
    topic_handlers[i]->object_publisher = create_publisher<quac_interfaces::msg::DetectedObjectArray>(camera_names[i] + "/" + topic_name, 10);
  }
}

void DetectionServer::publish_callback()
{
  quac_interfaces::msg::DetectedObjectArray object_msg;
  object_msg.header.stamp = now();
  object_msg.header.frame_id = mapping.reference_frame;

  {
    std::lock_guard<std::mutex> lock(mapping.detections_mutex);
    if (mapping.objects.size() == 0) return;
    object_msg.objects = mapping.objects;
  }  

  mapping.object_publisher->publish(object_msg);
}

void DetectionServer::image_callback(quac_interfaces::msg::ImageBGRD::SharedPtr msg, int i)
{
  std::vector<detection> detections;

  detection_callback(msg, i, detections);
  if (detections.size() == 0) return;

  quac_interfaces::msg::DetectedObjectArray object_msg;
  object_msg.header.stamp = now();
  object_msg.header.frame_id = msg->header.frame_id;

  for (int j = 0; j < detections.size(); j++)
  {
    quac_interfaces::msg::DetectedObject object;
    object.name = "detection_" + std::to_string(j);
    object.type = detections[j].data;

    object.pose.orientation.x = 0.0;
    object.pose.orientation.y = 0.0;
    object.pose.orientation.z = 0.0;
    object.pose.orientation.w = 1.0;

    int u = (detections[j].corners[0].x + detections[j].corners[1].x + detections[j].corners[2].x + detections[j].corners[3].x)/ 4;
    int v = (detections[j].corners[0].y + detections[j].corners[1].y + detections[j].corners[2].y + detections[j].corners[3].y)/ 4;

    object.pose.position.z = ((float)msg->depth_data[v * msg->width + u]) * msg->depth_scale;
    object.pose.position.x = (u - msg->ppx) * object.pose.position.z / msg->fx;
    object.pose.position.y = (v - msg->ppy) * object.pose.position.z / msg->fy;

    object_msg.objects.push_back(object);
  }

  topic_handlers[i]->object_publisher->publish(object_msg);

  std::lock_guard<std::mutex> lock(mapping.detections_mutex);

  geometry_msgs::msg::TransformStamped T_map_camera = tf_buffer.lookupTransform(
    mapping.reference_frame,
    msg->header.frame_id,
    tf2::TimePointZero
  );

  for (auto &object : object_msg.objects)
  {
    tf2::Transform tf_obj_cam;
    tf2::fromMsg(object.pose, tf_obj_cam);

    tf2::Transform tf_obj_map = tf_map_camera * tf_obj_cam;

    geometry_msgs::msg::PoseStamped obj_map;
    obj_map.header.frame_id = "map";
    obj_map.pose = tf2::toMsg(tf_obj_map);

    objects_map.push_back(obj_map);
  }
}
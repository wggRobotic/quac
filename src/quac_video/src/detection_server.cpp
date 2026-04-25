#include "detection_server/detection_server.hpp"
#include <cmath>

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

  declare_parameter<double>("consideration_radius", 0.1);
  consideration_radius = get_parameter("consideration_radius").as_double();

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

void DetectionServer::draw_bounding_box(cv::Mat& image, detection& detection)
{
  cv::Scalar color(0, 0, 255);

  cv::rectangle(
    image,
    cv::Point(detection.corners[0].x, detection.corners[0].y),
    cv::Point(detection.corners[2].x, detection.corners[2].y),
    color, 2, cv::LINE_AA
  );

  int fontFace = cv::FONT_HERSHEY_SIMPLEX;
  double fontScale = std::min(image.rows, image.cols) * 0.0008;
  fontScale = std::max(fontScale, 0.4);
  int textThickness = std::max(1, static_cast<int>(std::min(image.rows, image.cols) * 0.002));
  int baseline = 0;

  cv::Size textSize = cv::getTextSize(detection.data, fontFace, fontScale, textThickness, &baseline);

  int labelY = std::max(detection.corners[0].y, textSize.height + 5);
  cv::Point labelTopLeft(detection.corners[0].x, labelY - textSize.height - 5);
  cv::Point labelBottomRight(detection.corners[0].x + textSize.width + 5, labelY + baseline - 5);

  cv::rectangle(image, labelTopLeft, labelBottomRight, color, cv::FILLED);
  cv::putText(image, detection.data, cv::Point(detection.corners[0].x + 2, labelY - 2), fontFace, fontScale, cv::Scalar(255, 255, 255), textThickness, cv::LINE_AA);
}

void DetectionServer::image_callback(quac_interfaces::msg::ImageBGRD::SharedPtr msg, int i)
{
  std::vector<detection> detections;

  detection_callback(msg, i, detections);
  if (detections.size() == 0) return;

  quac_interfaces::msg::DetectedObjectArray object_msg;
  object_msg.header.stamp = now();
  object_msg.header.frame_id = msg->header.frame_id;
  object_msg.objects.reserve(detections.size());

  for (int j = 0; j < detections.size(); j++)
  {
    quac_interfaces::msg::DetectedObject object;
    object.name = "detection_" + std::to_string(j);
    object.type = detections[j].data;
    object.last_detection_stamp = msg->header.stamp;

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

 
  // global frame
  geometry_msgs::msg::TransformStamped tf_msg;

  try
  {
    tf_msg = tf_buffer.lookupTransform(
      mapping.reference_frame,
      msg->header.frame_id,
      tf2::TimePointZero
    );
  }
  catch (const tf2::TransformException &ex)
  {
    RCLCPP_WARN(this->get_logger(), "TF lookup failed: %s", ex.what());
    return;
  }

  tf2::Transform global_to_camera;
  tf2::fromMsg(tf_msg.transform, global_to_camera);

  for (auto &object : object_msg.objects)
  {
    tf2::Transform camera_to_object;
    tf2::fromMsg(object.pose, camera_to_object);

    tf2::toMsg(global_to_camera * camera_to_object, object.pose);
  }

  std::lock_guard<std::mutex> lock(mapping.detections_mutex);

  for (int j = 0; j < object_msg.objects.size(); j++)
  {
    for (int k = 0; ; k++)
    {
      if (k < mapping.objects.size())
      {
        if (object_msg.objects[j].type == mapping.objects[k].type)
        {
          geometry_msgs::msg::Point& mapped_object_pos = mapping.objects[k].pose.position;
          geometry_msgs::msg::Point& object_pos = object_msg.objects[j].pose.position;

          double distance = std::sqrt(
            (mapped_object_pos.x - object_pos.x) * (mapped_object_pos.x - object_pos.x) +
            (mapped_object_pos.y - object_pos.y) * (mapped_object_pos.y - object_pos.y) +
            (mapped_object_pos.z - object_pos.z) * (mapped_object_pos.z - object_pos.z)
          );
          if (distance < consideration_radius)
          {
            
            mapped_object_pos.x = 
              (mapped_object_pos.x * (double)mapping.object_datas[k].times_detected + object_pos.x) / 
              (double)(mapping.object_datas[k].times_detected + 1);

            mapped_object_pos.y = 
              (mapped_object_pos.y * (double)mapping.object_datas[k].times_detected + object_pos.y) / 
              (double)(mapping.object_datas[k].times_detected + 1);

            mapped_object_pos.z = 
              (mapped_object_pos.z * (double)mapping.object_datas[k].times_detected + object_pos.z) / 
              (double)(mapping.object_datas[k].times_detected + 1);

            mapping.object_datas[k].times_detected++;

            if (detections[j].confidence > mapping.object_datas[k].confidence)
            {
              mapping.object_datas[k].confidence = detections[j].confidence;

              cv::Mat temp(msg->height, msg->width, CV_8UC3, msg->bgr_data.data());
              mapping.object_datas[k].image = temp.clone();
              draw_bounding_box(mapping.object_datas[k].image, detections[j]);
            }

            break;
          }
        }
      }
      else
      {
        mapping.objects.push_back(object_msg.objects[j]);

        mapped_object_data data;
        data.times_detected = 1.;
        data.confidence = detections[j].confidence;

        cv::Mat temp(msg->height, msg->width, CV_8UC3, msg->bgr_data.data());
        data.image = temp.clone();
        draw_bounding_box(data.image, detections[j]);

        mapping.object_datas.push_back(data);

        break;
      }
      
    }
  }
}
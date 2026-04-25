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

  declare_parameter<int>("publish_images_period", 10);
  int publish_images_period = get_parameter("publish_images_period").as_int();

  declare_parameter<std::string>("object_name", "object");
  object_name = get_parameter("object_name").as_string();

  declare_parameter<double>("consideration_radius", 0.1);
  consideration_radius = get_parameter("consideration_radius").as_double();

  callback_group = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  mapping.object_timer = create_wall_timer(std::chrono::milliseconds((int)(1000.f/(float)publish_rate)), std::bind(&DetectionServer::publish_objects_callback, this), callback_group);
  mapping.image_timer = create_wall_timer(std::chrono::milliseconds((int)(1000.f * (float)publish_images_period)), std::bind(&DetectionServer::publish_images_callback, this), callback_group);

  mapping.object_publisher = create_publisher<quac_interfaces::msg::DetectedObjectArray>(object_name + "s", 10);
  mapping.image_publisher = create_publisher<sensor_msgs::msg::Image>(object_name + "s/images", 10);

  topic_handlers.resize(camera_names.size());

  rclcpp::SubscriptionOptions options;
  options.callback_group = callback_group;
  
  for (int i = 0; i < topic_handlers.size(); i++)
  {
    topic_handlers[i] = std::make_unique<TopicHandler>();
    topic_handlers[i]->subscriber = create_subscription<quac_interfaces::msg::ImageBGRD>(camera_names[i] + "/bgrd",10,[this, i](quac_interfaces::msg::ImageBGRD::SharedPtr msg) {image_callback(msg, i);}, options);
    topic_handlers[i]->object_publisher = create_publisher<quac_interfaces::msg::DetectedObjectArray>(camera_names[i] + "/" + object_name + "s", 10);
  }
}

void DetectionServer::publish_objects_callback()
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

void DetectionServer::publish_images_callback()
{
  std::lock_guard<std::mutex> lock(mapping.detections_mutex);
  for (int i = 0; i < mapping.object_datas.size(); i++)
    mapping.image_publisher->publish(*(mapping.object_datas[i].image));
  
}

void DetectionServer::draw_bounding_box(cv::Mat& image, const detection& detection, const std::string name)
{
  cv::Scalar color(0, 0, 255);

  int x_min = image.cols, y_min = image.rows, x_max = 0, y_max = 0;
  for (int i = 0; i < 4; i++)
  {
    if (x_min > detection.corners[i].x) x_min = detection.corners[i].x;
    if (y_min > detection.corners[i].y) y_min = detection.corners[i].y;
    if (x_max < detection.corners[i].x) x_max = detection.corners[i].x;
    if (y_max < detection.corners[i].y) y_max = detection.corners[i].y;
  }

  cv::rectangle(
    image,
    cv::Point(x_min, y_min),
    cv::Point(x_max, y_max),
    color, 2, cv::LINE_AA
  );

  std::string label = name + "__" + detection.data + "_" + std::to_string(detection.confidence);

  int fontFace = cv::FONT_HERSHEY_SIMPLEX;
  double fontScale = std::min(image.rows, image.cols) * 0.0008;
  fontScale = std::max(fontScale, 0.4);
  int textThickness = std::max(1, static_cast<int>(std::min(image.rows, image.cols) * 0.002));
  int baseline = 0;

  cv::Size textSize = cv::getTextSize(label, fontFace, fontScale, textThickness, &baseline);

  int labelY = std::max(y_min, textSize.height + 5);
  cv::Point labelTopLeft(x_min, labelY - textSize.height - 5);
  cv::Point labelBottomRight(x_min + textSize.width + 5, labelY + baseline - 5);

  cv::rectangle(image, labelTopLeft, labelBottomRight, color, cv::FILLED);
  cv::putText(image, label, cv::Point(x_min + 2, labelY - 2), fontFace, fontScale, cv::Scalar(255, 255, 255), textThickness, cv::LINE_AA);
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
    object.confidence = detections[j].confidence;
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

            if (object_msg.objects[j].confidence > mapping.objects[k].confidence)
            {
              mapping.objects[k].confidence = object_msg.objects[j].confidence;

              mapping.object_datas[k].image->width = msg->width;
              mapping.object_datas[k].image->height = msg->height;
              mapping.object_datas[k].image->data.resize(msg->height * msg->width * 3);
              memcpy(mapping.object_datas[k].image->data.data(), msg->bgr_data.data(), msg->height * msg->width * 3);
            
              cv::Mat temp(msg->height, msg->width, CV_8UC3, mapping.object_datas[k].image->data.data());
              draw_bounding_box(temp, detections[j], mapping.objects[k].name);
            }

            break;
          }
        }
      }
      else
      {
        object_msg.objects[j].name = object_name + "_" + std::to_string(k);
        mapping.objects.push_back(object_msg.objects[j]);

        mapped_object_data data;
        data.times_detected = 1.;
        
        data.image = std::make_shared<sensor_msgs::msg::Image>();
        data.image->width = msg->width;
        data.image->height = msg->height;
        data.image->encoding = "bgr8";
        data.image->header.frame_id = mapping.objects[k].name;
        data.image->data.resize(msg->height * msg->width * 3);
        memcpy(data.image->data.data(), msg->bgr_data.data(), msg->height * msg->width * 3);

        cv::Mat temp(msg->height, msg->width, CV_8UC3, data.image->data.data());
        draw_bounding_box(temp, detections[j], mapping.objects[k].name);

        mapping.object_datas.push_back(data);

        break;
      }
      
    }
  }
}
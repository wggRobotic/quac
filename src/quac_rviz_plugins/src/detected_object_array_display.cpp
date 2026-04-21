#include <quac_rviz_plugins/detected_object_array_display.hpp>
#include <rviz_common/properties/parse_color.hpp>
#include <rviz_common/logging.hpp>

namespace quac_rviz_plugins
{

using rviz_common::properties::StatusProperty;

void DetectedObjectArrayDisplay::onInitialize()
{
  MFDClass::onInitialize();
  point_shape =
    std::make_unique<rviz_rendering::Shape>(rviz_rendering::Shape::Type::Cube, scene_manager_,
      scene_node_);

  color_property = std::make_unique<rviz_common::properties::ColorProperty>(
      "Point Color", QColor(36, 64, 142), "Color to draw the point.", this, SLOT(updateStyle()));
  updateStyle();
}

void DetectedObjectArrayDisplay::processMessage(const quac_interfaces::msg::DetectedObjectArray::ConstSharedPtr msg)
{
  RVIZ_COMMON_LOG_INFO_STREAM("We got a message with frame " << msg->header.frame_id);

  Ogre::Vector3 position;
  Ogre::Quaternion orientation;
  if (!context_->getFrameManager()->getTransform(msg->header, position, orientation)) {
    RVIZ_COMMON_LOG_DEBUG_STREAM("Error transforming from frame '" << msg->header.frame_id <<
        "' to frame '" << qPrintable(fixed_frame_) << "'");
  }

  scene_node_->setPosition(position);
  scene_node_->setOrientation(orientation);

  if (msg->objects[0].pose.position.x < 0) {
    setStatus(StatusProperty::Warn, "Message",
        "I will complain about points with negative x values.");
  } else {
    setStatus(StatusProperty::Ok, "Message", "OK");
  }

  Ogre::Vector3 point_pos;
  point_pos.x = msg->objects[0].pose.position.x;
  point_pos.y = msg->objects[0].pose.position.y;
  point_shape->setPosition(point_pos);
}

void DetectedObjectArrayDisplay::updateStyle()
{
  Ogre::ColourValue color = rviz_common::properties::qtToOgre(color_property->getColor());
  point_shape->setColor(color);
}

}

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(quac_rviz_plugins::DetectedObjectArrayDisplay, rviz_common::Display)
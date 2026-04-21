#ifndef QUAC_RVIZ_PLUGINS__DETECTED_OBJECT_ARRAY_DISPLAY_HPP_
#define QUAC_RVIZ_PLUGINS__DETECTED_OBJECT_ARRAY_DISPLAY_HPP_

#include <rviz_common/message_filter_display.hpp>
#include <rviz_common/properties/color_property.hpp>
#include <rviz_rendering/objects/shape.hpp>
#include <quac_interfaces/msg/detected_object_array.hpp>

namespace quac_rviz_plugins
{
  
class DetectedObjectArrayDisplay : public rviz_common::MessageFilterDisplay<quac_interfaces::msg::DetectedObjectArray>
{
  Q_OBJECT

private Q_SLOTS:
  void updateStyle();

protected:
  void onInitialize() override;

  void processMessage(const quac_interfaces::msg::DetectedObjectArray::ConstSharedPtr msg) override;

  std::unique_ptr<rviz_rendering::Shape> point_shape;
  std::unique_ptr<rviz_common::properties::ColorProperty> color_property;
};

}

#endif
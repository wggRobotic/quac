#ifndef QUAC_RVIZ_PLUGINS__DETECTED_OBJECT_ARRAY_DISPLAY_HPP_
#define QUAC_RVIZ_PLUGINS__DETECTED_OBJECT_ARRAY_DISPLAY_HPP_

#include <rviz_common/message_filter_display.hpp>
#include <rviz_rendering/objects/shape.hpp>
#include <quac_interfaces/msg/detected_object_array.hpp>
#include <rviz_rendering/objects/movable_text.hpp>

#include <rviz_common/properties/color_property.hpp>
#include <rviz_common/properties/float_property.hpp>

struct ObjectVisual
{
  std::unique_ptr<rviz_rendering::Shape> sphere;
  std::unique_ptr<rviz_rendering::MovableText> label;
  Ogre::SceneNode* label_node = nullptr;
  Ogre::SceneNode* node;
};

namespace quac_rviz_plugins
{
  
class DetectedObjectArrayDisplay : public rviz_common::MessageFilterDisplay<quac_interfaces::msg::DetectedObjectArray>
{
  Q_OBJECT

  ~DetectedObjectArrayDisplay();
  void onDisable() override;
  void clearVisuals();

private Q_SLOTS:
  void updateStyle();

protected:
  void onInitialize() override;

  void processMessage(const quac_interfaces::msg::DetectedObjectArray::ConstSharedPtr msg) override;

  std::vector<ObjectVisual> visuals;
  std::unique_ptr<rviz_common::properties::FloatProperty> size_property;
  std::unique_ptr<rviz_common::properties::ColorProperty> color_property;

};

}

#endif
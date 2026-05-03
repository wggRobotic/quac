#include <quac_rviz_plugins/detected_object_array_display.hpp>
#include <rviz_common/logging.hpp>
#include <rviz_common/properties/parse_color.hpp>

namespace quac_rviz_plugins
{

void DetectedObjectArrayDisplay::onInitialize()
{
  MFDClass::onInitialize();
  size_property = std::make_unique<rviz_common::properties::FloatProperty>(
    "size",
    0.2f,
    "Size of the spheres",
    this,
    SLOT(updateStyle())
  );
  color_property = std::make_unique<rviz_common::properties::ColorProperty>("color", QColor(36, 64, 142), "Color to draw the sqheres.", this, SLOT(updateStyle()));
}

void DetectedObjectArrayDisplay::clearVisuals()
{

  for (auto & vis : visuals) {
    if (vis.label && vis.label_node) {
      vis.label_node->detachObject(vis.label.get());
    }
  }
  visuals.clear();
  scene_node_->removeAndDestroyAllChildren();
}

void DetectedObjectArrayDisplay::onDisable()
{
  clearVisuals();
  MFDClass::onDisable();
}

DetectedObjectArrayDisplay::~DetectedObjectArrayDisplay()
{
  if (scene_manager_) {
    clearVisuals();
  }
}

void DetectedObjectArrayDisplay::processMessage(const quac_interfaces::msg::DetectedObjectArray::ConstSharedPtr msg)
{
  clearVisuals();

  for (const auto & obj : msg->objects) {
    Ogre::Vector3 position;
    Ogre::Quaternion orientation;

    if (!context_->getFrameManager()->transform(
        msg->header, obj.pose, position, orientation))
    {
      continue;
    }

    ObjectVisual vis;

    vis.node = scene_node_->createChildSceneNode();
    vis.node->setPosition(position);
    vis.node->setOrientation(orientation);
    
    vis.sphere = std::make_unique<rviz_rendering::Shape>(
      rviz_rendering::Shape::Type::Sphere,
      scene_manager_,
      vis.node);

    vis.label_node = vis.node->createChildSceneNode();
    vis.label = std::make_unique<rviz_rendering::MovableText>(obj.header.frame_id + "\n" + obj.box.header.frame_id);
    vis.label_node->attachObject(vis.label.get());

    visuals.push_back(std::move(vis));
  }

  updateStyle();
}

void DetectedObjectArrayDisplay::updateStyle()
{
  if (!size_property || !color_property) return;

  float size = size_property->getFloat();
  Ogre::ColourValue color = rviz_common::properties::qtToOgre(color_property->getColor());

  for (auto & vis : visuals) {
    if (vis.sphere) {
      vis.sphere->setScale(Ogre::Vector3(size, size, size));
      vis.sphere->setColor(color);
      vis.label_node->setPosition(Ogre::Vector3(0.0f, 0.0f, 2.5f * size));
      vis.label->setCharacterHeight(size);
    }
  }
}

}

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  quac_rviz_plugins::DetectedObjectArrayDisplay,
  rviz_common::Display)
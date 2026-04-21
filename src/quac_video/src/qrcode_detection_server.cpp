#include "qrcode_detection_server/qrcode_detection_server.hpp"

QRCodeDetectionServer::QRCodeDetectionServer() : Node("qrcode_detection_server")
{
    
}

void QRCodeDetectionServer::run()
{
    
}

int main (int argc, char *argv[])
{
  rclcpp::init(argc, argv);

  auto node = std::make_shared<QRCodeDetectionServer>();
  node->run();
  
  rclcpp::shutdown();
}
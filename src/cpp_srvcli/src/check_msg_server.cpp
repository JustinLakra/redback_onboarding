#include "cipher_interfaces/srv/cipher_answer.hpp"
#include "rclcpp/rclcpp.hpp"

#include <memory>

void check_msg(
    const std::shared_ptr<cipher_interfaces::srv::CipherAnswer::Request>
        request,
    std::shared_ptr<cipher_interfaces::srv::CipherAnswer::Response> response) {
  response->result = request->answer == "Hello World" ? true : false;
  RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Incoming request\nanswer: %s",
              request->answer.c_str());
  RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "sending back response: [%s]",
              response->result ? "true" : "false");
}

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  std::shared_ptr<rclcpp::Node> node =
      rclcpp::Node::make_shared("check_msg_server");

  rclcpp::Service<cipher_interfaces::srv::CipherAnswer>::SharedPtr service =
      node->create_service<cipher_interfaces::srv::CipherAnswer>("check_msg",
                                                                 &check_msg);

  RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Ready to check answer.");

  rclcpp::spin(node);
  rclcpp::shutdown();
}

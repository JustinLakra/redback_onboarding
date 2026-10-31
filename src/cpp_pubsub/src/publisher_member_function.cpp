// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "cipher_interfaces/msg/cipher_message.hpp"
#include "cipher_interfaces/srv/cipher_answer.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses std::bind() to register a
 * member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node {
public:
  MinimalPublisher() : Node("minimal_publisher"), count_(0) {
    this->declare_parameter("message", "Hello World");
    this->declare_parameter("key", 1);
    publisher_ = this->create_publisher<cipher_interfaces::msg::CipherMessage>(
        "topic", 10);
    // std::shared_ptr<rclcpp::Node> node =
    //     rclcpp::Node::make_shared("check_msg_server");
    service_ = this->create_service<cipher_interfaces::srv::CipherAnswer>(
        "check_msg", std::bind(&MinimalPublisher::check_msg, this,
                               std::placeholders::_1, std::placeholders::_2));

    if (!service_) {
      RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to create service");
      return;
    }
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Ready to check answer.");
    timer_ = this->create_wall_timer(
        500ms, std::bind(&MinimalPublisher::timer_callback, this));
  }

private:
  void timer_callback() {
    auto message = cipher_interfaces::msg::CipherMessage();
    message.message = encode(this->get_parameter("key").as_int(),
                             this->get_parameter("message").as_string());
    message.key = this->get_parameter("key").as_int();
    RCLCPP_INFO_STREAM(this->get_logger(),
                       "Publishing: '" << message.message << "' with key: '"
                                       << static_cast<int>(message.key) << "'");
    publisher_->publish(message);
  }
  std::string encode(int key, std::string message) {
    for (char &c : message) {
      if (c >= 'a' && c <= 'z') {
        c = ((c - 'a' + key % 26) % 26) + 'a';
      } else if (c >= 'A' && c <= 'Z') {
        c = ((c - 'A' + key % 26) % 26) + 'A';
      }
    }
    return message;
  }
  void
  check_msg(const std::shared_ptr<cipher_interfaces::srv::CipherAnswer::Request>
                request,
            std::shared_ptr<cipher_interfaces::srv::CipherAnswer::Response>
                response) {
    response->result =
        request->answer == this->get_parameter("message").as_string() ? true
                                                                      : false;
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Incoming request\nanswer: %s",
                request->answer.c_str());
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "sending back response: [%s]",
                response->result ? "true" : "false");
  }
  rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<cipher_interfaces::msg::CipherMessage>::SharedPtr
      publisher_;
  rclcpp::Service<cipher_interfaces::srv::CipherAnswer>::SharedPtr service_;
  size_t count_;
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalPublisher>());
  rclcpp::shutdown();
  return 0;
}

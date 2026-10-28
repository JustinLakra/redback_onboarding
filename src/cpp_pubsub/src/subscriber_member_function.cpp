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

#include "cipher_interfaces/msg/cipher_message.hpp"
#include "cipher_interfaces/srv/cipher_answer.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using std::placeholders::_1;
using namespace std::chrono_literals;

class MinimalSubscriber : public rclcpp::Node {
public:
  MinimalSubscriber() : Node("minimal_subscriber") {
    subscription_ =
        this->create_subscription<cipher_interfaces::msg::CipherMessage>(
            "topic", 10,
            std::bind(&MinimalSubscriber::topic_callback, this, _1));
  }

private:
  void topic_callback(const cipher_interfaces::msg::CipherMessage &msg) {
    // RCLCPP_INFO_STREAM(this->get_logger(),
    //                    "I heard: '" << msg.message << "' with key: "
    //                                 << static_cast<int>(msg.key)
    //                                 << " and decoded message: '"
    //                                 << decode(msg.key, msg.message) << "'");

    RCLCPP_INFO_STREAM(this->get_logger(), "I heard: '"
                                               << msg.message << "' with key: "
                                               << static_cast<int>(msg.key));

    std::shared_ptr<rclcpp::Node> node =
        rclcpp::Node::make_shared("check_msg_client");
    rclcpp::Client<cipher_interfaces::srv::CipherAnswer>::SharedPtr client =
        node->create_client<cipher_interfaces::srv::CipherAnswer>("check_msg");

    auto request =
        std::make_shared<cipher_interfaces::srv::CipherAnswer::Request>();
    request->answer = decode(msg.key, msg.message);

    while (!client->wait_for_service(1s)) {
      if (!rclcpp::ok()) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"),
                     "Interrupted while waiting for the service. Exiting.");
        return;
      }
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"),
                  "service not available, waiting again...");
    }

    auto result = client->async_send_request(request);
    // Wait for the result.
    if (rclcpp::spin_until_future_complete(node, result) ==
        rclcpp::FutureReturnCode::SUCCESS) {
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Result: %s",
                  result.get()->result ? "true" : "false");
    } else {
      RCLCPP_ERROR(rclcpp::get_logger("rclcpp"),
                   "Failed to call service check_msg");
    }
  }
  std::string decode(int key, std::string message) {
    for (char &c : message) {
      if (c >= 'a' && c <= 'z') {
        c = (c - 'a' - (key % 26) + 26) % 26 + 'a';
      } else if (c >= 'A' && c <= 'Z') {
        c = (c - 'A' - (key % 26) + 26) % 26 + 'A';
      }
    }
    return message;
  }
  rclcpp::Subscription<cipher_interfaces::msg::CipherMessage>::SharedPtr
      subscription_;
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}

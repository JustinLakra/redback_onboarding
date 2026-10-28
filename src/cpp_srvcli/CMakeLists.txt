cmake_minimum_required(VERSION 3.8)
project(cpp_srvcli)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cipher_interfaces REQUIRED)

add_executable(server src/check_msg_server.cpp)
ament_target_dependencies(server rclcpp cipher_interfaces)

install(TARGETS
  server
  DESTINATION lib/${PROJECT_NAME})

ament_package()

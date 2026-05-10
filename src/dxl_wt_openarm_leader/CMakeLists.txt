cmake_minimum_required(VERSION 3.8)
project(dxl_wt_openarm_leader)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(dynamixel_sdk REQUIRED)
find_package(dynamixel_sdk_custom_interfaces REQUIRED)
find_package(rclcpp REQUIRED)

#for openarm
find_package(std_msgs REQUIRED)
find_package(control_msgs REQUIRED)
find_package(rclcpp_action REQUIRED)



include_directories(include)

add_executable(read_write_wt_position_node src/read_write_wt_position_node.cpp)
ament_target_dependencies(read_write_wt_position_node
  dynamixel_sdk_custom_interfaces
  dynamixel_sdk
  rclcpp
  #for openarm
  rclcpp_action
  std_msgs
  control_msgs
)

add_executable(pub_current_node src/pub_current_node.cpp) 
ament_target_dependencies(pub_current_node
  dynamixel_sdk_custom_interfaces
  dynamixel_sdk
  rclcpp
)

install(TARGETS
  read_write_wt_position_node
  pub_current_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

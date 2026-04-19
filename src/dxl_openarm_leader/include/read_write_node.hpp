// Copyright 2021 ROBOTIS CO., LTD.
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

#ifndef READ_WRITE_NODE_HPP_
#define READ_WRITE_NODE_HPP_

#include <cstdio>
#include <memory>
#include <string>

#include "dynamixel_sdk/dynamixel_sdk.h"
#include "dynamixel_sdk_custom_interfaces/msg/set_position.hpp"
#include "dynamixel_sdk_custom_interfaces/srv/get_position.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rcutils/cmdline_parser.h"
// #include "std_msgs/msg/int32.hpp"  // 追加
#include "std_msgs/msg/float64_multi_array.hpp"
#include "control_msgs/action/gripper_command.hpp"
#include "rclcpp_action/rclcpp_action.hpp"




class ReadWriteNode : public rclcpp::Node
{
public:
  using SetPosition = dynamixel_sdk_custom_interfaces::msg::SetPosition;
  using GetPosition = dynamixel_sdk_custom_interfaces::srv::GetPosition;
  using GripperCommand = control_msgs::action::GripperCommand;
  using GoalHandleGripperCommand = rclcpp_action::ClientGoalHandle<GripperCommand>;

  ReadWriteNode();
  virtual ~ReadWriteNode();

private:
  // 追加: timerから呼ばれる現在位置読み取り関数
  void read_present_position();
  void send_left_gripper_goal(double position);
  void send_right_gripper_goal(double position);

  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr left_openarm_command_publisher_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr right_openarm_command_publisher_;

  rclcpp_action::Client<GripperCommand>::SharedPtr left_gripper_client_;
  rclcpp_action::Client<GripperCommand>::SharedPtr right_gripper_client_;


  rclcpp::Subscription<SetPosition>::SharedPtr set_position_subscriber_;
  rclcpp::Service<GetPosition>::SharedPtr get_position_server_;


  // 追加: 一定周期で現在位置を読むためのtimer
  rclcpp::TimerBase::SharedPtr timer_;

  int present_position;
};

#endif  // READ_WRITE_NODE_HPP_

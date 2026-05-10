// Copyright (c) 2023 Takumi Asada
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

#ifndef JOINT_PUB_NODE_HPP_
#define JOINT_PUB_NODE_HPP_

#include <vector>

#include "dynamixel_sdk_custom_interfaces/msg/set_current.hpp"
#include "dynamixel_sdk_custom_interfaces/msg/set_position.hpp"
#include "rclcpp/rclcpp.hpp"

class JointPubNode : public rclcpp::Node
{
public:
  using SetPosition = dynamixel_sdk_custom_interfaces::msg::SetPosition;

  JointPubNode();

private:
  void publishData();
  rclcpp::Publisher<SetPosition>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::vector<int> positions_3_ = {0, 1023, 2047, 3095, 4095, 3095, 2047, 1023};
  
  size_t current_position_index_ = 0;
};

class JointCurrentPubNode : public rclcpp::Node
{
public:
  using SetCurrent = dynamixel_sdk_custom_interfaces::msg::SetCurrent;

  JointCurrentPubNode();

private:
  void publishData();
  rclcpp::Publisher<SetCurrent>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::vector<int> currents_0_ = {0, -38, -38, -38, -38, -38, -38, -38};
  size_t current_position_index_ = 0;
};

#endif  // JOINT_PUB_NODE_HPP_

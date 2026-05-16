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

#ifndef READ_WRITE_WT_NODE_HPP_
#define READ_WRITE_WT_NODE_HPP_

#include <cstdio>
#include <memory>
#include <string>
#include <array>


#include "dynamixel_sdk/dynamixel_sdk.h"
#include "dynamixel_sdk_custom_interfaces/msg/set_position.hpp"
#include "dynamixel_sdk_custom_interfaces/srv/get_position.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rcutils/cmdline_parser.h"
// #include "std_msgs/msg/int32.hpp"  // 追加
#include "std_msgs/msg/float64_multi_array.hpp"
#include "control_msgs/action/gripper_command.hpp"
#include "rclcpp_action/rclcpp_action.hpp"




class ReadWriteWTNode : public rclcpp::Node
{
public:
  using SetPosition = dynamixel_sdk_custom_interfaces::msg::SetPosition;
  using GetPosition = dynamixel_sdk_custom_interfaces::srv::GetPosition;
  using GripperCommand = control_msgs::action::GripperCommand;
  using GoalHandleGripperCommand = rclcpp_action::ClientGoalHandle<GripperCommand>;

  ReadWriteWTNode();
  virtual ~ReadWriteWTNode();

private:
  // 追加: timerから呼ばれる現在位置読み取り関数
  void read_present_position();
  void send_left_gripper_goal(double position);
  void send_right_gripper_goal(double position);

  void motion_compensation();

  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr left_openarm_command_publisher_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr right_openarm_command_publisher_;

  rclcpp_action::Client<GripperCommand>::SharedPtr left_gripper_client_;
  rclcpp_action::Client<GripperCommand>::SharedPtr right_gripper_client_;


  rclcpp::Subscription<SetPosition>::SharedPtr set_position_subscriber_;
  rclcpp::Service<GetPosition>::SharedPtr get_position_server_;


  // 追加: 一定周期で現在位置を読むためのtimer
  rclcpp::TimerBase::SharedPtr timer_;


  std::unique_ptr<dynamixel::GroupFastSyncRead> groupfastsyncread_;
  std::unique_ptr<dynamixel::GroupSyncWrite> group_sync_write_current_;

  std::vector<uint8_t> dxl_ids_ = {1};

  std::array<int, 18> present_position_{};
  std::array<double, 18> present_vel_rad_s_{};
  std::array<double, 18> prev_vel_rad_s_{};
  std::array<double, 18> present_acc_rad_ss_{};

  std::array<int, 18> present_current_mA_{};



  int read_period_ms_ = 10;
  double read_period_sec_ = read_period_ms_/1000.0;

  /* Servo physical value*/
  int current_limit_mA_ = 150;
  double kt_ = 0.354;  // Nm/A


  std::array<double, 18> J_{
    0.000022,0.000022,0.000022,0.000022,0.000022,0.000022,0.000032,0.000022,
    0.000022,0.000022,0.000022,0.000022,0.000022,0.000022,0.000022,0.000022,
    0.000022,0.000022
  };  // 慣性 [kg·m²]
  std::array<double, 18> b_{
    0.000095,0.000095,0.000095,0.000095,0.000095,0.000095,0.000095,0.000095,
    0.000095,0.000095,0.000095,0.000095,0.000095,0.000095,0.000095,0.000095,
    0.000095,0.000095
  };  // 粘性係数 [Nm·s/rad]
  std::array<double, 18> Ts_{
    0.00286,0.00286,0.00286,0.00286,0.00286,0.00286,0.00386,0.00286,
    0.00286,0.00286,0.00286,0.00286,0.00286,0.00286,0.00286,0.00286,
    0.00286,0.00286
  }; // 静止摩擦 [Nm]
  std::array<double, 18> Tc_{
    0.00328,0.00328,0.00328,0.00328,0.00328,0.00328,0.00428,0.00328,
    0.00328,0.00328,0.00328,0.00328,0.00328,0.00328,0.00328,0.00328,
    0.00328,0.00328
  }; // 動摩擦 [Nm]
  std::array<double, 18> vs_{
    0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,
    0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1,
    0.1,0.1
  }; // ストライベック速度 [rad/s]

  //仮想ばね [Nm/rad]
  std::array<double, 18> Kp_{
      0.00004, 0.011, 0.011, 0.011, 0.011, 0.011, 0.000011, 0.011,
      0.011, 0.011, 0.011, 0.011, 0.011, 0.011, 0.011, 0.011,
      0.011, 0.011
  };
  //仮想ダンパ [Nm·s/rad]
  std::array<double, 18> Kd_{
      0.00012, 0.0012, 0.0012, 0.0012, 0.0012, 0.0012, 0.00012, 0.0012,
      0.0012, 0.0012, 0.0012, 0.0012, 0.0012, 0.0012, 0.0012, 0.0012,
      0.0012, 0.0012
  };
  //仮想慣性 [kg·m²]
  std::array<double, 18> Jd_{
      0.0000038, 0.000038, 0.000038, 0.000038, 0.000038, 0.000038, 0.000038, 0.0000038,
      0.000038, 0.000038, 0.000038, 0.000038, 0.000038, 0.000038, 0.000038, 0.000038,
      0.000038, 0.000038
  };

  std::array<int, 18> target_imp_pos_{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0
  };

  std::array<double, 18> target_imp_vel_{
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0
  };
  std::array<double, 18> target_imp_acc_{
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0
  };



};

#endif  // READ_WRITE_WT_NODE_HPP_

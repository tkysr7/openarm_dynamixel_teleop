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

/*******************************************************************************
// This example is written for DYNAMIXEL X(excluding XL-320) and MX(2.0) series with U2D2.
// For other series, please refer to the product eManual and modify the Control Table addresses and other definitions.
// To test this example, please follow the commands below.
//
// Open terminal #1
// $ ros2 run dynamixel_sdk_examples read_write_wt_node
//
// Open terminal #2 (run one of below commands at a time)
// $ ros2 topic pub -1 /set_position dynamixel_sdk_custom_interfaces/SetPosition "{id: 1, position: 1000}"
// $ ros2 service call /get_position dynamixel_sdk_custom_interfaces/srv/GetPosition "id: 1"
//
// Author: Will Son
*******************************************************************************/

/******************************************************************************/
/* include                                                                    */
/******************************************************************************/
#include <cstdio>
#include <memory>
#include <string>
#include <array>
#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>

#include "dynamixel_sdk/dynamixel_sdk.h"
#include "dynamixel_sdk_custom_interfaces/msg/set_position.hpp"
#include "dynamixel_sdk_custom_interfaces/srv/get_position.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rcutils/cmdline_parser.h"
#include "read_write_wt_position_node.hpp"

// #include "std_msgs/msg/int32.hpp"  // 追加
#include "std_msgs/msg/float64_multi_array.hpp"


/******************************************************************************/
/* define                                                                     */
/******************************************************************************/
/* Control table address for X series                                         */
#define ADDR_OPERATING_MODE       11
#define ADDR_TORQUE_ENABLE				64
#define ADDR_GOAL_CURRENT		  		102	 // Does NOT exist in Rot motors
#define ADDR_GOAL_VELOCITY				104
#define ADDR_GOAL_POSITION				116
#define ADDR_PRESENT_CURRENT			126	 // Represents "Present Load" in Rot motors
#define ADDR_PRESENT_VELOCITY			128
#define ADDR_PRESENT_POSITION			132
//For Group fast sync read
#define ADDR_PRESENT_STATUS      ADDR_PRESENT_CURRENT
#define LEN_PRESENT_STATUS       10

/* Motors ID */
#define DXL1_ID                         1
#define DXL2_ID                         2
#define DXL3_ID                         3
#define DXL4_ID                         4

/* TORQUE ENABLE/DISABLE */
#define TORQUE_ENABLE                   1	 // Value for enabling the torque
#define TORQUE_DISABLE                  0	 // Value for disabling the torque

/* OPERATING_MODE */
#define CURRENT_BASED_POSITION_CONTROL 5
#define POSITION_CONTROL				       3
#define VELOCITY_CONTROL				       1
#define CURRENT_CONTROL					       0

/* Protocol version */ 
#define PROTOCOL_VERSION 2.0  // Default Protocol version of DYNAMIXEL X series.

/* Default setting */
#define BAUDRATE 1000000  // Default Baudrate of DYNAMIXEL X series
#define DEVICE_NAME "/dev/ttyUSB0"  // [Linux]: "/dev/ttyUSB*", [Windows]: "COM*"



dynamixel::PortHandler * portHandler;
dynamixel::PacketHandler * packetHandler;

uint8_t dxl_error = 0;
uint32_t goal_position = 0;
int dxl_comm_result = COMM_TX_FAIL;

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
// 追加: timerから定期的に呼ばれてgripper goalを送る関数
void ReadWriteWTNode::send_left_gripper_goal(double position)
{
  if (!left_gripper_client_->action_server_is_ready()) {
    RCLCPP_WARN(this->get_logger(), "Left gripper action server is not ready");
    return;
  }

  auto goal_msg = GripperCommand::Goal();
  goal_msg.command.position = position;
  goal_msg.command.max_effort = 5.0;

  auto send_goal_options =
    rclcpp_action::Client<GripperCommand>::SendGoalOptions();

  left_gripper_client_->async_send_goal(goal_msg, send_goal_options);
}

void ReadWriteWTNode::send_right_gripper_goal(double position)
{
  if (!right_gripper_client_->action_server_is_ready()) {
    RCLCPP_WARN(this->get_logger(), "Right gripper action server is not ready");
    return;
  }

  auto goal_msg = GripperCommand::Goal();
  goal_msg.command.position = position;
  goal_msg.command.max_effort = 5.0;

  auto send_goal_options =
    rclcpp_action::Client<GripperCommand>::SendGoalOptions();

  right_gripper_client_->async_send_goal(goal_msg, send_goal_options);
}

void ReadWriteWTNode::motion_compensation()
{
  int goal_current_mA = 0;

  // for (size_t i = 0; i < present_position_.size(); ++i) {
  //   RCLCPP_INFO(
  //     this->get_logger(),
  //     "present_position_[%zu]: %d",
  //     i,
  //     present_position_[i]
  //   );
  // }

  for (const auto dxl_id : dxl_ids_ ) {

    // Stribeck friction model
    double sign_v = 0.0;
    if (std::abs(present_vel_rad_s_[dxl_id-1]) > 3e-2) {
      sign_v = (present_vel_rad_s_[dxl_id-1] > 0.0) ? 1.0 : -1.0;
    }

    const double velocity_ratio = std::abs(present_vel_rad_s_[dxl_id-1]) / vs_[dxl_id-1];

    double tau_f = Tc_[dxl_id-1] * sign_v + (Ts_[dxl_id-1] - Tc_[dxl_id-1]) * std::exp(-(velocity_ratio * velocity_ratio)) * sign_v + b_[dxl_id-1] * present_vel_rad_s_[dxl_id-1];

    //Impedance control torque
    double tau_imp = (Kp_[dxl_id-1] * (target_imp_pos_[dxl_id-1] -present_position_[dxl_id-1] )
                  + Kd_[dxl_id-1] * (target_imp_vel_[dxl_id-1] - present_vel_rad_s_[dxl_id-1])
                  + Jd_[dxl_id-1] * (target_imp_acc_[dxl_id-1] - present_acc_rad_ss_[dxl_id-1]));



    //Compensation TORQUE
    // double tau_cmd = J_[dxl_id-1] * present_acc_rad_ss_[dxl_id-1] + tau_f +tau_imp; //Enable Impedance control
    double tau_cmd = J_[dxl_id-1] * present_acc_rad_ss_[dxl_id-1] + tau_f; //disable Impedance control
    goal_current_mA =  static_cast<int>((tau_cmd / kt_) * 1000);
    goal_current_mA = std::clamp(goal_current_mA, -current_limit_mA_, current_limit_mA_);


    RCLCPP_INFO(this->get_logger(), "ID %d: cur: %d vel: %.3f tau_f: %.3f tau_imp: %.3f pos: %d dt: %d",
                 dxl_id, goal_current_mA, present_vel_rad_s_[dxl_id -1],tau_f, tau_imp,present_position_[dxl_id-1], read_period_ms_);

    dxl_comm_result = packetHandler->write2ByteTxRx(
      portHandler,
      dxl_id,
      ADDR_GOAL_CURRENT,
      static_cast<uint16_t>(goal_current_mA),
      &dxl_error
    );

    if (dxl_comm_result != COMM_SUCCESS) {
      RCLCPP_INFO(this->get_logger(), "ID %d: %s", dxl_id, packetHandler->getTxRxResult(dxl_comm_result));
    } else if (dxl_error != 0) {
      RCLCPP_INFO(this->get_logger(), "ID %d: %s", dxl_id, packetHandler->getRxPacketError(dxl_error));
    }
  }

}

// 追加: timerから定期的に呼ばれてPresent Statusを読む関数
void ReadWriteWTNode::read_present_position()
{
  //for time measurement
  const auto cycle_start = std::chrono::steady_clock::now();

  std_msgs::msg::Float64MultiArray openarm_msg;
  openarm_msg.data.resize(14, 0.0);

  for (size_t i = 0; i < dxl_ids_ .size(); ++i) {

    dxl_comm_result = packetHandler->read4ByteTxRx(
      portHandler,
      dxl_ids_[i],
      ADDR_PRESENT_POSITION,
    reinterpret_cast<uint32_t *>(&present_position_[dxl_ids_[i]-1]),
    &dxl_error
    );

    if (dxl_comm_result != COMM_SUCCESS) {
        RCLCPP_INFO(
          this->get_logger(),
          "ID %d: %s",
          dxl_ids_[i],
          packetHandler->getTxRxResult(dxl_comm_result)
        );
    } else if (dxl_error != 0) {
        RCLCPP_INFO(
          this->get_logger(),
          "ID %d: %s",
          dxl_ids_[i],
          packetHandler->getRxPacketError(dxl_error)
        );
    } else {
      if (dxl_ids_[i]==15){
          double normalized =
            std::clamp(static_cast<double>(present_position_[dxl_ids_[i]-1]) / 4095.0, 0.0, 1.0);

          double gripper_position = normalized * 0.04;
          send_left_gripper_goal(gripper_position);

          RCLCPP_INFO(
            this->get_logger(),
            "DXL ID: %d Present: %d -> Gripper: %.3f",
            dxl_ids_[i],
            present_position_[dxl_ids_[i]],
            gripper_position
          );

      } else if (dxl_ids_ [i]==16) {
          double normalized =
            std::clamp(static_cast<double>(present_position_[dxl_ids_[i]-1]) / 4095.0, 0.0, 1.0);

          double gripper_position = normalized * 0.04;
          send_right_gripper_goal(gripper_position);

          RCLCPP_INFO(
            this->get_logger(),
            "DXL ID: %d Present: %d -> Gripper: %.3f",
            dxl_ids_[i],
            present_position_[dxl_ids_[i]-1],
            gripper_position
          );

      } else {

        // 追加: DXLのPresent Positionをradに変換
        double joint_rad =
          (static_cast<double>(present_position_[dxl_ids_[i]-1]) - 2048.0) * 2.0 * M_PI / 4096.0;
        openarm_msg.data[i] = joint_rad;

        RCLCPP_INFO(
          this->get_logger(),
          "DXL ID: %d Present: %d -> joint%zu: %.3f rad",
          dxl_ids_[i],
          present_position_[dxl_ids_[i]-1],
          i + 1,
          joint_rad
        );
      }
    }
    // Position Value of X series is 4 byte data. For AX & MX(1.0) use 2 byte data(int16_t) for the Position Value.
    int32_t present_velocity_raw = 0;
    // Read Present Velocity (length : 4 bytes) and Convert uint32 -> int32
    // When reading 2 byte data from AX / MX(1.0), use read2ByteTxRx() instead.
    dxl_comm_result = packetHandler->read4ByteTxRx(
      portHandler,
      dxl_ids_ [i],
      ADDR_PRESENT_VELOCITY,
      (uint32_t *)&present_velocity_raw,
      &dxl_error
    );

    if (dxl_comm_result != COMM_SUCCESS) {
      RCLCPP_INFO(
        this->get_logger(),
        "ID %d: %s",
        dxl_ids_[i],
        packetHandler->getTxRxResult(dxl_comm_result)
      );
    } else if (dxl_error != 0) {
      RCLCPP_INFO(
        this->get_logger(),
        "ID %d: %s",
        dxl_ids_[i],
        packetHandler->getRxPacketError(dxl_error)
      );
    } else{
      prev_vel_rad_s_[dxl_ids_[i]-1] = present_vel_rad_s_[dxl_ids_ [i]-1];
      present_vel_rad_s_[dxl_ids_[i]-1] = static_cast<double>(present_velocity_raw) * 0.229 * (2.0 * M_PI / 60.0);
      present_acc_rad_ss_[dxl_ids_[i]-1] = (present_vel_rad_s_[dxl_ids_[i]-1] -prev_vel_rad_s_[dxl_ids_[i]-1]) / read_period_sec_;

      // RCLCPP_INFO(
      //   this->get_logger(),
      //   "getVelocity : [ID:%d] -> [VEL_raw:%d],[pres_VEL:%.3f],[prev_VEL:%.3f],[ACC:%.3f]",
      //   dxl_ids_[i],
      //   present_velocity_raw,
      //   present_vel_rad_s_[dxl_ids_ [i]-1],
      //   prev_vel_rad_s_[dxl_ids_ [i]-1],
      //   present_acc_rad_ss_[dxl_ids_ [i]-1]
      // );
    }

    int16_t present_current_raw = 0;
    // Read Present Velocity (length : 4 bytes) and Convert uint32 -> int32
    // When reading 2 byte data from AX / MX(1.0), use read2ByteTxRx() instead.
    dxl_comm_result = packetHandler->read2ByteTxRx(
      portHandler,
      dxl_ids_[i],
      ADDR_PRESENT_CURRENT,
      (uint16_t *)&present_current_raw,
      &dxl_error
    );

    if (dxl_comm_result != COMM_SUCCESS) {
      RCLCPP_INFO(
        this->get_logger(),
        "ID %d: %s",
        dxl_ids_[i],
        packetHandler->getTxRxResult(dxl_comm_result)
      );
    } else if (dxl_error != 0) {
      RCLCPP_INFO(
        this->get_logger(),
        "ID %d: %s",
        dxl_ids_[i],
        packetHandler->getRxPacketError(dxl_error)
      );
    } else{
      present_current_mA_[dxl_ids_[i]-1] = present_current_raw;

      // RCLCPP_INFO(
      //   this->get_logger(),
      //   "getCurrent : [ID:%d] -> [Current_mA:%d]",
      //   dxl_ids_[i],
      //   present_current_mA_[dxl_ids_[i]-1]
      // );
    }
  }

  std_msgs::msg::Float64MultiArray left_msg;
    left_msg.data.assign(
      openarm_msg.data.begin(),
      openarm_msg.data.begin() + 7
    );

  std_msgs::msg::Float64MultiArray right_msg;
    right_msg.data.assign(
      openarm_msg.data.begin() + 7,
      openarm_msg.data.begin() + 14
    );

  left_openarm_command_publisher_->publish(left_msg); 
  right_openarm_command_publisher_->publish(right_msg);

  motion_compensation();

  const auto cycle_end = std::chrono::steady_clock::now();
  const auto cycle_us =
  std::chrono::duration_cast<std::chrono::microseconds>(cycle_end - cycle_start).count();

  RCLCPP_INFO_THROTTLE(
    this->get_logger(),
    *this->get_clock(),
    1000,
    "cycle time: %ld us (%.3f ms), target period: %d ms",
    cycle_us,
    static_cast<double>(cycle_us) / 1000.0,
    read_period_ms_
);


}

ReadWriteWTNode::ReadWriteWTNode()
: Node("read_write_wt_node")
{
  RCLCPP_INFO(this->get_logger(), "Run read write node");

  this->declare_parameter("qos_depth", 10);
  int8_t qos_depth = 0;
  this->get_parameter("qos_depth", qos_depth);
  
  this->declare_parameter<int>("read_period_ms", read_period_ms_);
  this->get_parameter("read_period_ms", read_period_ms_);
  read_period_sec_ = static_cast<double>(read_period_ms_) / 1000.0;
 
  this->declare_parameter<int>("current_limit_ma", current_limit_mA_);
  this->get_parameter("current_limit_ma", current_limit_mA_);
  this->declare_parameter<double>("kt", kt_);
  this->get_parameter("kt", kt_);

  std::vector<int64_t> dxl_ids_param(dxl_ids_.begin(), dxl_ids_.end());
  this->declare_parameter<std::vector<int64_t>>("dxl_ids", dxl_ids_param);
  this->get_parameter("dxl_ids", dxl_ids_param);

  dxl_ids_.clear();
  for (const auto id : dxl_ids_param) {
    if (id < 0 || id > 252) {
      RCLCPP_WARN(
        this->get_logger(),
        "Invalid dxl_id: %ld. Skipping.",
        static_cast<long>(id)
      );
      continue;
    }

    dxl_ids_.push_back(static_cast<uint8_t>(id));
  }

  auto load_double_array_parameter =
  [this](const std::string & name, std::array<double, 18> & target) {
    std::vector<double> values(target.begin(), target.end());

    this->declare_parameter<std::vector<double>>(name, values);
    this->get_parameter(name, values);

    if (values.size() != target.size()) {
      RCLCPP_WARN(
        this->get_logger(),
        "Parameter '%s' size is %zu, expected %zu. Keeping default values.",
        name.c_str(),
        values.size(),
        target.size()
      );
      return;
    }
    std::copy(values.begin(), values.end(), target.begin());
  };

  load_double_array_parameter("j", J_);
  load_double_array_parameter("b", b_);
  load_double_array_parameter("ts", Ts_);
  load_double_array_parameter("tc", Tc_);
  load_double_array_parameter("vs", vs_);

  load_double_array_parameter("kp", Kp_);
  load_double_array_parameter("kd", Kd_);
  load_double_array_parameter("jd", Jd_);


  const auto QOS_RKL10V =
    rclcpp::QoS(rclcpp::KeepLast(qos_depth)).reliable().durability_volatile();

  set_position_subscriber_ =
    this->create_subscription<SetPosition>(
    "set_position",
    QOS_RKL10V,
    [this](const SetPosition::SharedPtr msg) -> void
    {
      uint8_t dxl_error = 0;

      // Position Value of X series is 4 byte data.
      // For AX & MX(1.0) use 2 byte data(uint16_t) for the Position Value.
      uint32_t goal_position = (unsigned int)msg->position;  // Convert int32 -> uint32

      // Write Goal Position (length : 4 bytes)
      // When writing 2 byte data to AX / MX(1.0), use write2ByteTxRx() instead.
      dxl_comm_result =
      packetHandler->write4ByteTxRx(
        portHandler,
        (uint8_t) msg->id,
        ADDR_GOAL_POSITION,
        goal_position,
        &dxl_error
      );

      int32_t current_position = 0;

      dxl_comm_result = packetHandler->read4ByteTxRx(
        portHandler,
        (uint8_t) msg->id,
        ADDR_PRESENT_POSITION,
        reinterpret_cast<uint32_t *>(&current_position),
        &dxl_error
      );

      if (dxl_comm_result != COMM_SUCCESS) {
        RCLCPP_INFO(this->get_logger(), "%s", packetHandler->getTxRxResult(dxl_comm_result));
      } else if (dxl_error != 0) {
        RCLCPP_INFO(this->get_logger(), "%s", packetHandler->getRxPacketError(dxl_error));
      } else {
        // RCLCPP_INFO(
        //   this->get_logger(),
        //   "Set [ID: %d] [Goal Position: %d] [Present Position: %d]",
        //   msg->id,
        //   msg->position,
        //   current_position
        // );
      }

    }
    );

  auto get_present_position =
    [this](
    const std::shared_ptr<GetPosition::Request> request,
    std::shared_ptr<GetPosition::Response> response) -> void
    {
      int present_position;
      // Read Present Position (length : 4 bytes) and Convert uint32 -> int32
      // When reading 2 byte data from AX / MX(1.0), use read2ByteTxRx() instead.
      dxl_comm_result = packetHandler->read4ByteTxRx(
        portHandler,
        (uint8_t) request->id,
        ADDR_PRESENT_POSITION,
        reinterpret_cast<uint32_t *>(&present_position),
        &dxl_error
      );

      // RCLCPP_INFO(
      //   this->get_logger(),
      //   "Get [ID: %d] [Present Position: %d]",
      //   request->id,
      //   present_position
      // );

      response->position = present_position;
    };

  get_position_server_ = create_service<GetPosition>("get_position", get_present_position);

  // 追加: Present Positionをpublishするpublisher
  // present_position_publisher_ =
    // this->create_publisher<std_msgs::msg::Int32>("present_position", 1);

  left_openarm_command_publisher_ =
    this->create_publisher<std_msgs::msg::Float64MultiArray>(
    "/left_forward_position_controller/commands", 1);

  right_openarm_command_publisher_ =
    this->create_publisher<std_msgs::msg::Float64MultiArray>(
    "/right_forward_position_controller/commands", 1);

  // 追加: 左グリッパのAction Clientを作成
  left_gripper_client_ =
    rclcpp_action::create_client<GripperCommand>(
      this,
      "/left_gripper_controller/gripper_cmd"
    );

  right_gripper_client_ =
    rclcpp_action::create_client<GripperCommand>(
      this,
      "/right_gripper_controller/gripper_cmd"
    );

  //Present Positionを読むtimer
  timer_ = this->create_wall_timer(
    std::chrono::milliseconds(read_period_ms_),
    std::bind(&ReadWriteWTNode::read_present_position, this)
);

}

ReadWriteWTNode::~ReadWriteWTNode()
{
}

/******************************************************************************/
/* Function                                                                   */
/******************************************************************************/
void setupDynamixel(uint8_t dxl_id)
{
  // Disable Torque of DYNAMIXEL
  dxl_comm_result = packetHandler->write1ByteTxRx(
    portHandler,
    dxl_id,
    ADDR_TORQUE_ENABLE,
    TORQUE_DISABLE,  /* Torque OFF */
    &dxl_error
  );

  /* Use Position Control Mode                */
  /* #define CURRENT_BASED_POSITION_CONTROL 5 */
  /* #define POSITION_CONTROL				        3 */
  /* #define VELOCITY_CONTROL				        1 */
  /* #define CURRENT_CONTROL					        0 */
  dxl_comm_result = packetHandler->write1ByteTxRx(
    portHandler,
    dxl_id,
    ADDR_OPERATING_MODE,
    CURRENT_CONTROL,
    &dxl_error
  );

  if (dxl_comm_result != COMM_SUCCESS) {
    RCLCPP_ERROR(rclcpp::get_logger("read_write_wt_node"), "Failed to set Position Control Mode.");
  } else {
    RCLCPP_INFO(rclcpp::get_logger("read_write_wt_node"), "Succeeded to set Position Control Mode.");
  }

  // Enable Torque of DYNAMIXEL
  dxl_comm_result = packetHandler->write1ByteTxRx(
    portHandler,
    dxl_id,
    ADDR_TORQUE_ENABLE,
    TORQUE_ENABLE,  /* Torque ON */
    &dxl_error
  );

  if (dxl_comm_result != COMM_SUCCESS) {
    RCLCPP_ERROR(rclcpp::get_logger("read_write_wt_node"), "Failed to enable torque.");
  } else {
    RCLCPP_INFO(rclcpp::get_logger("read_write_wt_node"), "Succeeded to enable torque.");
  }

}

int main(int argc, char * argv[])
{
  portHandler = dynamixel::PortHandler::getPortHandler(DEVICE_NAME);
  packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);

  // Open Serial Port
  dxl_comm_result = portHandler->openPort();
  if (dxl_comm_result == false) {
    RCLCPP_ERROR(rclcpp::get_logger("read_write_wt_node"), "Failed to open the port!");
    return -1;
  } else {
    RCLCPP_INFO(rclcpp::get_logger("read_write_wt_node"), "Succeeded to open the port.");
  }

  // Set the baudrate of the serial port (use DYNAMIXEL Baudrate)
  dxl_comm_result = portHandler->setBaudRate(BAUDRATE);
  if (dxl_comm_result == false) {
    RCLCPP_ERROR(rclcpp::get_logger("read_write_wt_node"), "Failed to set the baudrate!");
    return -1;
  } else {
    RCLCPP_INFO(rclcpp::get_logger("read_write_wt_node"), "Succeeded to set the baudrate.");
  }

  setupDynamixel(BROADCAST_ID);

  rclcpp::init(argc, argv);

  auto read_write_wt_node = std::make_shared<ReadWriteWTNode>();
  rclcpp::spin(read_write_wt_node);

  // Disable Torque of DYNAMIXEL
  packetHandler->write1ByteTxRx(
    portHandler,
    BROADCAST_ID,
    ADDR_TORQUE_ENABLE,
    TORQUE_DISABLE,
    &dxl_error
  );
  portHandler->closePort();
	
	rclcpp::shutdown();

  return 0;
}

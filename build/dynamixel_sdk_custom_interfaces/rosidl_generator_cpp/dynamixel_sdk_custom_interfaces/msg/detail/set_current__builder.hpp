// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from dynamixel_sdk_custom_interfaces:msg/SetCurrent.idl
// generated code does not contain a copyright notice

#ifndef DYNAMIXEL_SDK_CUSTOM_INTERFACES__MSG__DETAIL__SET_CURRENT__BUILDER_HPP_
#define DYNAMIXEL_SDK_CUSTOM_INTERFACES__MSG__DETAIL__SET_CURRENT__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "dynamixel_sdk_custom_interfaces/msg/detail/set_current__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace dynamixel_sdk_custom_interfaces
{

namespace msg
{

namespace builder
{

class Init_SetCurrent_current
{
public:
  explicit Init_SetCurrent_current(::dynamixel_sdk_custom_interfaces::msg::SetCurrent & msg)
  : msg_(msg)
  {}
  ::dynamixel_sdk_custom_interfaces::msg::SetCurrent current(::dynamixel_sdk_custom_interfaces::msg::SetCurrent::_current_type arg)
  {
    msg_.current = std::move(arg);
    return std::move(msg_);
  }

private:
  ::dynamixel_sdk_custom_interfaces::msg::SetCurrent msg_;
};

class Init_SetCurrent_id
{
public:
  Init_SetCurrent_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_SetCurrent_current id(::dynamixel_sdk_custom_interfaces::msg::SetCurrent::_id_type arg)
  {
    msg_.id = std::move(arg);
    return Init_SetCurrent_current(msg_);
  }

private:
  ::dynamixel_sdk_custom_interfaces::msg::SetCurrent msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::dynamixel_sdk_custom_interfaces::msg::SetCurrent>()
{
  return dynamixel_sdk_custom_interfaces::msg::builder::Init_SetCurrent_id();
}

}  // namespace dynamixel_sdk_custom_interfaces

#endif  // DYNAMIXEL_SDK_CUSTOM_INTERFACES__MSG__DETAIL__SET_CURRENT__BUILDER_HPP_

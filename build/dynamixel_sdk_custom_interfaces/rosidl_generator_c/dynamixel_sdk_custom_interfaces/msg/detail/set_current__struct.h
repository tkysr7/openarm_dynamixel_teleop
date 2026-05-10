// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from dynamixel_sdk_custom_interfaces:msg/SetCurrent.idl
// generated code does not contain a copyright notice

#ifndef DYNAMIXEL_SDK_CUSTOM_INTERFACES__MSG__DETAIL__SET_CURRENT__STRUCT_H_
#define DYNAMIXEL_SDK_CUSTOM_INTERFACES__MSG__DETAIL__SET_CURRENT__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/SetCurrent in the package dynamixel_sdk_custom_interfaces.
/**
  * Messages
 */
typedef struct dynamixel_sdk_custom_interfaces__msg__SetCurrent
{
  uint8_t id;
  int32_t current;
} dynamixel_sdk_custom_interfaces__msg__SetCurrent;

// Struct for a sequence of dynamixel_sdk_custom_interfaces__msg__SetCurrent.
typedef struct dynamixel_sdk_custom_interfaces__msg__SetCurrent__Sequence
{
  dynamixel_sdk_custom_interfaces__msg__SetCurrent * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} dynamixel_sdk_custom_interfaces__msg__SetCurrent__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // DYNAMIXEL_SDK_CUSTOM_INTERFACES__MSG__DETAIL__SET_CURRENT__STRUCT_H_

# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_dxl_wt_openarm_leader_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED dxl_wt_openarm_leader_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(dxl_wt_openarm_leader_FOUND FALSE)
  elseif(NOT dxl_wt_openarm_leader_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(dxl_wt_openarm_leader_FOUND FALSE)
  endif()
  return()
endif()
set(_dxl_wt_openarm_leader_CONFIG_INCLUDED TRUE)

# output package information
if(NOT dxl_wt_openarm_leader_FIND_QUIETLY)
  message(STATUS "Found dxl_wt_openarm_leader: 0.0.0 (${dxl_wt_openarm_leader_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'dxl_wt_openarm_leader' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${dxl_wt_openarm_leader_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(dxl_wt_openarm_leader_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${dxl_wt_openarm_leader_DIR}/${_extra}")
endforeach()

# openarm_dynamixel_teleop
This repository contains code for controlling Open-Arm using Dynamixel.
Note: forward position control is used, please launch Open-Arm using the command below.

```
ros2 launch openarm_bringup openarm.bimanual.launch.py arm_type:=v10 use_fake_hardware:=true robot_controller:='forward_position_controller'
```


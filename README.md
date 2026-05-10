# openarm_dynamixel_teleop
This repository contains code for controlling Open-Arm using Dynamixel as leader device.
Note: forward position control is used, please launch Open-Arm using the command below.
```
ros2 launch openarm_bringup openarm.bimanual.launch.py arm_type:=v10 use_fake_hardware:=true robot_controller:='forward_position_controller'
```


このリポジトリはOpen-Armをダイナミクセルで構成したリーダ機でユニラテラル操作するためのものです。
JointMappingができるように各軸のポジションがそのまま出力できるようになっています。
各軸には、摩擦/粘性/慣性補償が入っておりバックドライブトルクを補償しています。
また、インピーダンス制御により目標値に戻るように制御することができます。

Open-Armを起動する際は、上記にあるようにforward position controllerで起動してください。


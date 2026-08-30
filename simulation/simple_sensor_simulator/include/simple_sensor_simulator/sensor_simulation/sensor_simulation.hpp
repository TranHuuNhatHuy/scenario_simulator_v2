// Copyright 2015 TIER IV, Inc. All rights reserved.
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

#ifndef SIMPLE_SENSOR_SIMULATOR__SENSOR_SIMULATION__SENSOR_SIMULATION_HPP_
#define SIMPLE_SENSOR_SIMULATOR__SENSOR_SIMULATION__SENSOR_SIMULATION_HPP_

#include <simulation_interface/simulation_api_schema.pb.h>

#include <agnocast_wrapper/agnocast_wrapper.hpp>
#include <autoware_perception_msgs/msg/detected_objects.hpp>
#include <autoware_perception_msgs/msg/tracked_objects.hpp>
#include <iomanip>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <simple_sensor_simulator/sensor_simulation/detection_sensor/detection_sensor.hpp>
#include <simple_sensor_simulator/sensor_simulation/imu/imu_sensor.hpp>
#include <simple_sensor_simulator/sensor_simulation/lidar/lidar_sensor.hpp>
#include <simple_sensor_simulator/sensor_simulation/occupancy_grid/occupancy_grid_sensor.hpp>
#include <simple_sensor_simulator/sensor_simulation/traffic_lights/traffic_lights_detector.hpp>
#include <vector>

// This message will be deleted in the future
#if __has_include(<autoware_perception_msgs/msg/traffic_signal_array.hpp>)
#include <autoware_perception_msgs/msg/traffic_signal_array.hpp>
#endif

#include <architecture_type/architecture_type.hpp>

#if __has_include(<autoware_perception_msgs/msg/traffic_light_group_array.hpp>)
#include <autoware_perception_msgs/msg/traffic_light_group_array.hpp>
#endif

namespace simple_sensor_simulator
{
/*
   The sensor bridges publish on the same topics for awf/core and awf/universe: core's
   autoware_ground_filter reads /perception/obstacle_segmentation/pointcloud and its
   autoware_perception_objects_converter reads /perception/object_recognition/detection/objects,
   exactly as Universe does. What changes between architectures is only whether a consumer exists
   -- core has no occupancy grid consumer, for instance -- and publishing into an unsubscribed
   topic costs nothing.

   So these gates are architecture *validation*, not architecture *dispatch*. Upstream spells
   them as `find("awf/universe") != npos`, which silently accepts anything with that substring
   and rejects awf/core; they now ask the one predicate in common/architecture_type instead.
*/
class SensorSimulation
{
public:
  auto attachLidarSensor(
    const double current_simulation_time,
    const simulation_api_schema::LidarConfiguration & configuration, rclcpp::Node & node) -> void
  {
    if (common::architecture_type::isSupported(configuration.architecture_type())) {
      lidar_sensors_.push_back(std::make_unique<LidarSensor<sensor_msgs::msg::PointCloud2>>(
        current_simulation_time, configuration,
        agnocast_wrapper::create_publisher<sensor_msgs::msg::PointCloud2>(
          node, "/perception/obstacle_segmentation/pointcloud", 1)));
    } else {
      common::architecture_type::reject(configuration.architecture_type(), __func__);
    }
  }

  auto attachDetectionSensor(
    const double current_simulation_time,
    const simulation_api_schema::DetectionSensorConfiguration & configuration, rclcpp::Node & node)
    -> void
  {
    if (common::architecture_type::isSupported(configuration.architecture_type())) {
      using Message = autoware_perception_msgs::msg::DetectedObjects;
      using GroundTruthMessage = autoware_perception_msgs::msg::TrackedObjects;
      detection_sensors_.push_back(std::make_unique<DetectionSensor<Message>>(
        current_simulation_time, configuration,
        node.create_publisher<Message>("/perception/object_recognition/detection/objects", 1),
        node.create_publisher<GroundTruthMessage>(
          "/perception/object_recognition/ground_truth/objects", 1)));
    } else {
      common::architecture_type::reject(configuration.architecture_type(), __func__);
    }
  }

  auto attachOccupancyGridSensor(
    const double current_simulation_time,
    const simulation_api_schema::OccupancyGridSensorConfiguration & configuration,
    rclcpp::Node & node) -> void
  {
    if (common::architecture_type::isSupported(configuration.architecture_type())) {
      using Message = nav_msgs::msg::OccupancyGrid;
      occupancy_grid_sensors_.push_back(std::make_unique<OccupancyGridSensor<Message>>(
        current_simulation_time, configuration,
        node.create_publisher<Message>("/perception/occupancy_grid_map/map", 1)));
    } else {
      common::architecture_type::reject(configuration.architecture_type(), __func__);
    }
  }

  auto attachPseudoTrafficLightsDetector(
    const double /*current_simulation_time*/,
    const simulation_api_schema::PseudoTrafficLightDetectorConfiguration & configuration,
    rclcpp::Node & node) -> void
  {
    traffic_lights_detectors_.push_back(std::make_unique<traffic_lights::TrafficLightsDetector>(
      node, configuration.architecture_type()));
  }

  auto attachImuSensor(
    const double /*current_simulation_time*/,
    const simulation_api_schema::ImuSensorConfiguration & configuration, rclcpp::Node & node)
    -> void
  {
    imu_sensors_.push_back(std::make_unique<ImuSensor<sensor_msgs::msg::Imu>>(
      configuration, "/sensing/imu/imu_data", node));
  }

  auto updateSensorFrame(
    double current_simulation_time, const rclcpp::Time & current_ros_time,
    const std::vector<traffic_simulator_msgs::EntityStatus> &,
    const simulation_api_schema::UpdateTrafficLightsRequest &) -> void;

private:
  std::vector<std::unique_ptr<ImuSensorBase>> imu_sensors_;
  std::vector<std::unique_ptr<LidarSensorBase>> lidar_sensors_;
  std::vector<std::unique_ptr<DetectionSensorBase>> detection_sensors_;
  std::vector<std::unique_ptr<OccupancyGridSensorBase>> occupancy_grid_sensors_;
  std::vector<std::unique_ptr<traffic_lights::TrafficLightsDetector>> traffic_lights_detectors_;
};
}  // namespace simple_sensor_simulator

#endif  // SIMPLE_SENSOR_SIMULATOR__SENSOR_SIMULATION__SENSOR_SIMULATION_HPP_

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

#ifndef SIMPLE_SENSOR_SIMULATOR__SENSOR_SIMULATION__TRAFFIC_LIGHTS__TRAFFIC_LIGHTS_DETECTOR_HPP_
#define SIMPLE_SENSOR_SIMULATOR__SENSOR_SIMULATION__TRAFFIC_LIGHTS__TRAFFIC_LIGHTS_DETECTOR_HPP_

#include <architecture_type/architecture_type.hpp>
#include <rclcpp/rclcpp.hpp>
#include <simulation_interface/conversions.hpp>
#include <string>
#include <traffic_simulator/traffic_lights/traffic_light_publisher.hpp>

// This message will be deleted in the future
#if __has_include(<autoware_perception_msgs/msg/traffic_signal_array.hpp>)
#include <autoware_perception_msgs/msg/traffic_signal_array.hpp>
#endif

#if __has_include(<autoware_perception_msgs/msg/traffic_light_group_array.hpp>)
#include <autoware_perception_msgs/msg/traffic_light_group_array.hpp>
#endif

namespace simple_sensor_simulator
{
namespace traffic_lights
{
/** @brief Implements traffic lights detector mechanism simulation
 * Currently it only allows to set traffic lights state and publish them on predefined topic
 * Future implementations might, for example, publish only traffic lights that are in a specific FoV
 * of a camera sensor Further refactoring would be required, however, to achieve this.
 */
class TrafficLightsDetector
{
public:
  template <typename NodeType>
  explicit TrafficLightsDetector(NodeType & node, const std::string & architecture_type)
  : publisher_ptr_(makePublisher(node, architecture_type))
  {
  }

  auto updateFrame(
    const rclcpp::Time & current_ros_time,
    const simulation_api_schema::UpdateTrafficLightsRequest & request) -> void
  {
    publisher_ptr_->publish(current_ros_time, request);
  }

private:
  template <typename NodeType>
  auto makePublisher(NodeType & node, const std::string & architecture_type)
    -> std::unique_ptr<traffic_simulator::TrafficLightPublisherBase>
  {
    /*
       TrafficLightsDetector in SimpleSensorSimulator publishes using architecture-dependent topics:
       "/perception/traffic_light_recognition/internal/traffic_signals" for >= "awf/universe/20240605"
       "/perception/traffic_light_recognition/internal/traffic_signals" for == "awf/universe/20230906"

       V2ITrafficLights in TrafficSimulator publishes using architecture-independent topics ("awf/universe..."):
       "/v2x/traffic_signals" and "/perception/traffic_light_recognition/external/traffic_signals"
    */
#if __has_include(<autoware_perception_msgs/msg/traffic_light_group_array.hpp>)
    if (common::architecture_type::isCore(architecture_type)) {
      /*
         autoware_core has no traffic light arbiter: nothing merges an "internal" (recognised)
         signal stream with an "external" (V2I) one, and its component interface manifest
         declares exactly one traffic light interface,
         /perception/traffic_light_recognition/traffic_signals. Publishing the simulated
         recognition result straight onto that topic is therefore the only wiring that reaches a
         consumer -- publishing to .../internal/... as Universe does would reach nothing.
      */
      using Message = autoware_perception_msgs::msg::TrafficLightGroupArray;
      return std::make_unique<traffic_simulator::TrafficLightPublisher<Message>>(
        &node, "/perception/traffic_light_recognition/traffic_signals");
    } else if (architecture_type == "awf/universe") {
#else
    if (architecture_type == "awf/universe") {
#endif
      throw common::SemanticError(
        "This version of scenario_simulator_v2 does not support ", std::quoted(architecture_type),
        " as ", std::quoted("architecture_type"), ". Please use older version.");
#if __has_include(<autoware_perception_msgs/msg/traffic_signal_array.hpp>)
    } else if (architecture_type == "awf/universe/20230906") {
      using Message = autoware_perception_msgs::msg::TrafficSignalArray;
      return std::make_unique<traffic_simulator::TrafficLightPublisher<Message>>(
        &node, "/perception/traffic_light_recognition/internal/traffic_signals");
#endif
#if __has_include(<autoware_perception_msgs/msg/traffic_light_group_array.hpp>)
    } else if (architecture_type >= "awf/universe/20240605") {
      using Message = autoware_perception_msgs::msg::TrafficLightGroupArray;
      return std::make_unique<traffic_simulator::TrafficLightPublisher<Message>>(
        &node, "/perception/traffic_light_recognition/internal/traffic_signals");
#endif
    } else {
      common::architecture_type::reject(architecture_type, "traffic light detection");
    }
  }

  const std::unique_ptr<traffic_simulator::TrafficLightPublisherBase> publisher_ptr_;
};
}  // namespace traffic_lights
}  // namespace simple_sensor_simulator

#endif  // SIMPLE_SENSOR_SIMULATOR__SENSOR_SIMULATION__TRAFFIC_LIGHTS__TRAFFIC_LIGHTS_DETECTOR_HPP_

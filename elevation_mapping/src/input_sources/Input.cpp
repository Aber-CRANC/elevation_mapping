/*
 *  Input.cpp
 *
 *  Created on: Oct 06, 2020
 *  Author: Magnus Gärtner
 *  Institute: ETH Zurich, ANYbotics
 */

#include "elevation_mapping/input_sources/Input.hpp"

#include "elevation_mapping/sensor_processors/LaserSensorProcessor.hpp"
#include "elevation_mapping/sensor_processors/PerfectSensorProcessor.hpp"
#include "elevation_mapping/sensor_processors/StereoSensorProcessor.hpp"
#include "elevation_mapping/sensor_processors/StructuredLightSensorProcessor.hpp"

namespace elevation_mapping {

Input::Input(rclcpp::Node::SharedPtr node) : node_(node), queueSize_(0), publishOnUpdate_(true) {}

bool Input::configure(std::string name, const XmlRpc::XmlRpcValue& parameters,
                      const SensorProcessorBase::GeneralParameters& generalSensorProcessorParameters) {
  // Configuration Guards.
  if (parameters.getType() != XmlRpc::XmlRpcValue::TypeStruct) {
    RCLCPP_ERROR(
        "Input source must be specified as map, but is "
        "XmlRpcType:%d.",
        parameters.getType());
    return false;
  }

  // Check that parameters exist and has an appropriate type.
  using nameAndType = std::pair<std::string, XmlRpc::XmlRpcValue::Type>;
  for (const nameAndType& member : std::vector<nameAndType>{{"type", XmlRpc::XmlRpcValue::TypeString},
                                                            {"topic", XmlRpc::XmlRpcValue::TypeString},
                                                            {"queue_size", XmlRpc::XmlRpcValue::TypeInt},
                                                            {"publish_on_update", XmlRpc::XmlRpcValue::TypeBoolean},
                                                            {"sensor_processor", XmlRpc::XmlRpcValue::TypeStruct}}) {
    if (!parameters.hasMember(member.first)) {
      RCLCPP_ERROR(node_->get_logger(), "Could not configure input source %s because no %s was given.", name.c_str(), member.first.c_str());
      return false;
    }
    if (parameters[member.first].getType() != member.second) {
      RCLCPP_ERROR(
          "Could not configure input source %s because member %s has the "
          "wrong type.",
          name.c_str(), member.first.c_str());
      return false;
    }
  }

  name_ = name;
  dataType_ = static_cast<std::string>(parameters["type"]);
  topic_ = static_cast<std::string>(parameters["topic"]);
  const int& queueSize = static_cast<int>(parameters["queue_size"]);
  if (queueSize >= 0) {
    queueSize_ = static_cast<unsigned int>(queueSize);
  } else {
    RCLCPP_ERROR(node_->get_logger(), "The specified queue_size is negative.");
    return false;
  }
  publishOnUpdate_ = static_cast<bool>(parameters["publish_on_update"]);

  // SensorProcessor
  if (!configureSensorProcessor(name, parameters, generalSensorProcessorParameters)) {
    return false;
  }

  RCLCPP_DEBUG(node_->get_logger(), "Configured %s:%s @ %s (publishing_on_update: %s), using %s to process data.\n", dataType_.c_str(), name_.c_str(),
            node_->resolveName(topic_).c_str(), publishOnUpdate_ ? "true" : "false",
            static_cast<std::string>(parameters["sensor_processor"]["type"]).c_str());
  return true;
}

std::string Input::getSubscribedTopic() const {
  return node_->resolveName(topic_);
}

bool Input::configureSensorProcessor(std::string name, const XmlRpc::XmlRpcValue& parameters,
                                     const SensorProcessorBase::GeneralParameters& generalSensorProcessorParameters) {
  if (!parameters["sensor_processor"].hasMember("type")) {
    RCLCPP_ERROR(node_->get_logger(), "Could not configure sensor processor of input source %s because no type was given.", name.c_str());
    return false;
  }
  if (parameters["sensor_processor"]["type"].getType() != XmlRpc::XmlRpcValue::TypeString) {
    RCLCPP_ERROR(
        "Could not configure sensor processor of input source %s because the member 'type' has the "
        "wrong type.",
        name.c_str());
    return false;
  }
  std::string sensorType = static_cast<std::string>(parameters["sensor_processor"]["type"]);
  if (sensorType == "structured_light") {
    sensorProcessor_.reset(new StructuredLightSensorProcessor(node_, generalSensorProcessorParameters));
  } else if (sensorType == "stereo") {
    sensorProcessor_.reset(new StereoSensorProcessor(node_, generalSensorProcessorParameters));
  } else if (sensorType == "laser") {
    sensorProcessor_.reset(new LaserSensorProcessor(node_, generalSensorProcessorParameters));
  } else if (sensorType == "perfect") {
    sensorProcessor_.reset(new PerfectSensorProcessor(node_, generalSensorProcessorParameters));
  } else {
    RCLCPP_ERROR(node_->get_logger(), "The sensor type %s is not available.", sensorType.c_str());
    return false;
  }

  return sensorProcessor_->readParameters();
}

}  // namespace elevation_mapping

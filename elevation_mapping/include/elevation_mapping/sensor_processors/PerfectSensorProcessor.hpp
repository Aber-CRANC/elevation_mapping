/*
 * PerfectSensorProcessor.hpp
 *
 *  Created on: Sep 28, 2014
 *      Author: Péter Fankhauser
 *   Institute: ETH Zurich, ANYbotics
 */

#pragma once

#include "elevation_mapping/PointXYZRGBConfidenceRatio.hpp"
#include "elevation_mapping/sensor_processors/SensorProcessorBase.hpp"

namespace elevation_mapping {

/*!
 * Sensor processor for laser range sensors.
 */
class PerfectSensorProcessor : public SensorProcessorBase {
 public:
  /*!
   * Constructor.
   * @param node the shared pointer to the ROS node node handle.
   */
  PerfectSensorProcessor(rclcpp::Node::SharedPtr node, const SensorProcessorBase::GeneralParameters& generalParameters);

  /*!
   * Destructor.
   */
  ~PerfectSensorProcessor() override;

 private:
  /*!
   * Reads and verifies the parameters.
   * @param[in] processorNamespace parameter namespace to read parameters from
   * @return true if successful.
   */
  bool readParameters(std::string processorNamespace) override;

  /*!
   * Computes the elevation map height variances for each point in a point cloud with the
   * sensor model and the robot pose covariance.
   * @param[in] pointCloud the point cloud for which the variances are computed.
   * @param[in] robotPoseCovariance the robot pose covariance matrix.
   * @param[out] variances the elevation map height variances.
   * @return true if successful.
   */
  bool computeVariances(const PointCloudType::ConstPtr pointCloud, const Eigen::Matrix<double, 6, 6>& robotPoseCovariance,
                        Eigen::VectorXf& variances) override;
};

} /* namespace elevation_mapping */

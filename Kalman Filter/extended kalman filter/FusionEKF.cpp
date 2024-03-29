#include "FusionEKF.h"
#include <iostream>
#include "Eigen/Dense"
#include "tools.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::cout;
using std::endl;
using std::vector;

/**
 * Constructor.
 */
FusionEKF::FusionEKF() {
  is_initialized_ = false;

  previous_timestamp_ = 0;

  // initializing matrices
  R_laser_ = MatrixXd(2, 2);
  R_radar_ = MatrixXd(3, 3);
  H_laser_ = MatrixXd(2, 4);
  Hj_ = MatrixXd(3, 4);

  //measurement covariance matrix - laser
  R_laser_ << 0.0225, 0,
              0, 0.0225;

  //measurement covariance matrix - radar
  R_radar_ << 0.09, 0, 0,
              0, 0.0009, 0,
              0, 0, 0.09;

  /**
   * TODO: Finish initializing the FusionEKF.
   * TODO: Set the process and measurement noises
   */

  ekf_.x_ = VectorXd(4);

  // State covariance matrix P
  ekf_.P_ = MatrixXd(4, 4);
  ekf_.P_ << 0.1, 0, 0, 0,
             0, 0.1, 0, 0,
             0, 0, 9, 0,
             0, 0, 0, 20;

  // Measurement matrix
  H_laser_ << 1, 0, 0, 0,
              0, 1, 0, 0;
  

  // Initial transiiton matrix F_
  ekf_.F_ = MatrixXd(4, 4);
  ekf_.F_ << 1, 0, 1, 0,
             0, 1, 0, 1,
             0, 0, 1, 0,
             0, 0, 0, 1;

  // Set acceleration noise
  noise_ax = 9;
	noise_ay = 9;

}

/**
 * Destructor.
 */
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) {
  
  const double  f_PI=3.14159265358979f;
  bool sensor = true;

  if (!is_initialized_) {
  
    // first measurement
    cout << "EKF: " << endl;
    ekf_.x_ = VectorXd(4);
    ekf_.x_ << 1, 1, 1, 1;

    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
      // Convert radar from polar to cartesian coordinates  and initialize state.
      double fRho;
      double fTheta;
      double fRho_d;
      double fpx_;
      double fpy_;
      double fvx_;
      double fvy_;

      fRho = measurement_pack.raw_measurements_[0];
      fTheta = measurement_pack.raw_measurements_[1];
      fRho_d = measurement_pack.raw_measurements_[2];

      if(fTheta <(-1*f_PI)){
        while(fTheta < f_PI){
          fTheta = fTheta + (2*f_PI);
        }
      }
      if(fTheta > f_PI){
        while(fTheta > -1*f_PI){
          fTheta = fTheta - (2*f_PI);
        }
      }

      fpx_ = fRho *sin(fTheta);
      fpy_ = fRho *cos(fTheta);
      fvx_ = fRho_d *sin(fTheta);
      fvy_ = fRho_d *cos(fTheta);

      ekf_.x_ << fpx_, fpy_, fvx_, fvy_;
      previous_timestamp_ = measurement_pack.timestamp_;

    }
    else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
      // Initialize state.
      ekf_.x_ << measurement_pack.raw_measurements_[0], measurement_pack.raw_measurements_[1], 0, 0;
      previous_timestamp_ = measurement_pack.timestamp_;

    }

    // done initializing
    is_initialized_ = true;
    return;
  }

  double dt_ = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0;

  previous_timestamp_ = measurement_pack.timestamp_;

  //  State transition matrix
  ekf_.F_ << 1, 0, dt_, 0,
             0, 1, 0, dt_,
             0, 0, 1, 0,
             0, 0, 0, 1;

  // Process covariance matrix
  ekf_.Q_ = MatrixXd(4, 4);
  double dt4 = pow(dt_, 4)/4.;
  double dt3 = pow(dt_, 3)/2.;
  double dt2 = pow(dt_, 2)/4.;

  ekf_.Q_ << dt4*noise_ax, 0, dt3*noise_ax, 0,
			  0, dt4*noise_ay, 0, dt3*noise_ay,
			  dt3*noise_ax, 0, dt2*noise_ax, 0,
			  0, dt3*noise_ay, 0, dt2*noise_ay;


  /**
   * Prediction
   */

  ekf_.Predict();

  /**
   * Update
   */


  if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
    //Radar updates
    ekf_.R_ = MatrixXd(3, 3);
    ekf_.R_ = R_radar_;
    ekf_.H_ = MatrixXd(3, 4);

    ekf_.H_ = tools.CalculateJacobian(ekf_.x_);
    ekf_.UpdateEKF(measurement_pack.raw_measurements_);


  } else {
    //Laser updates
    ekf_.R_ = MatrixXd(2, 2);
    ekf_.H_ = MatrixXd(2, 2);

    ekf_.H_ = H_laser_;
    ekf_.R_ = R_laser_;

    ekf_.Update(measurement_pack.raw_measurements_);

  }

  // print the output
  cout << "x_ = " << ekf_.x_ << endl;
  cout << "P_ = " << ekf_.P_ << endl;
}

/**
 * \file test_bench_8_motors.cpp
 * \brief The hardware wrapper of the the test bench with 8 blmc motors
 * \author Maximilien Naveau
 * \date 2018
 *
 * This file defines the Finger class.
 */

#include "blmc_robots/finger/finger.hpp"

#include <limits>

namespace blmc_robots
{

Finger::Finger()
{
  max_current_ = 1.0 ;
  max_range_ = 2.0;

  motor_positions_.setZero();
  motor_velocities_.setZero();
  motor_currents_.setZero();
  motor_encoder_indexes_.setZero();
  slider_positions_.setZero();
  target_currents.setZero();

}


void Finger::initialize()
{
  // not needed for system other than xenomai
  osi::initialize_realtime_printing();

  // initialize the communication with the can cards
  can_bus0_ = std::make_shared<blmc_drivers::CanBus>("can0");
  can_bus1_ = std::make_shared<blmc_drivers::CanBus>("can1");

  // get all informatino about the control cards
  board0_ = std::make_shared<blmc_drivers::CanBusMotorBoard>(can_bus0_);
  board1_ = std::make_shared<blmc_drivers::CanBusMotorBoard>(can_bus1_);

  // get the drivers for the motors and the sliders
  // two individual motors on individual leg style mounting on the left of the
  // table
  motors_[MotorIndexing::base]  = std::make_shared<blmc_drivers::SafeMotor>   (board0_, 0, max_current_);
  board0_motor1_  = std::make_shared<blmc_drivers::SafeMotor>   (board0_, 1, max_current_);
  board0_slider0_ = std::make_shared<blmc_drivers::AnalogSensor>(board0_, 0);
  board0_slider1_ = std::make_shared<blmc_drivers::AnalogSensor>(board0_, 1);

  // two individual motors with a wheel on top
  board1_motor0_  = std::make_shared<blmc_drivers::SafeMotor>   (board1_, 0, max_current_);
  board1_slider0_ = std::make_shared<blmc_drivers::AnalogSensor>(board1_, 0);

  Timer<>::sleep_ms(10);
}

void Finger::acquire_sensors()
{
  // acquire the motors positions
  motor_positions_(0) = motors_[MotorIndexing::base]->get_measurement(mi::position)->newest_element();
  motor_positions_(1) = board0_motor1_->get_measurement(mi::position)->newest_element();
  motor_positions_(2) = board1_motor0_->get_measurement(mi::position)->newest_element();

  // acquire the motors velocities
  motor_velocities_(0) = motors_[MotorIndexing::base]->get_measurement(mi::velocity)->newest_element();
  motor_velocities_(1) = board0_motor1_->get_measurement(mi::velocity)->newest_element();
  motor_velocities_(2) = board1_motor0_->get_measurement(mi::velocity)->newest_element();

  // acquire the motors current
  motor_currents_(0) = motors_[MotorIndexing::base]->get_measurement(mi::current)->newest_element();
  motor_currents_(1) = board0_motor1_->get_measurement(mi::current)->newest_element();
  motor_currents_(2) = board1_motor0_->get_measurement(mi::current)->newest_element();


  // acquire the motors encoder index
  motor_encoder_indexes_(0) =
      motors_[MotorIndexing::base]->get_measurement(mi::encoder_index)->length() > 0 ?
        motors_[MotorIndexing::base]->get_measurement(mi::encoder_index)->newest_element():
        std::numeric_limits<double>::quiet_NaN();

  motor_encoder_indexes_(1) =
      board0_motor1_->get_measurement(mi::encoder_index)->length() > 0 ?
        board0_motor1_->get_measurement(mi::encoder_index)->newest_element():
        std::numeric_limits<double>::quiet_NaN();

  motor_encoder_indexes_(2) =
      board1_motor0_->get_measurement(mi::encoder_index)->length() > 0 ?
        board1_motor0_->get_measurement(mi::encoder_index)->newest_element():
        std::numeric_limits<double>::quiet_NaN();


  // acquire the slider positions
  slider_positions_(0) = board0_slider0_->get_measurement()->newest_element();
  slider_positions_(1) = board0_slider1_->get_measurement()->newest_element();
  slider_positions_(2) = board1_slider0_->get_measurement()->newest_element();
}

void Finger::send_target_current(
    const Eigen::Vector3d target_currents)
{
  motors_[MotorIndexing::base]->set_current_target(target_currents(0));
  board0_motor1_->set_current_target(target_currents(1));
  board1_motor0_->set_current_target(target_currents(2));


  motors_[MotorIndexing::base]->send_if_input_changed();
  board0_motor1_->send_if_input_changed();
  board1_motor0_->send_if_input_changed();
}

} // namespace blmc_robots

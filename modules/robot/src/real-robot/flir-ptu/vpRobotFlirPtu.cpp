/****************************************************************************
 *
 * ViSP, open source Visual Servoing Platform software.
 * Copyright (C) 2005 - 2019 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://visp.inria.fr for more information.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Interface for Flir Ptu Cpi robot.
 *
 * Authors:
 * Fabien Spindler
 *
 *****************************************************************************/

#include <visp3/core/vpConfig.h>

#ifdef VISP_HAVE_FLIR_PTU_SDK

extern "C" {
#include <cpi.h>
}

#include <visp3/robot/vpRobotException.h>

/*!
  \file vpRobotFlirPtu.cpp
  \brief Interface for Flir Ptu Cpi robot.
*/

#include <visp3/core/vpHomogeneousMatrix.h>
#include <visp3/robot/vpRobotFlirPtu.h>

/*!
  Basic initialization.
 */
void vpRobotFlirPtu::init()
{
  // If you want to control the robot in Cartesian in a tool frame, set the corresponding transformation in m_eMc
  // that is set to identity by default in the constructor.

  maxRotationVelocity = maxRotationVelocityDefault;
  maxTranslationVelocity = maxTranslationVelocityDefault;

  // Set here the robot degrees of freedom number
  nDof = 2; // Flir Ptu has 2 dof
}

/*!
  Default constructor.
 */
vpRobotFlirPtu::vpRobotFlirPtu()
  : m_eMc(), m_cer(NULL), m_status(0),
    m_pos_max(2), m_pos_min(2), m_vel_max_tics(2), m_res(2),
    m_connected(false), m_njoints(2), m_positioning_velocity(20.)
{
  init();
}

/*!
  Destructor.
 */
vpRobotFlirPtu::~vpRobotFlirPtu()
{
  stopMotion();
  disconnect();
}

/*
  At least one of these function has to be implemented to control the robot with a
  Cartesian velocity:
  - get_eJe()
  - get_fJe()
*/

/*!
  Get the robot Jacobian expressed in the end-effector frame.
  \param[out] eJe : End-effector frame Jacobian.
*/
void vpRobotFlirPtu::get_eJe(vpMatrix &eJe)
{
  (void)eJe;
  std::cout << "Not implemented ! " << std::endl;
}

/*!
  Get the robot Jacobian expressed in the robot reference frame.
  \param[out] fJe : Base (or reference) frame Jacobian.
*/
void vpRobotFlirPtu::get_fJe(vpMatrix &fJe)
{
  (void)fJe;
  std::cout << "Not implemented ! " << std::endl;
}

/*
  At least one of these function has to be implemented to control the robot:
  - setCartVelocity()
  - setJointVelocity()
*/

/*!
  Send to the controller a 6-dim velocity skew vector expressed in a Cartesian frame.
  \param[in] frame : Cartesian control frame (either tool frame or end-effector) in which the velocity \e v is expressed.
  Units are m/s for translation and rad/s for rotation velocities.
  \param[in] v : 6-dim vector that contains the 6 components of the velocity skew to send to the robot.
  Units are m/s and rad/s.
*/
void vpRobotFlirPtu::setCartVelocity(const vpRobot::vpControlFrameType frame, const vpColVector &v)
{
  if (v.size() != 6) {
    throw(vpException(vpException::fatalError,
                      "Cannot send a velocity-skew vector in tool frame that is not 6-dim (%d)", v.size()));
  }

  vpColVector v_e; // This is the velocity that the robot is able to apply in the end-effector frame
  switch (frame) {
  case vpRobot::TOOL_FRAME: {
    // We have to transform the requested velocity in the end-effector frame.
    // Knowing that the constant transformation between the tool frame and the end-effector frame obtained
    // by extrinsic calibration is set in m_eMc we can compute the velocity twist matrix eVc that transform
    // a velocity skew from tool (or camera) frame into end-effector frame
    vpVelocityTwistMatrix eVc(m_eMc);
    v_e = eVc * v;
    break;
  }

  case vpRobot::END_EFFECTOR_FRAME:
  case vpRobot::REFERENCE_FRAME: {
    v_e = v;
    break;
  }
  case vpRobot::JOINT_STATE:
  case vpRobot::MIXT_FRAME:
    // Out of the scope
    break;
  }

  // Implement your stuff here to send the end-effector velocity skew v_e
  // - If the SDK allows to send cartesian velocities in the end-effector, it's done. Just wrap data in v_e
  // - If the SDK allows to send cartesian velocities in the reference (or base) frame you have to implement
  //   the robot Jacobian in set_fJe() and call:
  //   vpColVector v = get_fJe().inverse() * v_e;
  //   At this point you have to wrap data in v that is the 6-dim velocity to apply to the robot
  // - If the SDK allows to send only joint velocities you have to implement the robot Jacobian in set_eJe()
  //   and call:
  //   vpColVector qdot = get_eJe().inverse() * v_e;
  //   setJointVelocity(qdot);
  // - If the SDK allows to send only a cartesian position trajectory of the end-effector position in the base frame
  //   called fMe (for fix frame to end-effector homogeneous transformation) you can transform the cartesian
  //   velocity in the end-effector into a displacement eMe using the exponetial map:
  //   double delta_t = 0.010; // in sec
  //   vpHomogenesousMatrix eMe = vpExponentialMap::direct(v_e, delta_t);
  //   vpHomogenesousMatrix fMe = getPosition(vpRobot::REFERENCE_FRAME);
  //   the new position to reach is than given by fMe * eMe
  //   vpColVector fpe(vpPoseVector(fMe * eMe));
  //   setPosition(vpRobot::REFERENCE_FRAME, fpe);

  std::cout << "Not implemented ! " << std::endl;
  std::cout << "To implement me you need : " << std::endl;
  std::cout << "\t to known the robot jacobian expressed in ";
  std::cout << "the end-effector frame (eJe) " << std::endl;
  std::cout << "\t the frame transformation  between tool or camera frame ";
  std::cout << "and end-effector frame (cMe)" << std::endl;
}

/*!
  Send a joint velocity to the controller.
  \param[in] qdot : Joint velocities vector. Units are rad/s for a robot arm.
 */
void vpRobotFlirPtu::setJointVelocity(const vpColVector &qdot)
{
  if (! m_connected) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU is not connected."));
  }

  std::vector<int> vel(2);

  for (size_t i = 0; i < 2; i++) {
    vel[i]  = static_cast<int>(vpMath::deg(qdot[i]) / m_res[i]);
    if (std::fabs(vel[i]) > m_vel_max_tics[i]) {
      disconnect();
      throw(vpException(vpException::fatalError, "Cannot set joint %d velocity %f (deg/s). Out of limits [-%f, %f].",
                        i, vpMath::deg(qdot[i]), -m_vel_max_tics[i] * m_res[i], m_vel_max_tics[i] * m_res[i]));
    }
  }

  if(cpi_ptcmd(m_cer, &m_status, OP_PAN_DESIRED_SPEED_SET, vel[0]) ||
     cpi_ptcmd(m_cer, &m_status, OP_TILT_DESIRED_SPEED_SET, vel[1])) {
    throw(vpException(vpException::fatalError, "Unable to set velocity."));
  }
}

/*!
  Send to the controller a velocity in a given frame.
  \param[in] frame : Control frame in which the velocity \e vel is expressed.
  Velocities could be joint velocities, or cartesian velocities. Units are m/s for translation and
  rad/s for rotation velocities.
  \param[in] vel : Vector that contains the velocity to apply to the robot.
 */
void vpRobotFlirPtu::setVelocity(const vpRobot::vpControlFrameType frame, const vpColVector &vel)
{
  if (vpRobot::STATE_VELOCITY_CONTROL != getRobotState()) {
    throw vpRobotException(vpRobotException::wrongStateError,
                           "Cannot send a velocity to the robot. "
                           "Call setRobotState(vpRobot::STATE_VELOCITY_CONTROL) once before "
                           "entering your control loop.");
  }

  vpColVector vel_sat(6);

  // Velocity saturation
  switch (frame) {
  // Saturation in cartesian space
  case vpRobot::TOOL_FRAME:
  case vpRobot::REFERENCE_FRAME:
  case vpRobot::END_EFFECTOR_FRAME:
  case vpRobot::MIXT_FRAME: {
    if (vel.size() != 6) {
      throw(vpException(vpException::dimensionError,
                        "Cannot apply a Cartesian velocity that is not a 6-dim vector (%d)", vel.size()));
    }
    vpColVector vel_max(6);

    for (unsigned int i = 0; i < 3; i++)
      vel_max[i] = getMaxTranslationVelocity();
    for (unsigned int i = 3; i < 6; i++)
      vel_max[i] = getMaxRotationVelocity();

    vel_sat = vpRobot::saturateVelocities(vel, vel_max, true);

    setCartVelocity(frame, vel_sat);
    break;
  }
    // Saturation in joint space
  case vpRobot::JOINT_STATE: {
    if (vel.size() != static_cast<size_t>(nDof)) {
      throw(vpException(vpException::dimensionError, "Cannot apply a joint velocity that is not a %d-dim vector (%d)",
                        nDof, vel.size()));
    }
    vpColVector vel_max(vel.size());

    // Since the robot has only rotation axis all the joint max velocities are set to getMaxRotationVelocity()
    vel_max = getMaxRotationVelocity();

    vel_sat = vpRobot::saturateVelocities(vel, vel_max, true);

    setJointVelocity(vel_sat);
  }
  }
}

/*
  THESE FUNCTIONS ARE NOT MENDATORY BUT ARE USUALLY USEFUL
*/

/*!
  Get robot joint positions.
  \param[in] q : Joint position as a 2-dim vector [pan, tilt] with values in radians.
 */
void vpRobotFlirPtu::getJointPosition(vpColVector &q)
{
  if (! m_connected) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU is not connected."));
  }

  std::vector<int> pos(2);

  if(cpi_ptcmd(m_cer, &m_status, OP_PAN_CURRENT_POS_GET, &pos[0])) {
    disconnect();
    throw(vpException(vpException::fatalError, "Unable to query pan position."));
  }
  if(cpi_ptcmd(m_cer, &m_status, OP_TILT_CURRENT_POS_GET, &pos[1])) {
    disconnect();
    throw(vpException(vpException::fatalError, "Unable to query pan position."));
  }

  q.resize(2);
  for (size_t i = 0; i < 2; i++) {
    q[i] = vpMath::rad(m_res[i] * pos[i]);
  }
}

/*!
  Get robot position.
  \param[in] frame : Considered cartesian frame or joint state.
  \param[out] q : Position of the arm.
 */
void vpRobotFlirPtu::getPosition(const vpRobot::vpControlFrameType frame, vpColVector &q)
{
  if (frame == JOINT_STATE) {
    getJointPosition(q);
  } else {
    std::cout << "Not implemented ! " << std::endl;
  }
}

/*!
  Set a position to reach.
  \param[in] frame : Considered cartesian frame or joint state.
  \param[in] q : Position to reach.
 */
void vpRobotFlirPtu::setPosition(const vpRobot::vpControlFrameType frame, const vpColVector &q)
{
  if (frame != vpRobot::JOINT_STATE) {
    std::cout << "FLIR PTU positioning is not implemented in this frame" << std::endl;
    return;
  }

  if (q.size() != 2) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU has only %d joints. Cannot set a position that is %d-dim.", m_njoints, q.size()));

  }
  if (! m_connected) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU is not connected."));
  }

  double vmin = 0.01, vmax = 100.;
  if (m_positioning_velocity < vmin || m_positioning_velocity > vmax) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU Positioning velocity %f is not in range [%f, %f]", vmin, vmax));
  }

  std::vector<int> pos(2);

  for (size_t i = 0; i < 2; i++) {
    if (q[i] < m_pos_min[i] || q[i] > m_pos_max[i]) {
      disconnect();
      throw(vpException(vpException::fatalError, "Cannot set joint %d position %f (deg). Out of limits [%f, %f].",
                        i, vpMath::deg(q[i]), vpMath::deg(m_pos_min[i]), vpMath::deg(m_pos_max[i])));
    }

    pos[i]  = static_cast<int>(vpMath::deg(q[i] / m_res[i]));
  }

  // Set desired speed wrt max pan/tilt speed
  if(cpi_ptcmd(m_cer, &m_status, OP_PAN_DESIRED_SPEED_SET, (int)(m_vel_max_tics[0] * m_positioning_velocity / 100.)) ||
          cpi_ptcmd(m_cer, &m_status, OP_TILT_DESIRED_SPEED_SET, (int)(m_vel_max_tics[1] * m_positioning_velocity / 100.))){
    disconnect();
    throw(vpException(vpException::fatalError, "Setting FLIR pan/tilt positioning velocity failed"));
  }

  if(cpi_ptcmd(m_cer, &m_status, OP_PAN_DESIRED_POS_SET, pos[0]) ||
     cpi_ptcmd(m_cer, &m_status, OP_TILT_DESIRED_POS_SET, pos[1])){
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU failed to go to position %d, %d (deg).", pos[0] * m_res[0], pos[1] * m_res[1]));
  }
  if(cpi_block_until(m_cer, NULL, NULL, OP_PAN_CURRENT_POS_GET, pos[0]) ||
          cpi_block_until(m_cer, NULL, NULL, OP_TILT_CURRENT_POS_GET, pos[1])){
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU failed to wait until position %d, %d reached (deg)", pos[0] * m_res[0], pos[1] * m_res[1]));
  }
}

/*!
  Get a displacement.
  \param[in] frame : Considered cartesian frame or joint state.
  \param[out] q : Displacement in meter and rad.
 */
void vpRobotFlirPtu::getDisplacement(const vpRobot::vpControlFrameType frame, vpColVector &q)
{
  (void)frame;
  (void)q;
  std::cout << "Not implemented ! " << std::endl;
}

/*!
  Connect to FLIR PTU.
  \param[in] portname : Connect to serial/socket.
  \param[in] baudrate : Use baud rate (default: 9600).
  \sa disconnect()
 */
void vpRobotFlirPtu::connect(const std::string &portname, int baudrate)
{
  char errstr[128];

  if (m_connected) {
    disconnect();
  }

  if(portname.empty()) {
    disconnect();
    throw(vpException(vpException::fatalError, "Port name is required to connect to FLIR PTU."));
  }

  if((m_cer = (struct cerial *)malloc(sizeof(struct cerial))) == NULL) {
    disconnect();
    throw(vpException(vpException::fatalError, "Out of memory during FLIR PTU connection."));
  }

  // open a port
  if(ceropen(m_cer, portname.c_str(), 0)) {
    disconnect();
    throw(vpException(vpException::fatalError, "Failed to open %s: %s.", portname.c_str(), cerstrerror(m_cer, errstr, sizeof(errstr))));
  }

  // Set baudrate
  // ignore errors since not all devices are serial ports
  cerioctl(m_cer, CERIAL_IOCTL_BAUDRATE_SET, &baudrate);

  /* flush any characters already buffered */
  cerioctl(m_cer, CERIAL_IOCTL_FLUSH_INPUT, NULL);

  /* set two second timeout */
  int timeout = 2000;
  if(cerioctl(m_cer, CERIAL_IOCTL_TIMEOUT_SET, &timeout)){
    disconnect();
    throw(vpException(vpException::fatalError, "cerial: timeout ioctl not supported."));
  }

  // Sync and lock
  int trial = 0;
  do {
    trial ++;
  } while(trial <= 3 && (cpi_resync(m_cer) || cpi_ptcmd(m_cer, &m_status, OP_NOOP)));
  if(trial > 3){
    disconnect();
    throw(vpException(vpException::fatalError, "Cannot communicate with FLIR PTU."));
  }

  // Immediately execute commands
  // (slave mode should be opt-in)
  int rc;
  if((rc = cpi_ptcmd(m_cer, &m_status, OP_EXEC_MODE_SET,
                     (cpi_enum)CPI_IMMEDIATE_MODE))){
    disconnect();
    throw(vpException(vpException::fatalError, "Set Immediate Mode failed: %s", cpi_strerror(rc)));
  }

  m_connected = true;

  getLimits();
}

/*!
  Close connection to PTU.
  \sa connect()
 */
void vpRobotFlirPtu::disconnect()
{
  if (m_cer){
    cerclose(m_cer);
    free(m_cer);
    m_connected = false;
  }
}

/*!
 Read min/max position and speed.
*/
void vpRobotFlirPtu::getLimits()
{
  if (! m_connected) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU is not connected."));
  }

  int status;
  std::vector<int> pos_min(2), pos_max(2), vel_max(2);

  if((status = cpi_ptcmd(m_cer, &m_status, OP_PAN_MAX_POSITION, &pos_max[0])) ||
     (status = cpi_ptcmd(m_cer, &m_status, OP_PAN_MIN_POSITION, &pos_min[0])) ||
     (status = cpi_ptcmd(m_cer, &m_status, OP_TILT_MAX_POSITION, &pos_max[1])) ||
     (status = cpi_ptcmd(m_cer, &m_status, OP_TILT_MIN_POSITION, &pos_min[1])) ||
     (status = cpi_ptcmd(m_cer, &m_status, OP_PAN_UPPER_SPEED_LIMIT_GET, &m_vel_max_tics[0])) ||
     (status = cpi_ptcmd(m_cer, &m_status, OP_TILT_UPPER_SPEED_LIMIT_GET, &m_vel_max_tics[1]))){
    disconnect();
    throw(vpException(vpException::fatalError, "Failed to get limits (%d) %s.", m_status, cpi_strerror(m_status)));
  }

  // Get the ptu resolution so we can convert the angles to ptu positions
  if((status = cpi_ptcmd(m_cer, &m_status, OP_PAN_RESOLUTION, &m_res[0])) ||
     (status = cpi_ptcmd(m_cer, &m_status, OP_TILT_RESOLUTION, &m_res[1]))) {
    disconnect();
    throw(vpException(vpException::fatalError, "Failed to get resolution (%d) %s.", status, cpi_strerror(status)));
  }

  for (size_t i=0; i < 2; i++) {
    m_res[i] /= 3600.; // Resolutions are in arc-seconds, but we want degrees
    m_pos_max[i] = vpMath::rad(m_res[i] * pos_max[i]);
    m_pos_min[i] = vpMath::rad(m_res[i] * pos_min[i]);
  }
}

/*!
  Return pan/tilt axis max positions in radians as a 2-dim vector, with first value the pan max position and second value, the max tilt position.
 */
vpColVector vpRobotFlirPtu::getPosMax()
{
  if (! m_connected) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU is not connected."));
  }
  return m_pos_max;
}

/*!
  Return pan/tilt axis min positions in radians as a 2-dim vector, with first value the pan min position and second value, the min tilt position.
 */
vpColVector vpRobotFlirPtu::getPosMin()
{
  if (! m_connected) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU is not connected."));
  }
  return m_pos_min;
}

/*!
  Return pan/tilt axis max velocity in rad/s as a 2-dim vector, with first value the pan max velocity and second value, the max tilt velocity.
 */
vpColVector vpRobotFlirPtu::getVelMax()
{
  if (! m_connected) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU is not connected."));
  }
  vpColVector vel_max(2);
  for (size_t i=0; i < 2; i++) {
    vel_max[i] = vpMath::rad(m_res[i] * m_vel_max_tics[i]);
  }
  return vel_max;
}

/*!

  Set the velocity used for a position control.

  \param velocity : Velocity in % of the maximum velocity between [0, 100]. Default value is 20.
*/
void vpRobotFlirPtu::setPositioningVelocity(double velocity)
{
  m_positioning_velocity = velocity;
}

/*!

Change the robot state.

\param newState : New requested robot state.
*/
vpRobot::vpRobotStateType vpRobotFlirPtu::setRobotState(vpRobot::vpRobotStateType newState)
{
  if (! m_connected) {
    disconnect();
    throw(vpException(vpException::fatalError, "FLIR PTU is not connected."));
  }

  switch (newState) {
  case vpRobot::STATE_STOP: {
    // Start primitive STOP only if the current state is Velocity
    if (vpRobot::STATE_VELOCITY_CONTROL == getRobotState()) {
      stopMotion();

      // Set the PTU to pure velocity mode
      if(cpi_ptcmd(m_cer, &m_status, OP_SPEED_CONTROL_MODE_SET,
                  (cpi_enum)CPI_CONTROL_INDEPENDENT)) {
        throw(vpException(vpException::fatalError, "Unable to set control mode independent."));
      }
    }
    break;
  }
  case vpRobot::STATE_POSITION_CONTROL: {
    if (vpRobot::STATE_VELOCITY_CONTROL == getRobotState()) {
      std::cout << "Change the control mode from velocity to position control.\n";
      stopMotion();

      // Set the PTU to pure velocity mode
      if(cpi_ptcmd(m_cer, &m_status, OP_SPEED_CONTROL_MODE_SET,
                  (cpi_enum)CPI_CONTROL_INDEPENDENT)) {
        throw(vpException(vpException::fatalError, "Unable to set control mode independent."));
      }

    } else {
      // std::cout << "Change the control mode from stop to position
      // control.\n";
    }
    break;
  }
  case vpRobot::STATE_VELOCITY_CONTROL: {
    if (vpRobot::STATE_VELOCITY_CONTROL != getRobotState()) {
      std::cout << "Change the control mode from stop to velocity control.\n";

      // Set the PTU to pure velocity mode
      if(cpi_ptcmd(m_cer, &m_status, OP_SPEED_CONTROL_MODE_SET,
                  (cpi_enum)CPI_CONTROL_PURE_VELOCITY)) {
        throw(vpException(vpException::fatalError, "Unable to set velocity control mode."));
      }
    }
    break;
  }
  default:
    break;
  }

  return vpRobot::setRobotState(newState);
}

/*!
  Stop PTU motion in velocity control mode.
 */
void vpRobotFlirPtu::stopMotion()
{
  if (! m_connected) {
    return;
  }

  // Set speed back to 0
  int vel = 0;
  if(cpi_ptcmd(m_cer, &m_status, OP_PAN_DESIRED_SPEED_SET, vel) ||
     cpi_ptcmd(m_cer, &m_status, OP_TILT_DESIRED_SPEED_SET, vel)) {
    throw(vpException(vpException::fatalError, "Unable to stop PTU."));
  }
}

#elif !defined(VISP_BUILD_SHARED_LIBS)
// Work arround to avoid warning: libvisp_robot.a(vpRobotFlirPtu.cpp.o) has
// no symbols
void dummy_vpRobotFlirPtu(){};
#endif


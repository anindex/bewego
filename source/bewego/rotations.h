#pragma once
#include <bewego/differentiable_map.h>
#include <iostream>

namespace bewego {

class QuatToEuler : public DifferentiableMap {
 public:
  QuatToEuler() {}
  
  uint32_t output_dimension() const { return 3; }
  uint32_t input_dimension() const { return 4; }

  /** q - quaternion [x, y, z, w] **/
  Eigen::VectorXd Forward(const Eigen::VectorXd& q) const {
    assert(q.size() == 4);
    const double& x = q[0];
    const double& y = q[1];
    const double& z = q[2];
    const double& w = q[3];
    double t0 = 2. * (w * x + y * z);
    double t1 = 1. - 2. * (x*x + y*y);
    double X = atan2(t0, t1);
    double t2 = 2. * (w*y - z*y);
    t2 = fmin(.99999, t2);
    t2 = fmax(-.99999, t2);
    double Y = asin(t2);

    double t3 = 2. * (w * z + x * y);
    double t4 = 1. - 2. * (y*y + z*z);
    double Z = atan2(t3, t4);
     
    Eigen::VectorXd euler(3);
    euler << X, Y, Z;
    return euler;
  }
  
  Eigen::MatrixXd Jacobian(const Eigen::VectorXd& q) const {
    assert(q.size() == 4);
    const double& x = q[0];
    const double& y = q[1];
    const double& z = q[2];
    const double& w = q[3];
    double t0 = 2. * (w * x + y * z);
    Eigen::Vector4d dt0(2.*w, 2.*z, 2.*y, 2.*x);
    double t1 = 1. - 2. * (x*x + y*y);
    Eigen::Vector4d dt1(-4.*x, -4.*y, 0., 0.);
    
    Eigen::Vector4d dX = -t0/(t0*t0+t1*t1)*dt1 + t1/(t0*t0+t1*t1)*dt0;

    double t2 = 2. * (w*y - z*y);
    Eigen::Vector4d dY;

    if (t2 < -.99999 || t2 > .99999) {
      
    } else {
      Eigen::Vector4d dt2;
      dt2 << -2.*z, 2.*w, -2.*x, 2.*y;
      dY = 1./(sqrt(1. - t2*t2)) * dt2;
    }
    
    
    double t3 = 2. * (w * z + x * y);
    double t4 = 1. - 2. * (y*y + z*z);
    Eigen::Vector4d dt3(2.*y, 2.*x, 2.*w, 2.*z);
    Eigen::Vector4d dt4(-4.*y, -4.*z, 0., 0.);
    Eigen::Vector4d dZ = -t3/(t3*t3+t4*t4)*dt4 + t4/(t3*t3+t4*t4)*dt3;
    Eigen::MatrixXd jac(3, 4);
    jac.row(0) = dX;
    jac.row(1) = dY;
    jac.row(2) = dZ;
    return jac;
  }

};
}
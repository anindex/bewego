/*
 * Copyright (c) 2021
 * All rights reserved.
 *
 * Redistribution  and  use  in  source  and binary  forms,  with  or  without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of  source  code must retain the  above copyright
 *      notice and this list of conditions.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice and  this list of  conditions in the  documentation and/or
 *      other materials provided with the distribution.
 *
 * THE SOFTWARE  IS PROVIDED "AS IS"  AND THE AUTHOR  DISCLAIMS ALL WARRANTIES
 * WITH  REGARD   TO  THIS  SOFTWARE  INCLUDING  ALL   IMPLIED  WARRANTIES  OF
 * MERCHANTABILITY AND  FITNESS.  IN NO EVENT  SHALL THE AUTHOR  BE LIABLE FOR
 * ANY  SPECIAL, DIRECT,  INDIRECT, OR  CONSEQUENTIAL DAMAGES  OR  ANY DAMAGES
 * WHATSOEVER  RESULTING FROM  LOSS OF  USE, DATA  OR PROFITS,  WHETHER  IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR  OTHER TORTIOUS ACTION, ARISING OUT OF OR
 * IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 *                                                             Thu 11 Feb 2021
 */
// author: Jim Mainprice, mainprice@gmail.com

#include <bewego/derivatives/atomic_operators.h>

using namespace bewego;
using std::cout;
using std::endl;

SecondOrderTaylorApproximation::SecondOrderTaylorApproximation(
    const DifferentiableMap& f, const Eigen::VectorXd& x0)
    : x0_(x0) {
  assert(f.output_dimension() == 1);
  double v = f.ForwardFunc(x0);
  Eigen::VectorXd g = f.Gradient(x0);
  Eigen::MatrixXd H = f.Hessian(x0);
  double c = 0;

  H_ = H;
  a_ = H;
  b_ = g - x0.transpose() * H;
  c = v - g.transpose() * x0 + .5 * x0.transpose() * H * x0;
  c_ = Eigen::VectorXd::Constant(1, c);

  x0_ = x0;
  g0_ = g;
  fx0_ = v;
}

//------------------------------------------------------------------------------
// LogBarrier implementation
//------------------------------------------------------------------------------

Eigen::VectorXd LogBarrier::Forward(const Eigen::VectorXd& x_vect) const {
  assert(x_vect.size() == 1);
  double x = x_vect[0];
  return Eigen::VectorXd::Constant(
      1, x <= margin_ ? std::numeric_limits<double>::infinity() : -log(x));
}

Eigen::MatrixXd LogBarrier::Jacobian(const Eigen::VectorXd& x_vect) const {
  assert(x_vect.size() == 1);
  double x = x_vect[0];
  return Eigen::MatrixXd::Constant(1, 1, x <= margin_ ? 0 : (-1. / x));
}

Eigen::MatrixXd LogBarrier::Hessian(const Eigen::VectorXd& x_vect) const {
  assert(x_vect.size() == 1);
  double x = x_vect[0];
  return Eigen::MatrixXd::Constant(1, 1, x <= margin_ ? 0 : 1. / (x * x));
}

//------------------------------------------------------------------------------
// LogBarrierWithApprox implementation
//------------------------------------------------------------------------------

Eigen::VectorXd LogBarrierWithApprox::Forward(
    const Eigen::VectorXd& x_vect) const {
  assert(x_vect.size() == 1);
  double x = x_vect[0];
  double v = 0;
  if (x <= 0) {
    v = std::numeric_limits<double>::infinity();
  } else {
    if (x <= x_splice_) {
      return approximation_->Forward(x_vect);
    } else {
      v = -scalar_ * log(x);
    }
  }
  return Eigen::VectorXd::Constant(1, v);
}

Eigen::MatrixXd LogBarrierWithApprox::Jacobian(
    const Eigen::VectorXd& x_vect) const {
  assert(x_vect.size() == 1);
  double x = x_vect[0];
  double g = 0;
  if (x > 0) {
    g = x <= x_splice_ ? approximation_->Jacobian(x_vect)(0, 0) : -scalar_ / x;
  }
  return Eigen::MatrixXd::Constant(1, 1, g);
}

Eigen::MatrixXd LogBarrierWithApprox::Hessian(
    const Eigen::VectorXd& x_vect) const {
  assert(x_vect.size() == 1);
  double x = x_vect[0];
  double h = 0;
  if (x > 0) {
    h = x <= x_splice_ ? approximation_->Hessian(x_vect)(0, 0)
                       : scalar_ / (x * x);
  }
  return Eigen::MatrixXd::Constant(1, 1, h);
}

std::shared_ptr<DifferentiableMap> LogBarrierWithApprox::MakeTaylorLogBarrier()
    const {
  auto log_barrier = std::make_shared<LogBarrier>();
  auto scaled_log_barrier = std::make_shared<Scale>(log_barrier, scalar_);
  return std::make_shared<SecondOrderTaylorApproximation>(
      *scaled_log_barrier, Eigen::VectorXd::Constant(1, x_splice_));
}

//------------------------------------------------------------------------------
// SoftNorm implementation
//------------------------------------------------------------------------------

Eigen::VectorXd SoftNorm::Forward(const Eigen::VectorXd& x) const {
  assert(x.size() == n_);
  Eigen::VectorXd xd = x - x0_;
  double alpha_norm = sqrt(xd.transpose() * xd + alpha_sq_);
  return Eigen::VectorXd::Constant(1, alpha_norm - alpha_);
}

Eigen::MatrixXd SoftNorm::Jacobian(const Eigen::VectorXd& x) const {
  assert(x.size() == n_);
  Eigen::VectorXd xd = x - x0_;
  double alpha_norm = sqrt(xd.transpose() * xd + alpha_sq_);
  return xd.transpose() / alpha_norm;
}

Eigen::MatrixXd SoftNorm::Hessian(const Eigen::VectorXd& x) const {
  assert(x.size() == n_);
  Eigen::VectorXd xd = x - x0_;
  double alpha_norm = sqrt(xd.transpose() * xd + alpha_sq_);
  Eigen::VectorXd x_alpha_normalized = xd / alpha_norm;
  Eigen::MatrixXd I = Eigen::MatrixXd::Identity(n_, n_);
  double gamma = 1. / alpha_norm;
  return gamma * (I - x_alpha_normalized * x_alpha_normalized.transpose());
}

//------------------------------------------------------------------------------
// SoftDist implementation
//------------------------------------------------------------------------------

SoftDist::SoftDist(DifferentiableMapPtr sq_dist, double alpha)
    : sq_dist_(sq_dist), alpha_(alpha), alpha_sq_(alpha * alpha) {
  assert(sq_dist_.get() != nullptr);
  assert(sq_dist_->output_dimension() == 1);
}

Eigen::VectorXd SoftDist::Forward(const Eigen::VectorXd& x) const {
  double alpha_norm = sqrt(sq_dist_->ForwardFunc(x) + alpha_sq_);
  return Eigen::VectorXd::Constant(1, alpha_norm - alpha_);
}

Eigen::MatrixXd SoftDist::Jacobian(const Eigen::VectorXd& x) const {
  assert(x.size() == n_);
  double alpha_norm = sqrt(sq_dist_->ForwardFunc(x) + alpha_sq_);
  return 0.5 * sq_dist_->Jacobian(x) / alpha_norm;
}

Eigen::MatrixXd SoftDist::Hessian(const Eigen::VectorXd& x) const {
  uint32_t n = input_dimension();
  double alpha_norm = sqrt(sq_dist_->ForwardFunc(x) + alpha_sq_);
  Eigen::VectorXd x_alpha_normalized = 0.5 * sq_dist_->Gradient(x) / alpha_norm;
  Eigen::MatrixXd H_d = .5 * sq_dist_->Hessian(x);
  double gamma = 1. / alpha_norm;
  return gamma * (H_d - x_alpha_normalized * x_alpha_normalized.transpose());
}

//-----------------------------------------------------------------------------
// LogSumExp implementation.
//-----------------------------------------------------------------------------

Eigen::VectorXd LogSumExp::Forward(const Eigen::VectorXd& x) const {
  assert(x.size() == n_);
  Eigen::VectorXd z = (alpha_ * x).array().exp();
  return Eigen::VectorXd::Constant(1, inv_alpha_ * std::log(z.sum()));
}

Eigen::MatrixXd LogSumExp::Jacobian(const Eigen::VectorXd& x) const {
  assert(x.size() == n_);
  Eigen::VectorXd z = (alpha_ * x).array().exp();
  double z_sum = z.sum();
  return z.transpose() / z_sum;
}

Eigen::MatrixXd LogSumExp::Hessian(const Eigen::VectorXd& x) const {
  assert(n_ == x.size());
  Eigen::MatrixXd H(Eigen::MatrixXd::Zero(n_, n_));
  Eigen::VectorXd z = (alpha_ * x).array().exp();
  double z_sum = z.sum();
  double p_inv = 1 / z_sum;
  Eigen::MatrixXd M = z.asDiagonal();
  H = p_inv * M - std::pow(p_inv, 2) * z * z.transpose();
  H *= alpha_;
  return H;
}

//-----------------------------------------------------------------------------
// NegLogSumExp implementation.
//-----------------------------------------------------------------------------

NegLogSumExp::~NegLogSumExp() {}

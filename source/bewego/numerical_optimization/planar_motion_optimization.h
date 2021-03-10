/*
 * Copyright (c) 2020
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
 *                                               Jim Mainprice Wed 10 Mar 2021
 */
#pragma once

#include <bewego/motion/objective.h>
#include <bewego/motion/trajectory.h>
#include <bewego/workspace/workspace.h>

namespace bewego {
namespace numerical_optimization {

class PlanarOptimzer : public MotionObjective {
 public:
  PlanarOptimzer(uint32_t T,                        // number of cliques
                 double dt,                         // time between cliques
                 const Rectangle& workspace_bounds  // bounds
  );

  /**
   * @brief set_parameters_from_flags
   * Reads parameters in google flags
   */
  void set_parameters_from_flags();

  /**
   * @brief Optimize
   * @param initial_traj
   * @param x_goal
   * @return an optimized trajectory
   */
  std::shared_ptr<Trajectory> Optimize(const Eigen::VectorXd& initial_traj,
                                       const Eigen::Vector2d& x_goal) const;

 protected:
  typedef CliquesFunctionNetwork FunctionNetwork;
  typedef std::shared_ptr<const FunctionNetwork> FunctionNetworkPtr;
  typedef std::shared_ptr<const DifferentiableMap> ElementaryFunction;

  std::shared_ptr<const ConstrainedOptimizer> SetupIpoptOptimizer(
      const Eigen::VectorXd& q_init) const;

  std::vector<Bounds> GetJointLimits() const;
  std::vector<Bounds> GetDofBounds() const;

  // ipopt constraints
  FunctionNetwork GetEmptyFunctionNetwork() const;
  std::vector<FunctionNetwork> GetKeyPointsSurfaceConstraints() const;

  // Constraints networks
  std::vector<FunctionNetwork> g_constraints_;  // inequalities
  std::vector<FunctionNetwork> h_constraints_;  // equalities

  // Bounds of the workspace
  Rectangle workspace_bounds_;

  // options
  bool with_rotation_;
  bool with_attractor_constraint_;

  // Workspace
  std::shared_ptr<const DifferentiableMap> smooth_collision_constraint_;

  // Logging
  bool visualize_inner_loop_;
  bool monitor_inner_statistics_;
  // mutable std::shared_ptr<rieef::FreeflyerOptimizationVisualizer>
  // visualizer_; mutable std::shared_ptr<rieef::StatsMonitor> stats_monitor_;
};

}  // namespace numerical_optimization
}  // namespace bewego

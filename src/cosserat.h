#ifndef COSSERAT_H
#define COSSERAT_H

#include <Eigen/Sparse>
#include <unordered_set>
#include <memory>

namespace bigbang {

/// @brief generate a rod with n vertices
void generate_rod(const Eigen::Matrix<double, 3, 2> &ends, const size_t n, Eigen::Matrix3Xd &rod);

/// @brief x=R*cos(omega*t), y = R*sin(omega*t), z = h*t
void init_rod_as_helix(const double radius, const double h, const double omega, const double dt, Eigen::Matrix3Xd &rod);

/// @brief compute bishop frame
void compute_bishop_frame(const Eigen::Matrix3Xd &rod, Eigen::Matrix4d &frm);

template <typename T>
class Functional;

struct rod_material {
  double h;
  double radius;
  double density;
  double E, G, Es;
  double kappa;
};

/// @brief designed as a physics solver
/// rather than a variational solver
class cosserat_solver
{
public:
  cosserat_solver(const Eigen::Matrix3Xd &rest);
  void init_rod(const Eigen::Matrix3Xd &rinit, const Eigen::Matrix4Xd &qinit);
  void config_material(const rod_material &param) { param_ = param; }
  void pin_down_vert(const size_t id, const double *pos);
  void precompute();
  void advance(const size_t max_iter, const double tolerance=1e-8);
  Eigen::Matrix3Xd& get_rod_pos() { return r_; }
private:
  void assemble_mass_mat();
private:
  // physics part
  const Eigen::Matrix3Xd rest_;
  Eigen::Matrix3Xd r_, vr_;
  Eigen::Matrix4Xd q_, vq_;
//  Eigen::VectorXd Mr_, Mq_;
  std::vector<std::shared_ptr<Functional<double>>> buffer_;
  std::shared_ptr<Functional<double>> potential_;
  // material part
  rod_material param_;
  // numeric part
  std::unordered_set<size_t> fixDoF_;
  std::vector<size_t> g2l_;
};

}

#endif
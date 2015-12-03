#include "optimizer.h"

#include <iostream>
#include <Eigen/UmfPackSupport>
#include <lbfgs.h>

#include "config.h"
#include "def.h"
#include "HLBFGS/HLBFGS.h"
#include "HLBFGS/Lite_Sparse_Matrix.h"

using namespace std;
using namespace Eigen;

namespace bigbang {

int newton_solve(double *x, const size_t dim, const pfunc &f, const opt_args &args) {
  if ( dim != f->Nx() ) {
    cerr << "[error] dim not match\n";
    return __LINE__;
  }
  SimplicialCholesky<SparseMatrix<double>> sol;
  Map<VectorXd> X(x, dim);
  VectorXd xstar = X, dx(dim);
  for (size_t iter = 0; iter < args.max_iter; ++iter) {
    double value = 0; {
      f->Val(&xstar[0], &value);
      if ( iter % 100 == 0 ) {
        cout << "\t@iter " << iter << endl;
        cout << "\t@energy value: " << value << endl;
      }
    }
    VectorXd grad = VectorXd::Zero(dim); {
      f->Gra(&xstar[0], &grad[0]);
      if ( grad.norm() <= args.eps ) {
        cout << "\t@gradient converged\n\n";
        break;
      }
    }
    SparseMatrix<double> H(dim, dim); {
      vector<Triplet<double>> trips;
      f->Hes(&xstar[0], &trips);
      H.reserve(trips.size());
      H.setFromTriplets(trips.begin(), trips.end());
    }
    sol.compute(H);
    ASSERT(sol.info() == Success);
    dx = -sol.solve(grad);
    ASSERT(sol.info() == Success);
    double xstar_norm = xstar.norm();
    double h = 1.0;
    // line search here
    if ( args.lineseach ) {
      h /= 0.5;
      VectorXd xnew(dim);
      double lhs = 0.0, rhs = 0.0;
      do {
        h *= 0.5;
        xnew = xstar+h*dx;
        f->Val(&xnew[0], &lhs);
        rhs = value+h*0.45*(grad.dot(dx));
      } while ( lhs >= value && h > 1e-12 );
    }
    xstar += h*dx;
    if ( h*dx.norm() <= args.eps*xstar_norm ) {
      cout << "[info] converged after " << iter << " iterations\n\n";
      break;
    }
  }
  X = xstar;
  return 0;
}

static shared_ptr<Functional<double>> energy;

static lbfgsfloatval_t evaluate(void *instance, const lbfgsfloatval_t *x,
                                lbfgsfloatval_t *g, const int n,
                                const lbfgsfloatval_t step) {
  double f = 0;
  energy->Val(x, &f);
  std::fill(g, g+n, 0);
  energy->Gra(x, g);
  return f;
}

static int progress(void *instance,
                    const lbfgsfloatval_t *x,
                    const lbfgsfloatval_t *g,
                    const lbfgsfloatval_t fx,
                    const lbfgsfloatval_t xnorm,
                    const lbfgsfloatval_t gnorm,
                    const lbfgsfloatval_t step,
                    int n, int k, int ls) {
//  printf("Iteration %d:\n", k);
//  printf("  fx = %f, x[0] = %f, x[1] = %f\n", fx, x[0], x[1]);
//  printf("  xnorm = %f, gnorm = %f, step = %f\n", xnorm, gnorm, step);
//  printf("\n");
  return 0;
}

int lbfgs_solve(double *x, const size_t dim, const pfunc &f, const opt_args &args) {
  if ( dim != f->Nx() ) {
    cerr << "[error] dim not match\n";
    return __LINE__;
  }

  energy = f;
  lbfgsfloatval_t fx;
  lbfgs_parameter_t param;
  lbfgs_parameter_init(&param);

  int ret = lbfgs(dim, x, &fx, evaluate, progress, NULL, &param);

  printf("L-BFGS optimization terminated with status code = %d\n", ret);
  return 0;
}

//int constrained_newton_solve(double *x, const size_t dim, pfunc &f, const pcons &c) {
//  if ( dim != f->Nx() || dim != c->Nx() ) {
//    cerr << "[error] dim not match\n";
//    return __LINE__;
//  }
//  const size_t fdim = c->Nf();
//  UmfPackLU<SparseMatrix<double>> sol;
//  VectorXd X = VectorXd::Zero(dim+fdim);
//  std::copy(x, x+dim, X.data());
//  VectorXd xprev = X.head(dim);
//  for (size_t iter = 0; iter < MAX_ITER; ++iter) {
//    VectorXd rhs(dim+fdim); {
//      rhs.setZero();
//      f->Gra(&X[0], &rhs[0]);
//      c->Val(&X[0], &rhs[dim]);
//    }
//    SparseMatrix<double> LHS(dim+fdim, dim+fdim); {
//      vector<Triplet<double>> trips;
//      f->Hes(&X[0], &trips);
//      const auto begin = trips.end();
//      c->Jac(&X[0], dim, &trips);
//      const auto end = trips.end();
//      for (auto it = begin; it != end; ++it) {

//      }
//    }
//  }
//  std::copy(X.data(), X.data()+dim, x);
//  return 0;
//}

//int gauss_newton_solve(double *x, const size_t dim, const pcons &f) {
//  if ( dim != f->Nx() ) {
//    return __LINE__;
//  }
//  const size_t fdim = f->Nf();
//  for (size_t i = 0; i < MAX_ITER; ++i) {
//    VectorXd value(fdim); {
//      f->Val(x, &value[0]);
//    }
//    SparseMatrix<double> J(fdim, dim); {

//    }

//  }
//  return 0;
//}

}

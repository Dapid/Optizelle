#include "peopt/peopt.h"
#include "peopt/vspaces.h"
#include "peopt/json.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>

// Optimize a simple optimization problem with an optimal solution
// of (1/3,1/3)

// Squares its input
template <typename Real>
Real sq(Real x){
    return x*x; 
}

// Define a simple objective where 
// 
// f(x,y)=(x+1)^2+(y+1)^2
//
struct MyObj
    : public peopt::ScalarValuedFunction <double,peopt::Rm>
{
    typedef peopt::Rm <double> X;

    // Evaluation 
    double operator () (const X::Vector& x) const {
        return sq(x[0]+1.)+sq(x[1]+1.);
    }

    // Gradient
    void grad(
        const X::Vector& x,
        X::Vector& grad
    ) const {
        grad[0]=2.*x[0]+2.;
        grad[1]=2.*x[1]+2.;
    }

    // Hessian-vector product
    void hessvec(
        const X::Vector& x,
        const X::Vector& dx,
        X::Vector& H_dx
    ) const {
        H_dx[0]=2.*dx[0]; 
        H_dx[1]=2.*dx[1]; 
    }
};

// Define simple inequalities 
//
// h(x,y)= [ x + 2y >= 1 ] 
//         [ 2x + y >= 1 ] 
//
struct MyIneq
    :public peopt::VectorValuedFunction<double,peopt::Rm,peopt::Rm>
{
    typedef peopt::Rm <double> X;
    typedef peopt::Rm <double> Y;

    // y=h(x) 
    void operator () (
        const X::Vector& x,
        Y::Vector& y
    ) const {
        y[0]=x[0]+2.*x[1]-1.;
        y[1]=2.*x[0]+x[1]-1.;
    }

    // y=h'(x)dx
    void p(
        const X::Vector& x,
        const X::Vector& dx,
        Y::Vector& y
    ) const {
        y[0]= dx[0]+2.*dx[1];
        y[1]= 2.*dx[0]+dx[1];
    }

    // z=h'(x)*dy
    void ps(
        const X::Vector& x,
        const Y::Vector& dy,
        X::Vector& z
    ) const {
        z[0]= dy[0]+2.*dy[1];
        z[1]= 2.*dy[0]+dy[1];
    }

    // z=(h''(x)dx)*dy
    void pps(
        const X::Vector& x,
        const X::Vector& dx,
        const Y::Vector& dy,
        X::Vector& z
    ) const {
        X::zero(z);
    }
};

int main(int argc,char* argv[]){
    // Read in the name for the input file
    if(argc!=2) {
        std::cerr << "rosenbrock <parameters>" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string fname(argv[1]);

    // Create a type shortcut
    using peopt::Rm;

    // Generate an initial guess
    std::vector <double> x(2);
    x[0]=2.1; x[1]=1.1;

    // Allocate memory for the inequality multipler 
    std::vector <double> z(2);

    // Create an optimization state
    peopt::InequalityConstrained <double,Rm,Rm>::State::t
        state(x,z);

    // Read the parameters from file
    peopt::json::InequalityConstrained <double,peopt::Rm,peopt::Rm>
        ::read(peopt::Messaging(),fname,state);
    
    // Create a bundle of functions
    peopt::InequalityConstrained <double,Rm,Rm>::Functions::t fns;
    fns.f.reset(new MyObj);
    fns.h.reset(new MyIneq);

    // Solve the optimization problem
    peopt::InequalityConstrained <double,Rm,Rm>::Algorithms
        ::getMin(peopt::Messaging(),fns,state);

    // Print out the reason for convergence
    std::cout << "The algorithm converged due to: " <<
        peopt::StoppingCondition::to_string(state.opt_stop) <<
        std::endl;

    // Print out the final answer
    const std::vector <double>& opt_x=*(state.x.begin());
    std::cout << std::scientific << std::setprecision(16)
        << "The optimal point is: (" << opt_x[0] << ','
	<< opt_x[1] << ')' << std::endl;

    // Write out the final answer to file
    peopt::json::InequalityConstrained<double,peopt::Rm,peopt::Rm>
        ::write_restart(peopt::Messaging(),"simple_inequality.perst",state);

    // Return that the program exited properly
    return EXIT_SUCCESS;
}

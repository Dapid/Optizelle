#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "peopt/peopt.h"
#include "peopt/vspaces.h"
#include "peopt/linalg.h"

// This tests our linear algebra routines 
typedef peopt::Natural Natural;

template <typename Real>
struct BasicOperator : public peopt::Operator <Real,peopt::Rm,peopt::Rm> {
private:
    // Create some type shortcuts
    typedef std::vector <Real> X_Vector;
    typedef std::vector <Real> Y_Vector;
    
    // Size of the matrix
    Natural m;
public:
    // Storage for the matrix
    std::vector <Real> A;

    // Create an empty matrix, which must be filled by the user 
    BasicOperator(const Natural m_) : m(m_) {
        A.resize(m*m); 
    }
    
    // Apply the random matrix to the vector 
    void operator () (const X_Vector& x,Y_Vector &y) const  {
        for(Natural i=0;i<m;i++){
            y[i]=0;
            for(Natural j=0;j<m;j++)
                y[i]+=A[i+m*j]*x[j];
        }
    }
};

// Create the identity operator
template <typename Real>
struct IdentityOperator : public peopt::Operator <Real,peopt::Rm,peopt::Rm> {
private:
    // Create some type shortcuts
    typedef std::vector <Real> X_Vector;
    typedef std::vector <Real> Y_Vector;

public:
    // We don't need to do anything on creation
    IdentityOperator() {};

    // Just copy the input to the output 
    void operator () (const X_Vector& x,Y_Vector &y) const  {
        peopt::Rm <Real>::copy(x,y);
    }
};


BOOST_AUTO_TEST_SUITE(linear_algebra)

BOOST_AUTO_TEST_CASE(gmres_full) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set how often we restart GMRES
    Natural rst_freq = 0;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural i=1;i<=m*m;i++)
        A.A[i-1]=cos(pow(i,m-1));
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 
    
    // Create the left preconditioner
    BasicOperator <double> Ml_inv(m);
    for(Natural i=1;i<=m*m;i++)
        Ml_inv.A[i-1]=cos(pow(30.+i,m-1));
    
    // Create the right preconditioner
    BasicOperator <double> Mr_inv(m);
    for(Natural i=1;i<=m*m;i++)
        Mr_inv.A[i-1]=cos(pow(55.+i,m-1));

    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create an empty GMRES manipulator
    peopt::GMRESManipulator <double,peopt::Rm> gmanip;

    // Solve this linear system
    std::pair <double,Natural> err_iter = peopt::gmres <double,peopt::Rm>
        (A,b,eps_krylov,iter_max,rst_freq,Ml_inv,Mr_inv,gmanip,x);

    // Check the error is less than our tolerance 
    BOOST_CHECK(err_iter.first < eps_krylov);

    // Check that we ran to the maximum number of iterations
    BOOST_CHECK(err_iter.second == m);
    
    // Check the relative error between the true solution and that
    // returned from GMRES
    std::vector <double> x_star(5);
    x_star[0] = -1.203932331447497;
    x_star[1] = -0.186416740769010;
    x_star[2] = -0.457476984550115;
    x_star[3] = -0.830522778995837;
    x_star[4] = -1.125112777803922;
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);
}

BOOST_AUTO_TEST_CASE(gmres_left_preconditioner) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set how often we restart GMRES
    Natural rst_freq = 0;

    // Create some operator with only three elements on the diagonal 
    BasicOperator <double> A(m);
    for(Natural i=1;i<=m*m;i++) A.A[i-1]=0;
    A.A[0]=2.;
    A.A[2+2*m]=3.;
    A.A[4+4*m]=4.;
    
    // Create some right hand side with ones in the places that correspond
    // to the operator above.  In this case, we have them at 1, 3, and 5.
    std::vector <double> b(m);
    for(Natural i=0;i<m;i++) b[i] = 0.;
    b[0]= 1.;
    b[2]= 1.;
    b[4]= 1.;
    
    // Create the left preconditioner.  Do this by inverting the matrix by
    // hand.
    BasicOperator <double> Ml_inv(m);
    for(Natural i=1;i<=m*m;i++) Ml_inv.A[i-1]=0.;
    Ml_inv.A[0]=1./2.;
    Ml_inv.A[2+2*m]=1./3.;
    Ml_inv.A[4+4*m]=1./4.;
    
    // Set the right preconditioner to the identity
    IdentityOperator <double> Mr_inv;

    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create an empty GMRES manipulator
    peopt::GMRESManipulator <double,peopt::Rm> gmanip;

    // Solve this linear system
    std::pair <double,Natural> err_iter = peopt::gmres <double,peopt::Rm>
        (A,b,eps_krylov,iter_max,rst_freq,Ml_inv,Mr_inv,gmanip,x);

    // Check the error is less than our tolerance 
    BOOST_CHECK(err_iter.first < eps_krylov);

    // Check that we ran to the maximum number of iterations
    BOOST_CHECK(err_iter.second == 1);
    
    // Check the relative error between the true solution and that
    // returned from GMRES
    std::vector <double> x_star(5);
    x_star[0] = 0.5;  
    x_star[1] = 0.; 
    x_star[2] = 1./3.; 
    x_star[3] = 0.; 
    x_star[4] = 0.25; 
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);
}

BOOST_AUTO_TEST_CASE(gmres_right_preconditioner) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set how often we restart GMRES
    Natural rst_freq = 0;

    // Create some operator with only three elements on the diagonal 
    BasicOperator <double> A(m);
    for(Natural i=1;i<=m*m;i++) A.A[i-1]=0;
    A.A[0]=2.;
    A.A[2+2*m]=3.;
    A.A[4+4*m]=4.;
    
    // Create some right hand side with ones in the places that correspond
    // to the operator above.  In this case, we have them at 1, 3, and 5.
    std::vector <double> b(m);
    for(Natural i=0;i<m;i++) b[i] = 0.;
    b[0]= 1.;
    b[2]= 1.;
    b[4]= 1.;
    
    // Create the right preconditioner.  Do this by inverting the matrix by
    // hand.
    BasicOperator <double> Mr_inv(m);
    for(Natural i=1;i<=m*m;i++) Mr_inv.A[i-1]=0.;
    Mr_inv.A[0]=1./2.;
    Mr_inv.A[2+2*m]=1./3.;
    Mr_inv.A[4+4*m]=1./4.;
    
    // Set the left preconditioner to the identity
    IdentityOperator <double> Ml_inv;

    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create an empty GMRES manipulator
    peopt::GMRESManipulator <double,peopt::Rm> gmanip;

    // Solve this linear system
    std::pair <double,Natural> err_iter = peopt::gmres <double,peopt::Rm>
        (A,b,eps_krylov,iter_max,rst_freq,Ml_inv,Mr_inv,gmanip,x);

    // Check the error is less than our tolerance 
    BOOST_CHECK(err_iter.first < eps_krylov);

    // Check that we ran to the maximum number of iterations
    BOOST_CHECK(err_iter.second == 1);
    
    // Check the relative error between the true solution and that
    // returned from GMRES
    std::vector <double> x_star(5);
    x_star[0] = 0.5;  
    x_star[1] = 0.; 
    x_star[2] = 1./3.; 
    x_star[3] = 0.; 
    x_star[4] = 0.25; 
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);
}

BOOST_AUTO_TEST_CASE(gmres_restart) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 300;

    // Set how often we restart GMRES
    Natural rst_freq = 3;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural i=1;i<=m*m;i++)
        A.A[i-1]=cos(pow(i,2));
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 

    // Create the left preconditioner
    IdentityOperator <double> Ml_inv;
    
    // Create the right preconditioner
    IdentityOperator <double> Mr_inv;

    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create an empty GMRES manipulator
    peopt::GMRESManipulator <double,peopt::Rm> gmanip;

    // Solve this linear system
    std::pair <double,Natural> err_iter = peopt::gmres <double,peopt::Rm>
        (A,b,eps_krylov,iter_max,rst_freq,Ml_inv,Mr_inv,gmanip,x);

    // Check the error is less than our tolerance 
    BOOST_CHECK(err_iter.first < eps_krylov);

    // Check that we ran to the maximum number of iterations
    BOOST_CHECK(err_iter.second == 242);
}

BOOST_AUTO_TEST_CASE(tpcd_basic_solve) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 100.;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 

    // Get the norm of the RHS
    double norm_b = std::sqrt(X::innr(b,b));
    
    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_cd <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,false,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check the error is less than our tolerance 
    BOOST_CHECK(norm_r < eps_krylov*norm_b);

    // Check that we ran to the maximum number of iterations
    BOOST_CHECK(iter == m);
    
    // Check the relative error between the true solution and that
    // returned from TPCG 
    std::vector <double> x_star(5);
    x_star[0] = 0.062210523692158425;
    x_star[1] = -0.027548098303754341;
    x_star[2] = -0.11729291808469694;
    x_star[3] = -0.080812473373141375;
    x_star[4] = 0.032637688404329734;
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);

    // Check that the returned solution is different than the Cauchy point
    peopt::Rm <double>::copy(x_cp,residual);
    peopt::Rm <double>::axpy(-1,x,residual);
    err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_cp,x_cp)));
    BOOST_CHECK(err > 1e-4);
}

BOOST_AUTO_TEST_CASE(tpcd_tr_stopping) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 0.1;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 
    
    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_cd <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,false,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check that the size of x is just the trust-region radius
    double norm_x = sqrt(X::innr(x,x));
    BOOST_CHECK_CLOSE(norm_x,delta,1e-8);
}

// In this problem, we have
// A = [ 1 -1 ]
//     [-1  1 ]
// b = [ 3 ]
//     [ 4 ]
// This has no solution.  On the first iteration, CG will move
// in the steepest descent direction, which is b.  In order to check the code
// for moving the center of a trust-region, we put the center at [-3;-4] with 
// a radius of 7.5.  By setting the center in the opposite direction with a
// radius of 7.5, it should only move half the distance.
BOOST_AUTO_TEST_CASE(tpcd_tr_stopping_moved_center) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 2;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 7.5;

    // Create some operator 
    BasicOperator <double> A(m);
    A.A[0]=1.;
    A.A[1]=-1.;
    A.A[2]=-1.;
    A.A[3]=1.;
    
    // Create some right hand side
    std::vector <double> b(m);
    b[0]=3.;
    b[1]=4.;
    
    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create a vector for the solution 
    std::vector <double> x(m);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    x_cntr[0]=-3.;
    x_cntr[1]=-4.;

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_cd <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,false,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check that the size of x is 2.5 
    double norm_x = sqrt(X::innr(x,x));
    BOOST_CHECK_CLOSE(norm_x,2.5,1e-8);

    // Check that the solution is [1.5;2]
    std::vector <double> x_star(m);
    x_star[0] = 1.5; 
    x_star[1] = 2.; 
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);
}

BOOST_AUTO_TEST_CASE(tpcd_cp) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 1;

    // Set the trust-reregion radius 
    double delta = 100.;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 

    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_cd <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,false,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check that we ran to a single iteration 
    BOOST_CHECK(iter == 1);
    
    // Check that the returned solution and the Cauchy point are the same
    std::vector <double> residual = x_cp;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_cp,x_cp)));
    BOOST_CHECK(err < 1e-14);
}

BOOST_AUTO_TEST_CASE(tpcd_nullspace_solve) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 100.;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create a simple nullspace projector.  This projects out the first
    // two elements
    BasicOperator <double> W(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            W.A[I-1]=(i==j && i<=2) ? 1. : 0.;
        }

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create some right hand side.  Make sure that this is in the range
    // of A*W.
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = A.A[i-1]+A.A[i-1+m];

    // Get the norm of the RHS
    double norm_b = std::sqrt(X::innr(b,b));
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_cd <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,true,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check the error is less than our tolerance 
    BOOST_CHECK(norm_r < eps_krylov*norm_b);

    // Check that we completed in two iterations.  This is due to the
    // nullspace projection
    BOOST_CHECK(iter == 2);
    
    // Check the relative error between the true solution and that
    // returned from TPCG. 
    std::vector <double> x_star(5);
    x_star[0] = 1.0; 
    x_star[1] = 1.0; 
    x_star[2] = 0.0;
    x_star[3] = 0.0;
    x_star[4] = 0.0;
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);

    // Check that the returned solution is different than the Cauchy point
    peopt::Rm <double>::copy(x_cp,residual);
    peopt::Rm <double>::axpy(-1,x,residual);
    err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_cp,x_cp)));
    BOOST_CHECK(err > 1e-4);
}

BOOST_AUTO_TEST_CASE(tpcd_starting_solution) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 100.;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 

    // Get the norm of the RHS
    double norm_b = std::sqrt(X::innr(b,b));
    
    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    for(Natural i=1;i<=m;i++) x[i-1]=1.;

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_cd <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,true,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check the error is less than our tolerance 
    BOOST_CHECK(norm_r < eps_krylov*norm_b);

    // Check that we ran to the maximum number of iterations
    BOOST_CHECK(iter == m);
    
    // Check the relative error between the true solution and that
    // returned from TPCG 
    std::vector <double> x_star(5);
    x_star[0] = 0.062210523692158425;
    x_star[1] = -0.027548098303754341;
    x_star[2] = -0.11729291808469694;
    x_star[3] = -0.080812473373141375;
    x_star[4] = 0.032637688404329734;
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);

    // Check that the returned solution is different than the Cauchy point
    peopt::Rm <double>::copy(x_cp,residual);
    peopt::Rm <double>::axpy(-1,x,residual);
    err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_cp,x_cp)));
    BOOST_CHECK(err > 1e-4);
}

BOOST_AUTO_TEST_CASE(tminres_basic_solve) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 100.;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 

    // Get the norm of the RHS
    double norm_b = std::sqrt(X::innr(b,b));
    
    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_minres <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check the error is less than our tolerance 
    BOOST_CHECK(norm_r < eps_krylov*norm_b);

    // Check that we ran to the maximum number of iterations
    BOOST_CHECK(iter == m);
    
    // Check the relative error between the true solution and that
    // returned from TPCG 
    std::vector <double> x_star(5);
    x_star[0] = 0.062210523692158425;
    x_star[1] = -0.027548098303754341;
    x_star[2] = -0.11729291808469694;
    x_star[3] = -0.080812473373141375;
    x_star[4] = 0.032637688404329734;
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);

    // Check that the returned solution is different than the Cauchy point
    peopt::Rm <double>::copy(x_cp,residual);
    peopt::Rm <double>::axpy(-1,x,residual);
    err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_cp,x_cp)));
    BOOST_CHECK(err > 1e-4);
}

BOOST_AUTO_TEST_CASE(tminres_tr_stopping) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 0.1;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 
    
    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_minres <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check that the size of x is just the trust-region radius
    double norm_x = sqrt(X::innr(x,x));
    BOOST_CHECK_CLOSE(norm_x,delta,1e-8);
}

// In this problem, we have
// A = [ 1 -1 ]
//     [-1  1 ]
// b = [ 3 ]
//     [ 4 ]
// This has no solution.  On the first iteration, MINRES will move in the
// space generated by the first Krylov vector, which is b.  The optimal
// amount will put us at [1.5;2]  In order to check the code
// for moving the center of a trust-region, we put the center at [-3;-4] with 
// a radius of 6.25.  By setting the center in the opposite direction with
// a radius of 6.25, it should only move half the distance to [0.75;1].
BOOST_AUTO_TEST_CASE(tminres_tr_stopping_moved_center) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 2;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 6.25;

    // Create some operator 
    BasicOperator <double> A(m);
    A.A[0]=1.;
    A.A[1]=-1.;
    A.A[2]=-1.;
    A.A[3]=1.;
    
    // Create some right hand side
    std::vector <double> b(m);
    b[0]=3.;
    b[1]=4.;
    
    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create a vector for the solution 
    std::vector <double> x(m);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    x_cntr[0]=-3.;
    x_cntr[1]=-4.;

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_minres <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check that the size of x is 1.25 
    double norm_x = sqrt(X::innr(x,x));
    BOOST_CHECK_CLOSE(norm_x,1.25,1e-8);

    // Check that the solution is [0.75,1]
    std::vector <double> x_star(m);
    x_star[0] = 0.75; 
    x_star[1] = 1.; 
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);
}

BOOST_AUTO_TEST_CASE(tminres_cp) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 1;

    // Set the trust-reregion radius 
    double delta = 100.;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create some right hand side
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = cos(i+25); 

    // Create some empty null-space projection 
    IdentityOperator <double> W;

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_minres <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check that we ran to a single iteration 
    BOOST_CHECK(iter == 1);
    
    // Check that the returned solution and the Cauchy point are the same
    std::vector <double> residual = x_cp;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_cp,x_cp)));
    BOOST_CHECK(err < 1e-14);
}

BOOST_AUTO_TEST_CASE(tminres_nullspace_solve) {
    // Create a type shortcut
    typedef peopt::Rm <double> X;
    typedef X::Vector X_Vector;

    // Set the size of the problem
    Natural m = 5;

    // Set the stopping tolerance
    double eps_krylov = 1e-12;

    // Set the maximum number of iterations
    Natural iter_max = 200;

    // Set the trust-reregion radius 
    double delta = 100.;

    // Create some operator 
    BasicOperator <double> A(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            Natural J = i+(j-1)*m;
            if(i>j) {
                A.A[I-1]=cos(pow(I,m-1));
                A.A[J-1]=A.A[I-1];
            } else if(i==j)
                A.A[I-1]=cos(pow(I,m-1))+10;
        }
    
    // Create a simple nullspace projector.  This projects out the first
    // two elements
    BasicOperator <double> W(m);
    for(Natural j=1;j<=m;j++)
        for(Natural i=1;i<=m;i++) {
            Natural I = j+(i-1)*m;
            W.A[I-1]=(i==j && i<=2) ? 1. : 0.;
        }

    // Create some empty trust-region shape operator
    IdentityOperator <double> TR_op;
    
    // Create some right hand side.  Make sure that this is in the range
    // of A*W.
    std::vector <double> b(m);
    for(Natural i=1;i<=m;i++) b[i-1] = A.A[i-1]+A.A[i-1+m];

    // Get the norm of the RHS
    double norm_b = std::sqrt(X::innr(b,b));
    
    // Create an initial guess at the solution
    std::vector <double> x(m);
    X::zero (x);

    // Create a vector for the Cauchy point
    std::vector <double> x_cp(m);

    // Create a vector for the center of the trust-region
    std::vector <double> x_cntr(m);
    peopt::Rm <double>::zero(x_cntr);

    // Solve this linear system
    double norm_r;
    Natural iter;
    peopt::KrylovStop::t krylov_stop;
    peopt::truncated_minres <double,peopt::Rm>
        (A,b,W,TR_op,eps_krylov,iter_max,1,delta,x_cntr,x,x_cp,
            norm_r,iter,krylov_stop);

    // Check the error is less than our tolerance 
    BOOST_CHECK(norm_r < eps_krylov*norm_b);

    // Check that we completed in two iterations.  This is due to the
    // nullspace projection
    BOOST_CHECK(iter == 2);
    
    // Check the relative error between the true solution and that
    // returned from TPCG. 
    std::vector <double> x_star(5);
    x_star[0] = 1.0; 
    x_star[1] = 1.0; 
    x_star[2] = 0.0;
    x_star[3] = 0.0;
    x_star[4] = 0.0;
    std::vector <double> residual = x_star;
    peopt::Rm <double>::axpy(-1,x,residual);
    double err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_star,x_star)));
    BOOST_CHECK(err < 1e-14);

    // Check that the returned solution is different than the Cauchy point
    peopt::Rm <double>::copy(x_cp,residual);
    peopt::Rm <double>::axpy(-1,x,residual);
    err=std::sqrt(peopt::Rm <double>::innr(residual,residual))
        /(1+sqrt(peopt::Rm <double>::innr(x_cp,x_cp)));
    BOOST_CHECK(err > 1e-4);
}

BOOST_AUTO_TEST_SUITE_END()

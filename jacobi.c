/*
 *  OpenMP lecture exercises
 *  Copyright (C) 2011 by Christian Terboven <terboven@rz.rwth-aachen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 */

#define U(i,j) u[(i)*n+(j)]
#define UOLD(i,j) uold[(i)*n+(j)]
#define F(i,j) f[(i)*n+(j)]
#define BLOCK_X 8
#define BLOCK_Y 8
#define min(a,b)            (((a) < (b)) ? (a) : (b))
/* 
******************************************************************
* Subroutine HelmholtzJ
* Solves poisson equation on rectangular grid assuming : 
* (1) Uniform discretization in each direction, and 
* (2) Dirichlect boundary conditions 
* 
* Jacobi method is used in this routine 
*
* Input : n,m   Number of grid points in the X/Y directions 
*         dx,dy Grid spacing in the X/Y directions 
*         alpha Helmholtz eqn. coefficient 
*         omega Relaxation factor 
*         f(n,m) Right hand side function 
*         u(n,m) Dependent variable/Solution
*         tol    Tolerance for iterative solver 
*         maxit  Maximum number of iterations 
*
* Output : u(n,m) - Solution 
*****************************************************************
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>


int get_cache_line_size();

void jacobi (
             int n, 
             int m, 
             double dx, 
             double dy, 
             double alpha, 
             double omega, 
             double *u, 
             double *f, 
             double tol, 
             int maxit )

{
  int i,j,k,BFj, BFi;
  double error, resid, ax, ay, b;
  double *uold;

  int cache_line_size=get_cache_line_size();
  int align=cache_line_size/sizeof(int);

  //BFj=m/sqrt(omp_get_max_threads())/4;
  BFj=1;
  
  BFi=n/sqrt(omp_get_max_threads())/4;
  BFi=(BFi+align-1)/align*align;
  
  uold = (double *) malloc(sizeof(double)*n*m);
  if (!uold){
    fprintf(stderr, "Error: cant allocate memory\n");
    exit(1);
  }

  ax = 1.0/(dx * dx); /* X-direction coef */
  ay = 1.0/(dy*dy); /* Y_direction coef */
  b = -2.0/(dx*dx)-2.0/(dy*dy) - alpha; /* Central coeff */

  error = 10.0 * tol;


  k = 1;

  while (k <= maxit && error > tol) {
    /* copy new solution into old */
    #pragma omp parallel for collapse(2)
    for (j=0; j<m; j++)
    {
      for (i=0; i<n; i++){
        UOLD(j,i) = U(j,i);
      }
    }
    /* compute stencil, residual and update */
    double error_inside = 0.0;
    double result;

    int jj,ii;
    #pragma omp parallel for reduction(+:error_inside) collapse(2) private(jj,ii,i,j,resid,result)
    for(jj=1; jj<m-1; jj+=BFj){
      for(ii=1; ii<n-1; ii+=BFi){
        int jmax=min(jj+BFj,m-1);
        int imax=min(ii+BFi,n-1);
        for (j=jj; j<jmax; j++){
          for (i=ii+1; i<imax; i+=2){
            resid =(
                ax * (UOLD(j,i-2) + UOLD(j,i))
                + ay * (UOLD(j-1,i-1) + UOLD(j+1,i-1))
                + b * UOLD(j,i-1) - F(j,i-1)
                ) / b;
        
            U(j,i-1) = UOLD(j,i-1) - omega * resid;
            result=resid*resid;
            error_inside += result;

            resid =(
                ax * (UOLD(j,i-1) + UOLD(j,i+1))
                + ay * (UOLD(j-1,i) + UOLD(j+1,i))
                + b * UOLD(j,i) - F(j,i)
                ) / b;
        
            U(j,i) = UOLD(j,i) - omega * resid;
            result=resid*resid;
            error_inside += result;
            
          }
        }
      }
    }
   
    /* error check */
    k++;
    error = sqrt(error_inside) /(n*m);

  } /* while */

  printf("Total Number of Iterations %d\n", k);
  printf("Residual                   %.15g\n", error);
}  


/*
************************************************************
* program to solve a finite difference 
* discretization of Helmholtz equation :  
* (d2/dx2)u + (d2/dy2)u - alpha u = f 
* using Jacobi iterative method. 
*
* Modified: Sanjiv Shah,       Kuck and Associates, Inc. (KAI), 1998
* Author:   Joseph Robicheaux, Kuck and Associates, Inc. (KAI), 1998
* 
* Directives are used in this code to achieve paralleism. 
* All do loops are parallized with default 'static' scheduling.
* 
* Input :  n - grid dimension in x direction 
*          m - grid dimension in y direction
*          alpha - Helmholtz constant (always greater than 0.0)
*          tol   - error tolerance for iterative solver
*          relax - Successice over relaxation parameter
*          mits  - Maximum iterations for iterative solver
*
* On output 
*       : u(n,m) - Dependent variable (solutions)
*       : f(n,m) - Right hand side function 
**************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <omp.h>
int n, m, mits;
double tol, relax, alpha;

void jacobi (int n, int m, double dx, double dy, 
             double alpha, double omega, 
             double *u, double *f, 
             double tol, int maxit );


/******************************************************
* Initializes data 
* Assumes exact solution is u(x,y) = (1-x^2)*(1-y^2)
*
******************************************************/
void initialize(  
                int n,    
                int m,
                double alpha,
                double *dx,
                double *dy,
                double *u,
                double *f)
{
  int i,j,xx,yy;

  *dx = 2.0 / (n-1);
  *dy = 2.0 / (m-1);

  /* Initilize initial condition and RHS */
  #pragma omp parallel for collapse(2)
  for (j=0; j<m; j++){
    for (i=0; i<n; i++){
      xx = -1.0 + *dx * (i-1);
      yy = -1.0 + *dy * (j-1);
      U(j,i) = 0.0;
      F(j,i) = -alpha * (1.0 - xx*xx) * (1.0 - yy*yy)
                - 2.0 * (1.0 - xx*xx) - 2.0 * (1.0 - yy*yy);
    }
  }
}


/************************************************************
* Checks error between numerical and exact solution 
*
************************************************************/
void error_check(
                 int n,
                 int m,
                 double alpha,
                 double dx,
                 double dy,
                 double *u,
                 double *f)
{
  int i,j;
  double xx, yy, temp, error;

  dx = 2.0 / (n-1);
  dy = 2.0 / (m-1);
  error = 0.0;
  #pragma omp parallel for reduction(+:error) private(i,j,xx, yy, temp)
  for (j=0; j<m; j++){
    for (i=0; i<n; i++){
      xx = -1.0 + dx * (i-1);
      yy = -1.0 + dy * (j-1);
      temp = U(j,i) - (1.0 - xx*xx) * (1.0 - yy*yy);
      error += temp*temp;
    }
  }

  error = sqrt(error)/(n*m);

  printf("Solution Error : %g\n", error);

}

int get_cache_line_size(){
  FILE *fp;
  char var[4];
  fp = popen("getconf LEVEL1_DCACHE_LINESIZE", "r");
  fgets(var, sizeof(var), fp);
  pclose(fp);
  return  atoi(var);
}



int main(int argc, char* argv[]){
    double *u, *f, dx, dy;
    double r1;
    //Read info 
    
    printf("Input n,m - grid dimension in x,y direction :\n ");
    scanf("%d,%d", &n, &m);
    printf("Input alpha - Helmholts constant : \n");
    scanf("%lf", &alpha);
    printf("Input relax - Successive over-relaxation parameter:\n ");
    scanf("%lf", &relax);
    printf("Input tol - error tolerance for iterrative solver:\n");
    scanf("%lf", &tol);
    printf("Input mits - Maximum iterations for solver:\n ");
    scanf("%d", &mits);
    
    printf("-> %d, %d, %f, %f, %f, %d\n",
           n, m, alpha, relax, tol, mits);
    
    u = (double *) malloc(n*m*sizeof(double));
    f = (double *) malloc(n*m*sizeof(double));

    if (!u || !f){
      fprintf(stderr, "Error: Can't allocate memory\n");
      exit(1);
    }

    /* arrays are allocated and initialzed */
    initialize(n, m, alpha, &dx, &dy, u, f);
    

    /* Solve Helmholtz eqiation */
    r1 = omp_get_wtime();

    jacobi(n, m, dx, dy, alpha, relax, u,f, tol, mits);

    r1 = omp_get_wtime() - r1;

    printf(" elapsed time : %12.6f\n", r1);
    printf(" MFlops       : %12.6g\n",
           mits*(m-2)*(n-2)*0.000001*13 / r1);

    error_check(n, m, alpha, dx, dy, u, f);
    
    
  return 0;
}
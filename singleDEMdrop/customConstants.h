
#ifndef CUSTOMCONSTANTS_H
#define CUSTOMCONSTANTS_H

#include <cmath>

#define _2D_
#define NDIM 2
#define INCOMPRESS

const double PI = 3.14159265;

const double RMIN[NDIM] = {-1,-1};
const double RMAX[NDIM] = {1.0,2.0};
const double BMIN[NDIM] = {0,0};
const double BMAX[NDIM] = {0.5,1.5};

#define TIME_OVERIDE 0
#define START_TIME 0
#define NSTEP_OVERIDE 0
#define START_NSTEP 0

const double GAMMA = 1.4;
const double HFAC = 1.5;

const int PERIODIC[NDIM] = {0,0};

const double DENS = 1.0;
const double REFD = 1.0;
//#define QUINTIC
#define WENDLAND
//#define VISC_MORRIS
//#define HANN
#define VISC_MONAGHAN
//#define VISC_CLEARY
#define VORT_LEASTSQUARES
//#define REFERENCE_FRAME
//

#define LIQ_DEM
#define NO_ANTICLUMPING
//#define CONST_H
//#define MORRIS_SPH_BOUNDARY


const double MAXTIME = 10;
const double DAMP_STEPS = 600;
const int OUTSTEP = 1000;
const int RESTART_EVERY_N_STEPS = 100;
const int REINIT_DENS_EVERY_N_STEPS = 2000000;

const double REYNOLDS_NUMBER = 1000;
const double VREF = 1.0;
const double VMAX = VREF;
const double VISCOSITY = VREF*(BMAX[0]-BMIN[0])/REYNOLDS_NUMBER;

//const double LYAP_DT = 1.0;

const double CSFAC = 10.0;
//const double PRB = pow(SPSOUND,2)*REFD/7.0;
const double SPSOUND = CSFAC*VMAX;
const double PRB = pow(REFD/DENS,6)*pow(SPSOUND,2)*REFD/7.0;


const double DENS_DROP[NDIM] = {0,0};
const double MY_VAR_RES_DV = VMAX/8.0;

const int NX = 50;
const int NY = 150;
const double PSEP = (BMAX[0]-BMIN[0])/NX;
//const double H = HFAC*PSEP;
const double H = HFAC*(BMAX[0]-BMIN[0])/NX;
const double BFAC = 1.0;

const double DEM_RADIUS = (BMAX[0]-BMIN[0])/200;
const double DEM_DENS = DENS*10.5;

const double APPROX_SMOOTH_GRID_SIZE = H;
const double SMOOTH_GRID_SIZE = (BMAX[0]-BMIN[0])/floor((BMAX[0]-BMIN[0])/APPROX_SMOOTH_GRID_SIZE+0.5);

const double GRIDSEP = PSEP;

#define MIN_ALPHA 0.1
#endif

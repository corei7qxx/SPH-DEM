#ifndef SPH_H
#define SPH_H

#ifndef PRICE2
const double PRICE2_ALPHA = 1.5;
const double PRICE2_BETA = 0.7;
#endif

const double PRICE2_A = 4.0*(25.0-pow(PRICE2_BETA,2))/(pow(PRICE2_ALPHA,2)*(pow(PRICE2_ALPHA,2)-pow(PRICE2_BETA,2)));
const double PRICE2_B = (16 + PRICE2_A*pow(PRICE2_ALPHA,4))/pow(PRICE2_BETA,4);

#ifndef MY_CUBIC
const double MY_CUBIC_ALPHA = 1.0;
#endif

const double MY_CUBIC_A = -4.0/pow(MY_CUBIC_ALPHA,2);

#ifdef _1D_
const double WCON = 1;
const double WCON_HANN = 0.25;
const double WCON_PRICE2 = 6.0/(2.0*(PRICE2_A*pow(PRICE2_ALPHA,6)+PRICE2_B*pow(PRICE2_BETA,6)+pow(2.0,6)));
#endif
#ifdef _2D_
const double WCON = 15.0/(7.0*PI);
const double WCON_QUINTIC = 7.0/(478.0*PI);
const double WCON_HANN = 1.0/(4.0*PI-16.0/PI);
const double WCON_PRICE2 = 21.0/(PI*(PRICE2_A*pow(PRICE2_ALPHA,7)+PRICE2_B*pow(PRICE2_BETA,7)+pow(2.0,7)));
const double WCON_MY_CUBIC = 10.0/(PI*(32.0+MY_CUBIC_A*pow(MY_CUBIC_ALPHA,5)));
const double WCON_WENDLAND = 7.0/(64.0*PI);
#endif
#ifdef _3D_
const double WCON = 3.0/(2.0*PI);
const double WCON_WENDLAND = 21.0/(256.0*PI);
#endif

#include "misc.h"

const double BETA_MAX = 1.5;
const double QIN  = 2.0/3.0;
const double W1 = 15.0*(2.0/3.0 - 1.0/pow(HFAC,2) + 0.5/pow(HFAC,3))/(7.0*PI);
const int NEPS = 4;
const double ALPHA = Nmisc::viscToAlpha(VISCOSITY,H,SPSOUND);

#include "demConstants.h"
#include "vect.h"
#include "particle.h"
#include "customConstants.h"
#include "globalVars.h"

#include <gsl/gsl_multifit.h>
#include <gsl/gsl_linalg.h>
#include <blitz/array.h>


namespace Nsph {
#ifdef MY_VAR_RES
      //inline vect varResDx(vect newDr,vect gradV[NDIM]) { 
      //   vect newDx;
         //TODO:this is 2d!
      //   newDx[0] = newDr[0]*(gradV[0][0]*PSEP/MY_VAR_RES_DV+1) + newDr[1]*gradV[0][1]*PSEP/MY_VAR_RES_DV;
      //   newDx[1] = newDr[1]*(gradV[1][1]*PSEP/MY_VAR_RES_DV+1) + newDr[0]*gradV[1][0]*PSEP/MY_VAR_RES_DV;
      //     return newDx;
      //}
     inline vect varResDx(vect dx,vect dv,vect gradV[NDIM]) { 
         vect newDx;
         for (int i=0;i<NDIM;i++) {
            newDx[i] = dx[i] + dot(dv,gradV[i])*pow(PSEP/MY_VAR_RES_DV,2);
         }
         return newDx;
      }
      
#endif

      inline double F(const double q,const double h) {
#ifdef WENDLAND
         if (q<=2.0) {
            return (1/pow(h,NDIM+2))*WCON_WENDLAND*(-4*pow(2-q,3)*(1+2*q) + 2*pow(2-q,4))/q;
         } else {
            return 0.0;
         }
#else
#ifdef MY_CUBIC
         if (q<=MY_CUBIC_ALPHA) {
            return -(1/pow(h,NDIM+2))*WCON_MY_CUBIC*3*(pow(2-q,2) + MY_CUBIC_A*pow(MY_CUBIC_ALPHA-q,2))/q;
         } else if (q<=2.0) {
            return -(1/pow(h,NDIM+2))*WCON_MY_CUBIC*3*pow(2-q,2)/q;
         } else {
            return 0.0;
         }
#else
#ifdef PRICE2 
         if (q<=PRICE2_BETA) {
            return -(1/pow(h,NDIM+2))*WCON_PRICE2*5*(pow(2-q,4) + PRICE2_A*pow(PRICE2_ALPHA-q,4) + PRICE2_B*pow(PRICE2_BETA-q,4))/q;
         } else if (q<=PRICE2_ALPHA) {
            return -(1/pow(h,NDIM+2))*WCON_PRICE2*5*(pow(2-q,4) + PRICE2_A*pow(PRICE2_ALPHA-q,4))/q;
         } else if (q<=2.0) {
            return -(1/pow(h,NDIM+2))*WCON_PRICE2*5*pow(2-q,4)/q;
         } else {
            return 0.0;
         }
#else
#ifdef QUINTIC
         if (q<=1.0) {
            return -(1/pow(h,NDIM+2))*WCON_QUINTIC*5*(pow(3-q,4) - 6*pow(2-q,4) + 15*pow(1-q,4))/q;
         } else if (q<=2.0) {
            return -(1/pow(h,NDIM+2))*WCON_QUINTIC*5*(pow(3-q,4) - 6*pow(2-q,4))/q;
         } else if (q<=3.0) {
            return -(1/pow(h,NDIM+2))*WCON_QUINTIC*5*pow(3-q,4)/q;
         } else {
            return 0.0;
         }
#else
#ifdef HANN
         if (q<=2.0) {
            return (1/pow(h,NDIM+2))*WCON_HANN*0.5*PI*sin(0.5*PI*(q-2.0))/q;
         } else {
            return 0.0;
         }
#else
         if (q<=1.0) {
             return (1/pow(h,NDIM+2))*WCON*(-2.0+ 1.5*q);
         }
         else if (q<=2.0) {
            return -(1/pow(h,NDIM+2))*3.0*(WCON/6.0)*pow(2.0-q,2)/q;
         }
         else {    
            return 0.0; 
         }
#endif
#endif
#endif
#endif
#endif
      }

      inline double dKdq(const double q,const double h) {
#ifdef WENDLAND
         if (q<=2.0) {
            return WCON_WENDLAND*(-4*pow(2-q,3)*(1+2*q) + 2*pow(2-q,4));
         } else {
            return 0.0;
         }
#else
#ifdef MY_CUBIC
         if (q<=MY_CUBIC_ALPHA) {
            return -WCON_MY_CUBIC*3*(pow(2-q,2) + MY_CUBIC_A*pow(MY_CUBIC_ALPHA-q,2));
         } else if (q<=2.0) {
            return -WCON_MY_CUBIC*3*pow(2-q,2);
         } else {
            return 0.0;
         }
#else
#ifdef PRICE2 
         if (q<=PRICE2_BETA) {
            return -WCON_PRICE2*5*(pow(2-q,4) + PRICE2_A*pow(PRICE2_ALPHA-q,4) + PRICE2_B*pow(PRICE2_BETA-q,4));
         } else if (q<=PRICE2_ALPHA) {
            return -WCON_PRICE2*5*(pow(2-q,4) + PRICE2_A*pow(PRICE2_ALPHA-q,4));
         } else if (q<=2.0) {
            return -WCON_PRICE2*5*pow(2-q,4);
         } else {
            return 0.0;
         }
#else
#ifdef QUINTIC
         if (q<=1.0) {
            return -WCON_QUINTIC*5*(pow(3-q,4) - 6*pow(2-q,4) + 15*pow(1-q,4));
         } else if (q<=2.0) {
            return -WCON_QUINTIC*5*(pow(3-q,4) - 6*pow(2-q,4));
         } else if (q<=3.0) {
            return -WCON_QUINTIC*5*pow(3-q,4);
         } else {
            return 0.0;
         }
#else
#ifdef HANN
         if (q<=2.0) {
            return WCON_HANN*0.5*PI*sin(0.5*PI*(q-2.0));
         } else {
            return 0.0;
         }
#else
         if (q<=1.0) {
            double q2 = pow(q,2);
            return WCON*(-2.0*q + 1.5*q2);
         }
         else if (q<=2.0) {
            return -3.0*(WCON/6.0)*pow(2.0-q,2);
         }
         else {    
            return 0.0; 
         }
#endif
#endif
#endif
#endif
#endif
      }
      
      inline double K(const double q,const double h) {
#ifdef WENDLAND
         if (q<=2.0) {
            return WCON_WENDLAND*pow(2-q,4)*(1+2*q);
         } else {
            return 0.0;
         }
#else
#ifdef MY_CUBIC
         if (q<=MY_CUBIC_ALPHA) {
            return WCON_MY_CUBIC*(pow(2-q,3) + MY_CUBIC_A*pow(MY_CUBIC_ALPHA-q,3));
         } else if (q<=2.0) {
            return WCON_MY_CUBIC*pow(2-q,3);
         } else {
            return 0.0;
         }
#else
#ifdef PRICE2 
         if (q<=PRICE2_BETA) {
            return WCON_PRICE2*(pow(2-q,5) + PRICE2_A*pow(PRICE2_ALPHA-q,5) + PRICE2_B*pow(PRICE2_BETA-q,5));
         } else if (q<=PRICE2_ALPHA) {
            return WCON_PRICE2*(pow(2-q,5) + PRICE2_A*pow(PRICE2_ALPHA-q,5));
         } else if (q<=2.0) {
            return WCON_PRICE2*pow(2-q,5);
         } else {
            return 0.0;
         }
#else
#ifdef QUINTIC
         if (q<=1.0) {
            return WCON_QUINTIC*(pow(3-q,5) - 6*pow(2-q,5) + 15*pow(1-q,5));
         } else if (q<=2.0) {
            return WCON_QUINTIC*(pow(3-q,5) - 6*pow(2-q,5));
         } else if (q<=3.0) {
            return WCON_QUINTIC*pow(3-q,5);
         } else {
            return 0.0;
         }
#else
#ifdef HANN
         if (q<=2.0) {
            return WCON_HANN*(1.0-cos(0.5*PI*(q-2.0)));
         } else {
            return 0.0;
         }
#else
         if (q<=1.0) {
            double q2 = pow(q,2);
            double q3 = q*q2;
            return WCON*(2.0/3.0 - q2 + 0.5*q3);
         }
         else if (q<=2.0) {
            return (WCON/6.0)*pow(2.0-q,3);
         }
         else {    
            return 0.0;
         }
#endif
#endif
#endif
#endif
#endif
      }

   inline double K3d(const double q,const double h) {
#ifdef WENDLAND
         if (q<=2.0) {
            return (21.0/(256.0*PI))*pow(2-q,4)*(1+2*q);
         } else {
            return 0.0;
         }
#else
         if (q<=1.0) {
            double q2 = pow(q,2);
            double q3 = q*q2;
            return 3.0*(2.0/3.0 - q2 + 0.5*q3)/(2.0*PI);
         }
         else if (q<=2.0) {
            return (1.0/(4.0*PI))*pow(2.0-q,3);
         }
         else {    
            return 0.0;
         }
#endif
      }

      inline double pdKh(const double q,const double h) {
#ifdef WENDLAND
         if (q<=2.0) {
            return -WCON_WENDLAND*q*(-4*pow(2-q,3)*(1+2*q) + 2*pow(2-q,4))/h;
         } else {
            return 0.0;
         }
#else
         if (q<=1.0) {
             return -WCON*(-2.0 + 1.5*q)*pow(q,2)/h;
         }
         else if (q<=2.0) {
            return 3.0*(WCON/6.0)*pow(2.0-q,2)*q/h;
         }
         else {    
            return 0.0; 
         }
#endif
      }


      inline double W(const double q,const double h) {
         return (1/pow(h,NDIM))*K(q,h);
      }
      
      inline double pdWh(const double q,const double h) {
         return (1/pow(h,NDIM))*pdKh(q,h)-NDIM/pow(h,NDIM+1)*K(q,h);
      }

      inline double W3d(const double q,const double h) {
         return (1/pow(h,3))*K3d(q,h);
      }

      inline double W_MLS(const vect dx,const double Wab,const double b0,const vect bRest) {
         return (b0 + dot(bRest,dx))*Wab;
      }

      inline vect gradW(const Cparticle &pa, const Cparticle &pb, const vect dr, const double q, const double h) {
#ifdef CORRECTED_GRADIENT
         const vect ret = product(pa.invM,(dr*F(q,h)-pa.sumGrad/pa.sum)/pa.sum);
#else
         const vect ret = dr*F(q,h);
#endif
#ifdef VAR_H_CORRECTION
         return ret/pa.omega;
#else
         return ret;
#endif
      }

      inline void calcB_MLS(Cparticle &p,vector<Cparticle *> &neighbrs,double &b0,vect &bRest,vector<double> &vWab) {
         int n = neighbrs.size();

         vWab.resize(n);
         gsl_matrix *matA = gsl_matrix_alloc(NDIM+1,NDIM+1);
         gsl_matrix_set_all(matA,0.0);
         gsl_vector *dxExt = gsl_vector_alloc(NDIM+1);
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            vect dx = p.r-pn->r;
            double hav = 0.5*(p.h+pn->h);
            vWab[i] = W(len(dx)/hav,hav);
            gsl_vector_set(dxExt,0,1.0);
            for (int j=0;j<NDIM;j++) {
               gsl_vector_set(dxExt,j+1,dx[j]);
            }
            for (int j=0;j<NDIM+1;j++) {
               for (int k=0;k<NDIM+1;k++) {
                  gsl_matrix_set(matA,j,k,gsl_matrix_get(matA,j,k)+gsl_vector_get(dxExt,j)*gsl_vector_get(dxExt,k)*vWab[i]);
               }
            }
         }
         gsl_matrix *invA = gsl_matrix_alloc(NDIM+1,NDIM+1);
         gsl_permutation *perm = gsl_permutation_alloc(NDIM+1);
         int tmp;
         gsl_linalg_LU_decomp(matA,perm,&tmp);

         gsl_linalg_LU_invert(matA,perm,invA);

         b0 = gsl_matrix_get(invA,0,0);
         for (int i=0;i<NDIM;i++) {
            bRest[i] = gsl_matrix_get(invA,0,i+1);
         }
         gsl_matrix_free(matA);
         gsl_matrix_free(invA);
         gsl_permutation_free(perm);
         gsl_vector_free(dxExt);
      }

#ifdef CORRECTED_GRADIENT
      inline void calcInvM(Cparticle &p,vector<Cparticle *> &neighbrs,CglobalVars &g) {
         p.invM = 0.0;
         p.sum = (p.mass/p.dens)*W(0,p.h);
         p.sumGrad = 0.0;
         int n = neighbrs.size();
         if (n == 0) {
            for (int i=0;i<NDIM;i++) p.invM[NDIM*i+i] = 1.0;
            return;
         }
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam==sph)||(pn->iam==sphBoundary)) {
               vect dr = p.r-pn->r;
               double r = len(dr);
               const double dvol = pn->mass/pn->dens;
               double hav = 0.5*(p.h+pn->h);
               const double scaleR = r/hav;
               const double Fa = F(scaleR,hav);
               const double Wab = W(scaleR,hav);
               p.sum += dvol*Wab;
               p.sumGrad += dvol*dr*Fa;
            }
         }  
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam==sph)||(pn->iam==sphBoundary)) {
               vect dr = p.r-pn->r;
               double r = len(dr);
               const double dvol = pn->mass/pn->dens;
               double hav = 0.5*(p.h+pn->h);
               const double scaleR = r/hav;
               const double Fa = F(scaleR,hav);
               const vect gradW = (dr*Fa-p.sumGrad/p.sum)/p.sum;
               p.invM -= (pn->mass/pn->dens)*outerProduct(gradW,dr);
            }
         }
         //if (p.tag ==2101) cout<<"M = "<<p.invM<<endl;
         inverseSym(p.invM);
         //if (p.tag ==2101) cout<<"M-1 = "<<p.invM<<endl;
         //   cout <<sum<<' '<<p.invM<<endl;
      }
#endif

#ifdef VAR_H_CORRECTION
      inline void calcOmega(Cparticle &p,vector<Cparticle *> &neighbrs,CglobalVars &g) {
         p.omega = p.mass*pdWh(0,p.h);
         int n = neighbrs.size();
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam==sph)||(pn->iam==sphBoundary)) {
               vect dr = p.r-pn->r;
               double r = len(dr);
               double hav = 0.5*(p.h+pn->h);
               p.omega += pn->mass*pdWh(r/hav,hav);
            }
         }  
         p.omega = 1+p.h*p.omega/(p.dens*NDIM);
      }
#endif




      /*
      inline void calc_aom(Cparticle &pa,Cparticle &pb,CglobalVars &g) {
         double qa = len(pa.r-pb.r)/pa.h;
         pa.aom -= (1.0/(NDIM*pow(HFAC,NDIM)*pa.mass))*pb.mass*(dKdq(qa,pa.h)*qa+NDIM*K(qa,pa.h));
         //cout << "aom increment = "<<(1.0/(NDIM*pow(HFAC,NDIM)*pa.mass))*pb.mass*(Csph::dKdq(qa,pa.h)*qa+NDIM*Csph::K(qa,pa.h))<<endl;
         //cout << "right most bit = "<<(Csph::dKdq(qa,pa.h)*qa+NDIM*Csph::K(qa,pa.h))<<endl;
         //pa.aom -= (1/NDIM)*(1/pow(HFAC,NDIM))*(pb.mass/pa.mass)*(Csph::dKdq(qa,pa.h)*qa+NDIM*Csph::K(qa,pa.h));
         //pa.aom = 1;
      }
      */

      /*
      inline void init_aom(Cparticle &p,CglobalVars &g) {
         p.aom= 1;
         calc_aom(p,p,g);
      }
      */

      

      /*
      inline void calc_dhdt_and_dalphdt(Cparticle &p,CglobalVars &g) {
         double graddotv = -p.dddt/p.dens;
         p.dhdt = 0.5*p.h*graddotv;
         double tou = p.h/(0.1*p.maxvsig);
         double s = max(-graddotv,0.0)*(2.0-p.alpha);
         p.dalphdt = -(p.alpha-MIN_ALPHA)/tou + s;
      }
      */

      inline double courantCondition(const double h, const double vsig) {
         //return 0.8*h/vsig; 
#ifdef HALF_COURANT
         return 0.3*h/vsig; 
#else
         return 0.6*h/vsig; 
#endif
      }

      inline double viscDiffusionCondition(const double h, const double viscosity) {
#ifdef VISC_MONAGHAN
         return 0.06*pow(h,2)/viscosity;
#else
         return 0.125*pow(h,2)/viscosity;
#endif
      }
      
      inline double accelCondition(const double h, const double accel) {
         return 0.3*sqrt(h/accel);
      }

      inline void accelCondition(Cparticle &pa, CglobalVars &g) {
         g.newDt = min(g.newDt,0.25*sqrt(pa.h/len(pa.f)));
      }


      inline void calcViscEnergy(Cparticle &pa, Cparticle &pb, CglobalVars &g, const vect fv) {
         if (pb.iam == sphBoundary) {
            pa.deViscBdt -= dot(pa.v,fv);
         } else {
            pa.deViscFdt -= dot(pa.v,fv);
         }
      }

      inline void calcViscForce(Cparticle &pa,Cparticle &pb,CglobalVars &g,const vect gradWa, const vect dv,const double r) { 
         vect dx = pa.r-pb.r;
         double vdr = dot(dx,dv);
         double hav = 0.5*(pa.h+pb.h);
         vect fv = 0.0;
         double vsig = 0.0;
#ifdef VISC_MORRIS
         double visc = VISCOSITY*(pa.dens+pb.dens)/(pa.dens*pb.dens);
#ifdef SLK
         vsig = 2.0*SPSOUND*r/len(pa.currR-pb.currR);
#else
         vsig = 2.0*SPSOUND;
#endif
#ifdef LIQ_DEM
         visc = 2.0*VISCOSITY*DENS/(pa.dens*pb.dens);
         //vsig *= 0.5/sqrt(pa.porosity)+0.5/sqrt(pb.porosity);
#endif
         fv = dv*pb.mass*visc*dot(dx,gradWa)/(pow(r,2)+0.01*pow(hav,2));
#endif
#ifdef VISC_CLEARY 
         double visc = 19.8*VISCOSITY*vdr/((pa.dens+pb.dens)*(pow(r,2)+0.01*pow(hav,2)));
         vsig = 2.0*SPSOUND;
#ifdef LIQ_DEM
         //vsig *= 0.5/sqrt(pa.porosity)+0.5/sqrt(pb.porosity);
#endif
         fv = pb.mass*visc*gradWa;
#endif
#ifdef VISC_MONAGHAN
#ifdef SLK
         double dr = 0.0;
         vect newDx = pa.currR-pb.currR;
         double newVdr = dot(newDx,dv);
         double newR = len(newDx);
         if (newR!=0.0) dr = 1/newR;
         double viss = newVdr*dr;
         vsig = 2.0*(SPSOUND + abs(viss))*r/len(pa.currR-pb.currR);
#else
         double dr = 0.0;
         if (r!=0.0) dr = 1/r;
         double viss = vdr*dr;
#ifdef LIQ_DEM
         //double vsig = SPSOUND*(1.0/sqrt(pa.porosity)+1.0/sqrt(pb.porosity)) + 2.0*abs(viss);
#else
         vsig = 2.0*SPSOUND + 2.0*abs(viss);
#endif
#endif
         //cout << "abs(viss) = "<<abs(viss)<<" avspsound = "<<0.5*(pa.spsound+pb.spsound)<<endl;
         //double vsig = 2.0*SPSOUND;
         //if (vsig > pa.maxvsig) pa.maxvsig = vsig;
         double visc = viss*vsig*ALPHA/(pa.dens+pb.dens);
         fv = pb.mass*visc*gradWa;
#endif
#ifdef VISC_ARTIFICIAL
         double artificial_vsig = 0;
         double artificial_visc = 0.0;
         if (vdr<0) {
            double dr = 0.0;
            if (r!=0.0) dr = 1/r;
            double viss = vdr*dr;
           // vsig = SPSOUND*(1.0/sqrt(pa.porosity)+1.0/sqrt(pb.porosity) - 4.0*viss);
            artificial_vsig = 2.0*SPSOUND - 4.0*viss;
            artificial_visc = viss*artificial_vsig*ALPHA_ARTIFICIAL/(pa.dens+pb.dens);
         }
         pa.f += pb.mass*artificial_visc*gradWa;
         vsig = max(vsig,artificial_vsig);
#endif
 

         g.newDt = min(g.newDt,courantCondition(hav,vsig));
         //g.newDt = min(g.newDt,courantCondition(hav,2*SPSOUND));
         g.newDt = min(g.newDt,viscDiffusionCondition(hav,VISCOSITY));

         pa.fv += fv;
         pa.f += fv;

         //calcViscEnergy(pa,pb,g,fv);
      }
      
      inline void calcElasticEnergy(Cparticle &pa, Cparticle &pb, CglobalVars &g, const double Fa, const double antic) {
         double vdr = dot(pa.v-pb.v,pa.r-pb.r);
         double ufac = vdr*Fa;
         double dudtinc = pb.mass*(pa.pdr2+0.5*antic)*ufac;
         pa.dudt += dudtinc;
         //if (pb.iam == sphBoundary) {
         //   dudtinc = pa.mass*(pb.pdr2+0.5*antic)*ufac;
         //   pb.dudt += dudtinc;
         //}
      }
      
      inline void calcBoundaryEnergy(Cparticle &pa, Cparticle &pb, CglobalVars &g, const vect fb) {
         pa.deBForcedt -= dot(pa.v,fb);
      }

#ifdef BACKGROUND_PRESSURE_FORCE
      inline void addBackgroundPressure(Cparticle &pa,CglobalVars &g) { 
	 for (int i=0;i<NDIM;i++) {
            pa.fp[i] += BGP_ACCEL[i];
            pa.f[i] += BGP_ACCEL[i];
	 }
      }
#endif

      inline void calcPressForce(Cparticle &pa, Cparticle &pb,CglobalVars &g, const vect gradWa, const vect gradWb,const double kdwPowNeps) { 
         vect dx = pa.r-pb.r;
#ifdef DDENS_VARIANT
         vect prfac = 2.0*sqrt(pa.press*pb.press)/(pa.dens*pb.dens)*gradWa;
#else
         vect prfac = (pa.pdr2*gradWa + pb.pdr2*gradWb);
#endif
         vect antic = 0.01*abs(pa.pdr2+pb.pdr2)*kdwPowNeps;
#ifdef DIRECT_SMOOTHING
         prfac = prfac + antic - (EPSILON/2.0)*len2(pa.v-pb.v)/DENS;
#else
         prfac = prfac + antic;
#endif
         //if ((pa.tag == 1)&&(pb.tag == 11)) cout << "prfac = "<<prfac<<" antic = "<<antic<<" dens = "<<pa.dens<<" press = "<<pa.press<<" dens b = "<<pb.dens<<" press b = "<<pb.press<<endl;
         //if ((pa.tag == 1)&&(pb.tag == 11)) cout << "pb.mass = "<<pb.mass<<" gradWa = "<<gradWa <<endl;
         vect fp = -pb.mass*prfac;
         //if ((pa.tag == 1)&&(pb.tag == 11)) {
         //   cout << "fp = "<<fp<<" pa.pdr2 = "<<pa.pdr2<<" pb.pdr2 = "<<pb.pdr2<<" pb.iam = "<<pb.iam<<" p.y = "<<pb.r[1]<<" b.tag == "<<pb.tag<<"pb.r = "<<pb.r<<" pa.r = "<<pa.r<<endl;
         //}

#ifdef DENS_DIFFUSE_PRESS
         double r = len(dx);
         vect dv = pa.v-pb.v;
         double vdr = dot(dv,dx);
         double dr = 0.0;
         if (r!=0.0) dr = 1/r;
         double viss = vdr*dr;
         //cout << "abs(viss) = "<<abs(viss)<<" avspsound = "<<0.5*(pa.spsound+pb.spsound)<<endl;
         double vsig = 2.0*SPSOUND + 2.0*abs(viss);
         //double vsig = 2.0*SPSOUND;
         //if (vsig > pa.maxvsig) pa.maxvsig = vsig;

         double pressDiff = 2.0*ALPHA*vsig*(pa.dens-pb.dens)*(pa.pdr2-pb.pdr2)/(pa.dens+pb.dens);
         fp -= pb.mass*pressDiff*gradWa; 
#endif
         pa.fp += fp;
         pa.f += fp;
      }

      inline void calcNormal(Cparticle &pa, Cparticle &pb,const vect &dx,const double &r,vect &normp,vect &normt) {
         if (all(pb.norm2==0)) {
            normp = pb.norm1;
#ifdef _3D_
            normt = pb.norm3;
#endif
            return;
         }
 
         float rdot1 = dot(dx,pb.norm1);
         float rdot2 = dot(dx,pb.norm2);
         float rntest = r*r*dot(pb.norm1,pb.norm2) - rdot1*rdot2;
         if (rntest <= 0) {
            //use radial vector
            normp = dx/r;
            //This assumes that pb.norm3 is parallel to the edge
            //normt = cross(normp,pb.norm3);
#ifdef _3D_
            normt = pb.norm3;
#endif
            return;
         } else {
            //use either normal
            int con;
            if (pb.concave) {
               con = 1;
            } else {
               con = -1;
            }
            if (con*(rdot1-rdot2) > 0) {
               normp = pb.norm1;
#ifdef _3D_
               normt = pb.norm3;
#endif
               return;
            } else {
               normp = pb.norm2;
#ifdef _3D_
               normt = pb.norm3;
#endif
               return;
            }
         }
      }
      
      inline void calcMorrisDv(Cparticle &pa, Cparticle &pb,const vect &norm,const vect &dx,vect &dv) {
         double dista = dot(dx,norm)-pb.dist;
         //dv = min(BETA_MAX,1.0+pb.dist/dista)*pa.v;
         //in this case pb.v is the velocity of the boundary
         dv = (1.0+pb.dist/dista)*(pa.v-pb.v);
      }

      inline void calcRadialBoundaryForces2D(Cparticle &p, Cparticle &bp,CglobalVars &g) {
         
         const double bforce = 1;
         vect fb = 0.02*pow(SPSOUND,2)*bforce/(p.mass+bp.mass);
         p.f += fb;
         p.fb += fb;
      }

      inline void calcBoundaryForces(Cparticle &p, Cparticle &bp,CglobalVars &g,const vect normp, const vect normt,const double kdwPowNeps) {
         vect dx = p.r-bp.r;
         vect dv = p.vhat-bp.vhat;
         double rperp = dot(normp,dx);
         vect rtang = dx - rperp*normp;
         double ptang[2];
         double pdist[2];
         const double rDelP = 1.0/(BFAC*PSEP);
#ifdef _3D_
         ptang[0] = abs(dot(rtang,normt));
         ptang[1] = len(cross(rtang,normt));
#else
         ptang[0] = len(rtang);
         ptang[1] = 0;
#endif
         pdist[0] = ptang[0]*rDelP;
         pdist[1] = ptang[1]*rDelP;
         if ((pdist[0] <= 1.0)&&(pdist[1] <= 1.0)) { 
            const double pesky = 0.25*(1+cos(PI*pdist[0]))*(1+cos(PI*pdist[1]));
            double hav = 0.5*(p.h+bp.h);
#ifdef MY_VAR_RES
            vect gradV[NDIM]; 
            for (int i=0;i<NDIM;i++) {
               gradV[i]=0.5*(p.gradV[i]+bp.gradV[i]);
            }
            //TODO:This is 2D
            vect newDv;
            newDv[0] = dx[0]*gradV[0][0] + dx[1]*gradV[1][0];
            newDv[1] = dx[0]*gradV[0][1] + dx[1]*gradV[1][1];
            double q = (abs(rperp)+len(newDv))/hav;
#else
            //double q = abs(rperp)/hav;
#endif
            /*
            double bfact = 1.0/abs(rperp);
            double bforce = 0.0;
            if (q <= QIN) {
               bforce = bfact*QIN;
            } else if (q <= 1.0) {
               bforce = bfact*(2.0*q - 1.5*q*q);
            } else if (q <= 2.0) {
               bforce = bfact*0.5*pow(2.0-q,2);
            }

            int srperp = 1;
            if (rperp < 0) srperp = -1;

            double coef = 0.01*pow(SPSOUND,2)*(1.0+kdwPowNeps);
            double amassrat = 2.0*p.mass/(p.mass+bp.mass);
            double bff = coef*bforce*pesky*amassrat;
            vect fb = srperp*bff*norm;
            */

            //double q = abs(rperp)/(2.0*hav);
            double q = abs(rperp)/hav;
            vect fb = 0.0;
            if (q < 2.0) {

               double epsilon = 1;
#ifdef MODIFY_BFORCE_WITH_STILL_LEVEL
               epsilon = 0;
#ifdef _3D_
               double vert = bp.r[2]-STILL_LEVEL;
#else
               double vert = bp.r[1]-STILL_LEVEL;
#endif
               if (vert > 0) {
                  epsilon = 0.02;
               } else if (vert>-STILL_LEVEL) {
                  epsilon = abs(vert/STILL_LEVEL) + 0.02;
               } else {
                  epsilon = 1.0;
               }

               double normVel = dot(dv,normp);
               if (normVel > 0) {
                  epsilon += 0;
               } else if (normVel > -SPSOUND/20.0) {
                  epsilon += -20.0*normVel/SPSOUND;
               } else {
                  epsilon += 1.0;
               }
               const double bigA = 0.01*pow(SPSOUND,2);
#else
               int beta = 0;
               if (dot(dx,dv) < 0) beta = 1;
               const double bigA = 0.01*pow(SPSOUND,2) - beta*SPSOUND*dot(dv,normp);
#endif

               double bfact = 1.0/abs(rperp);
               //double bfact = 1.0/hav;
               double bforce = 0.0;
               if (q <= QIN) {
                  bforce = bfact*QIN;
               } else if (q <= 1.0) {
                  bforce = bfact*(2.0*q - 1.5*q*q);
               } else if (q <= 2.0) {
                  bforce = bfact*0.5*pow(2.0-q,2);
               }
               const double bigR = bigA*bforce;
               //const double bigR = bigA*(1-0.5*q)/(hav*sqrt(0.5*q));
               const double amassrat = 2.0*p.mass/(p.mass+bp.mass);
               int sign = 1;
               if (rperp<0) {
                  sign = -1;
               }
               fb = sign*amassrat*normp*bigR*pesky*epsilon;
               //if (len(fb)>4000) {
               //   cout <<"fb = "<<fb<<"pb.r = "<<bp.r<<"bp.norm1 = "<<bp.norm1<<"pb.norm2 = "<<bp.norm2<<"bp.norm3 = "<<bp.norm3<<" normp = "<<normp<<" normt = "<<normt<<endl;
               //}
               
            }

            p.f += fb;
            p.fb += fb;

            //p.debfor += -bff*(bp.norm[0]*p.v[0] + bp.norm[1]*p.v[1]);
            //p.minDt = min(p.minDt,0.5*abs(rperp)/sqrt(coef));

            //g.newDt = min(g.newDt,0.5*abs(rperp)/sqrt(coef));
            //g.newDt = min(g.newDt,0.5*abs(rperp)/sqrt(bigA));

            //if (p.tag==2204) cout << "norm = "<<norm<<" pdist = "<<pdist<<" fb = "<<fb<<" p.r = "<<p.r<<" bp.r = "<<bp.r<<endl;
         }
      }

      inline void calcBoundaryForces2D(Cparticle &p, Cparticle &bp,CglobalVars &g,const vect norm, const double kdwPowNeps) {
         vect dx = p.r-bp.r;
         double rperp = norm[0]*dx[0] + norm[1]*dx[1];
         double rtang = norm[0]*dx[1] - norm[1]*dx[0];
         double pdist = abs(rtang/BFAC*PSEP);
         if (pdist <= 1.0) { 
            double pesky = 1.0-pdist;
            double hav = 0.5*(p.h+bp.h);
#ifdef MY_VAR_RES
            vect gradV[NDIM]; 
            for (int i=0;i<NDIM;i++) {
               gradV[i]=0.5*(p.gradV[i]+bp.gradV[i]);
            }
            //TODO:This is 2D
            vect newDv;
            newDv[0] = dx[0]*gradV[0][0] + dx[1]*gradV[1][0];
            newDv[1] = dx[0]*gradV[0][1] + dx[1]*gradV[1][1];
            double q = (abs(rperp)+len(newDv))/hav;
#else
            double q = abs(rperp)/hav;
#endif
            double bfact = 1.0/abs(rperp);
            double bforce = 0.0;
            if (q <= QIN) {
               bforce = bfact*QIN;
            } else if (q <= 1.0) {
               bforce = bfact*(2.0*q - 1.5*q*q);
            } else if (q <= 2.0) {
               bforce = bfact*0.5*pow(2.0-q,2);
            }

            int srperp = 1;
            if (rperp < 0) srperp = -1;

            double coef = 0.01*pow(SPSOUND,2)*(1.0+kdwPowNeps);
            double amassrat = 2.0*bp.mass/(bp.mass+p.mass);
            double bff = coef*bforce*pesky*amassrat;
            vect fb = srperp*bff*norm;
            p.f += fb;
            p.fb += fb;

            //p.debfor += -bff*(bp.norm[0]*p.v[0] + bp.norm[1]*p.v[1]);
            //p.minDt = min(p.minDt,0.5*abs(rperp)/sqrt(coef));
            g.newDt = min(g.newDt,0.5*abs(rperp)/sqrt(coef));
            //if (p.tag==2204) cout << "norm = "<<norm<<" pdist = "<<pdist<<" fb = "<<fb<<" p.r = "<<p.r<<" bp.r = "<<bp.r<<endl;
         }
      }


      inline void calcVortSPHSum(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         //TODO: currently only supports 2D, make it more general

         double correctionTerm = 0;
         p.vort = 0;

         for (int i=0;i<neighbrs.size();i++) {
            if (neighbrs[i]->iam == sph) {
               Cparticle *pn = neighbrs[i];
               vect dr = p.r-pn->r;
               vect dv = p.v-pn->v;
               double r2 = len2(dr);
               double r = sqrt(r2);
               double hav = 0.5*(p.h+pn->h);
               double Fa = F(r/hav,hav);

               //Here is the 2D specific line
               p.vort += pn->mass*Fa*(dv[0]*dr[1]-dv[1]*dr[0])/pn->dens;
               correctionTerm -= 0.5*pn->mass*r2*Fa/pn->dens;
            }
         }
         
         if (correctionTerm != 0) p.vort /= correctionTerm;
      }
#ifdef SLK
      inline void calcGLeastSquares(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {

         double chisq;

         int n = neighbrs.size();

         gsl_matrix *X = gsl_matrix_alloc(n,NDIM);
         gsl_vector *y = gsl_vector_alloc(n);

         gsl_vector *c = gsl_vector_alloc(NDIM);
         gsl_matrix *cov = gsl_matrix_alloc(NDIM,NDIM);
            
         gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(n,NDIM);

         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            vect dx = pn->currR-p.currR;
            for (int j=0;j<NDIM;j++) {
               gsl_matrix_set(X,i,j,dx[j]);
            }
         }

         for (int j=0;j<NDIM;j++) {
            for (int i=0;i<n;i++) {
               Cparticle *pn = neighbrs[i];
               gsl_vector_set(y,i,pn->r[j]-p.r[j]);
            }
            
            gsl_multifit_linear(X,y,c,cov,&chisq,work);

            for (int i=0;i<NDIM;i++) {
               p.G[i*NDIM+j] = gsl_vector_get(c,i);
            }
         }
         gsl_multifit_linear_free(work);
         gsl_vector_free(c);
         gsl_vector_free(y);
         gsl_matrix_free(X);
         gsl_matrix_free(cov);
      }
#endif


      inline void calcVortLeastSquares(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         //TODO: currently only supports 2D, make it more general

         Array<double,2> coeff(NDIM,NDIM);

         double chisq;

         vector<Cparticle *> sphNeighbrs;

         for (int i=0;i<neighbrs.size();i++) {
            if ((neighbrs[i]->iam == sph)||(neighbrs[i]->iam == ghost)) sphNeighbrs.push_back(neighbrs[i]);
         }
         
         int n = sphNeighbrs.size();

         gsl_matrix *X = gsl_matrix_alloc(n,NDIM);
         gsl_vector *y = gsl_vector_alloc(n);

         gsl_vector *c = gsl_vector_alloc(NDIM);
         gsl_matrix *cov = gsl_matrix_alloc(NDIM,NDIM);
            
         gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(n,NDIM);

         for (int i=0;i<n;i++) {
            Cparticle *pn = sphNeighbrs[i];
            vect dx = pn->r-p.r;
            for (int j=0;j<NDIM;j++) {
               gsl_matrix_set(X,i,j,dx[j]);
            }
         }

         for (int j=0;j<NDIM;j++) {
            for (int i=0;i<n;i++) {
               Cparticle *pn = sphNeighbrs[i];
               gsl_vector_set(y,i,pn->v[j]-p.v[j]);
            }
            
            gsl_multifit_linear(X,y,c,cov,&chisq,work);

            for (int i=0;i<NDIM;i++) {
               coeff(j,i) = gsl_vector_get(c,i);
            }
         }
         gsl_multifit_linear_free(work);
         gsl_vector_free(c);
         gsl_vector_free(y);
         gsl_matrix_free(X);
         gsl_matrix_free(cov);

         //This is the bit restricted to 2D, should be easy to entend for 3D, but I can't be bothered
         p.vort = coeff(1,0) - coeff(0,1);
      }

#ifdef MY_VAR_RES
      inline void calcGradVLeastSquares(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {

         double chisq;

         vector<Cparticle *> sphNeighbrs;

         for (int i=0;i<neighbrs.size();i++) {
            if ((neighbrs[i]->iam == sph)||(neighbrs[i]->iam == ghost)||(neighbrs[i]->iam == sphBoundary)) sphNeighbrs.push_back(neighbrs[i]);
         }
         
         int n = sphNeighbrs.size();

         if (n>NDIM) {

            gsl_matrix *X = gsl_matrix_alloc(n,NDIM);
            gsl_vector *y = gsl_vector_alloc(n);

            gsl_vector *c = gsl_vector_alloc(NDIM);
            gsl_matrix *cov = gsl_matrix_alloc(NDIM,NDIM);
            
            gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(n,NDIM);

            for (int i=0;i<n;i++) {
               Cparticle *pn = sphNeighbrs[i];
               vect dx = pn->r-p.r;
               for (int j=0;j<NDIM;j++) {
                  gsl_matrix_set(X,i,j,dx[j]);
               }
            }

            for (int j=0;j<NDIM;j++) {
               for (int i=0;i<n;i++) {
                  Cparticle *pn = sphNeighbrs[i];
                  gsl_vector_set(y,i,pn->v[j]-p.v[j]);
               }
            
               gsl_multifit_linear(X,y,c,cov,&chisq,work);

               for (int i=0;i<NDIM;i++) {
                  p.gradV[i][j] = gsl_vector_get(c,i);
                  //p.gradV(j,i) = gsl_vector_get(c,i);
               }
            }
            gsl_multifit_linear_free(work);
            gsl_vector_free(c);
            gsl_vector_free(y);
            gsl_matrix_free(X);
            gsl_matrix_free(cov);
         } else {
            for (int i=0;i<NDIM;i++) {
               p.gradV[i] = 0;
            }
         }
      }
#endif



      inline void calcDensity(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
#ifdef REINIT_DENS_MLS
         double b0;
         vect bRest;
         vector<double> vWab;
         calcB_MLS(p,neighbrs,b0,bRest,vWab);
         int n = neighbrs.size();
         p.dens = 0;
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            p.dens += pn->mass*W_MLS(p.r-pn->r,vWab[i],b0,bRest);
         }
         vWab.~vector<double>();
#else
         double Wsum = p.mass*W(0,p.h)/p.dens;
         p.dens = p.mass*W(0,p.h);
         for (int i=0;i<neighbrs.size();i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam != sph)&&(pn->iam != sphBoundary)) continue;
            vect dr = p.r-pn->r;
            double r = len(dr);
            double hav = 0.5*(p.h+pn->h);
            double Wa = W(r/hav,hav);
            p.dens += pn->mass*Wa;
            Wsum += pn->mass*Wa/pn->dens;
         }
         //Wsum *= 4*PI*pow(p.h,2)/(neighbrs.size()+1);
         //cout << "before: dens = "<<p.dens <<" h = "<<p.h<<" Wsum = "<<Wsum<<" dens/Wsum = "<<p.dens/Wsum<<endl;
         p.dens /= Wsum;
         //cout << "dens = "<<p.dens <<" h = "<<p.h<<endl;
#endif
      }


      inline void calcVort(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         int n = neighbrs.size();
         p.vort = 0.0;
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if (pn->iam!=sph) continue;
            const vect dx = p.r-pn->r;
            const double r = len(dx);
            const vect dv = p.vhat - pn->vhat;
            const double dvol = pn->mass/pn->dens;
            const double q = r/pn->h;
            const vect gradWa = gradW(p,*pn,dx,q,pn->h);
            //fGradV += dvol*outerProduct(-dv,gradWa);
            p.vort += dvol*cross(gradWa,dv);
         }
      }
           

#ifdef LIQ_DEM
      inline double demCondition() {
         return DEM_TIMESTEP;
//#ifdef LINEAR
//         return 0.5*PI/sqrt(DEM_K/DEM_MIN_REDUCED_MASS-pow(0.5*DEM_GAMMA/DEM_MIN_REDUCED_MASS,2));
//#endif
      }
      inline double liqDemCondition() {
         return LIQ_DEM_TIMESTEP;
//#ifdef LINEAR
//         return 0.5*PI/sqrt(DEM_K/DEM_MIN_REDUCED_MASS-pow(0.5*DEM_GAMMA/DEM_MIN_REDUCED_MASS,2));
//#endif
      }


      inline void calcFPIOnFluid(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         int n = neighbrs.size();
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam != sph)&&(pn->iam != sphBoundary)) continue;
            const double r = len(p.r-pn->r);
#ifdef _2D_
#ifdef _2D_DEM
            const double rdens = 1.0/pn->dens;
#else
            const double rdens = (1.0/(2.0*DEM_RADIUS))/pn->dens;
#endif
#else
            const double rdens = 1.0/pn->dens;
#endif
            pn->fdrag -= p.mass*rdens*p.fdrag*W(r*(2.0/LIQ_DEM_COUPLING_RADIUS),LIQ_DEM_COUPLING_RADIUS/2.0)/p.shepSum;
         }
      }

      inline double calcPorosityIncr(double w) {
#ifdef _2D_
#ifdef _2D_DEM
         return PI*pow(DEM_RADIUS,2)*w;          
#else
         return (4.0/3.0)*PI*pow(DEM_RADIUS,3)*w/(2.0*DEM_RADIUS);          
#endif
#else
         return (4.0/3.0)*PI*pow(DEM_RADIUS,3)*w;          
#endif
      }
      inline void initPorosityAndDrag(Cparticle &p,CglobalVars &g) {
         p.porosity = 0.0;
         p.fdrag = 0.0;
      }

      inline void setPorosityForBoundary(Cparticle &p,CglobalVars &g) {
         p.porosity = 1.0;
      }
 
      inline void finalisePorosityAndDrag(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         p.shepSum = p.mass*W(0,LIQ_DEM_COUPLING_RADIUS/2.0)/p.dens;
         int n = neighbrs.size();
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam==sph)||(pn->iam==sphBoundary)) {
               const double r = len(p.r-pn->r);
               const double dvWab = pn->mass*W(r*(2.0/LIQ_DEM_COUPLING_RADIUS),LIQ_DEM_COUPLING_RADIUS/2.0)/pn->dens;
               p.shepSum += dvWab;
            }
         }
         p.porosity = 1.0-p.porosity/p.shepSum;
         //p.porosity = 1.0-p.porosity;
         p.f += p.fdrag;
      }

      inline void finalisePorosityAndDrag2(Cparticle &p, CglobalVars &g) {
         p.porosity = 1.0-p.porosity;
         p.f += p.fdrag;
      }

      inline void calcPorosityAndDrag(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         if (p.shepSum < 0.5) return;
         int n = neighbrs.size();
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam != sph)&&(pn->iam != sphBoundary)) continue;
            double r = len(p.r-pn->r);
 	    pn->porosity += calcPorosityIncr(W(r*(2.0/LIQ_DEM_COUPLING_RADIUS),LIQ_DEM_COUPLING_RADIUS/2.0));
         }
#ifndef LIQ_DEM_ONE_WAY_COUPLE
         calcFPIOnFluid(p,neighbrs,g);
#endif
      }

      inline void calcPorosityAndDrag2(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         p.porosity = 0;
         p.fdrag = 0;
#ifdef _2D_
#ifdef _2D_DEM
         const double rdens = 1.0/p.dens;
#else
         const double rdens = (1.0/(2.0*DEM_RADIUS))/p.dens;
#endif
#else
         const double rdens = 1.0/p.dens;
#endif
         int n = neighbrs.size();
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            double r = len(p.r-pn->r);
            const double Wcoup = W(r*(2.0/LIQ_DEM_COUPLING_RADIUS),LIQ_DEM_COUPLING_RADIUS/2.0);
            if (pn->iam == dem) {
               if (pn->shepSum> 0.5) {
                  p.porosity += calcPorosityIncr(Wcoup);
                  p.fdrag -= pn->mass*rdens*pn->fdrag*Wcoup/pn->shepSum;
               }
            } else if ((pn->iam == sph)||(pn->iam == sphBoundary)) {
               const double dvWab = pn->mass*Wcoup/pn->dens;
               p.shepSum += dvWab;
            }
         }
         p.porosity = 1.0-p.porosity/p.shepSum;
         p.f += p.fdrag;
      }

      inline void interpolatePorosityAndH(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         p.porosity = 0;
         //p.hErr = 0;
         double sum = 0;
         int n = neighbrs.size();
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if (pn->iam == dem) continue;
            double r = len(p.r-pn->r);
            double dvWab = pn->mass*W(r/pn->h,pn->h)/pn->dens;
            p.porosity += pn->porosity*dvWab;          
            //p.hErr += pn->h*dvWab;          
            sum += dvWab;
         }
         if (sum>0.5) {
            p.porosity /= sum;
            //p.hErr /= sum;
         }
         p.shepSum = sum;
         //p.porosity = p.mass*W(0,p.hErr);
         //for (int i=0;i<n;i++) {
         //   Cparticle *pn = neighbrs[i];
         //   double r = len(p.r-pn->r);
         //   if (pn->iam != dem) continue;
         //   p.porosity += pn->mass*W(r/p.hErr,p.hErr);          
         //}
         //p.porosity = 1.0 - p.porosity/DEM_DENS;
      }
      
      inline vect demNormal(const double overlap, const double overlap_dot, const vect normal) {
#ifdef HERTZ
         return (DEM_K*pow(overlap,1.5) + DEM_GAMMA*overlap_dot)*normal;
#endif
#ifdef HERTZ_KUWABARA
         return (DEM_K*pow(overlap,1.5) + DEM_GAMMA*sqrt(overlap)*overlap_dot)*normal;
#endif
#ifdef LINEAR
         return (DEM_K*overlap + DEM_GAMMA*overlap_dot)*normal;
#endif
      }
      
      inline vect demTangential(const vect tdv,const vect normal, const double reducedRadius, const double k, const double gamma, const double fStat, const double fSlid, vect &eta, bool &sliding) {
         const double absEta = len(eta);
         eta = eta - normal*dot(normal,eta);
         eta = eta * absEta/len(eta);
         const vect testForce = -k*eta - gamma*tdv;
         const double absTestForce = len(testForce);
         const vect ft = cross((vect)(-reducedRadius*normal),testForce);
         if (((!sliding)&&(absTestForce <= fStat)) || ((sliding)&&(absTestForce <= fSlid))) {
            sliding = false;
         } else {
            sliding = true;
            eta = -(1.0/k)*(fSlid*ft/len(ft) + gamma*tdv);
         }
         return ft;
      }

      inline void demTangentialKick(const double dt, const vect tdv, const bool sliding, vect &eta) {
         if (!sliding) {
            eta += tdv*dt;
         }
      }

      inline vect genFtest(const double mu, const double k,const vect fNorm, const double overlap) {
         return mu*(fNorm + k*overlap);
      }

      inline vect demWallContact(const vect dx, const vect dv, const vect normP, const vect normT) {
         double rperp = dot(normP,dx);
         vect rtang = dx - rperp*normT;
         double ptang[2];
         double pdist[2];
         const double rDelP = 1.0/(BFAC*PSEP);
#ifdef _3D_
         ptang[0] = abs(dot(rtang,normT));
         ptang[1] = len(cross(rtang,normT));
#else
         ptang[0] = len(rtang);
         ptang[1] = 0;
#endif
         pdist[0] = ptang[0]*rDelP;
         pdist[1] = ptang[1]*rDelP;
         const double overlap = 1.0*DEM_RADIUS-rperp;
         if ((pdist[0] <= 1.0)&&(pdist[1] <= 1.0)&&(overlap>0)) { 
            //const double pesky = 0.25*(1+cos(PI*pdist[0]))*(1+cos(PI*pdist[1]));
            const double overlap_dot = -dot(dv,normP);
            return demNormal(overlap,overlap_dot,normP);
         } else {
            return 0,0,0;
         }
      }

      inline vect calcParticleDrag(Cparticle &p, const vect &ff, const vect &fpfvfb, const vect &fdrag, const vect &fvel, const double &fdens, const vect &fvort) {
         double pVol = p.mass/DEM_DENS;
         vect dv = p.porosity*(fvel-p.vhat);
         const double vdiff = len(dv);
         vect fd = 0.0;
         double pmass = p.mass;
         if (vdiff!=0) {
#ifdef ERGUN
            const double beta = 150*(1-p.porosity)*VISCOSITY/(pow(2.0*DEM_RADIUS,2)*p.porosity) + (1.75*0.5/DEM_RADIUS)*vdiff;
#ifdef _2D_DEM
            fd = PI*pow(DEM_RADIUS,2)*beta*dv;
#else
            fd = (4.0/3.0)*PI*pow(DEM_RADIUS,3)*beta*dv;
#endif
#else
#ifdef LIQ_DEM_SIMPLE_DRAG
            fd = 6.0*PI*VISCOSITY*DEM_RADIUS*dv;
#else
            const double A = PI*pow(DEM_RADIUS,2);
            //const double Re = (2.0*DEM_RADIUS/VISCOSITY)*p.porosity*vdiff;
            const double Re = (2.0*DEM_RADIUS/VISCOSITY)*vdiff;
            const double dragGamma = 3.7 - 0.65*exp(-pow(1.5-log10(Re),2)/2.0);
            const double C = pow(0.63 + 4.8/pow(Re,0.5),2);
            fd = (1.0/2.0)*A*C*vdiff*dv*pow(p.porosity,-dragGamma);
#endif
#endif

#ifdef LIQ_DEM_LIFT
            const double Res = 4.0*pow(DEM_RADIUS,2)*len(fvort)/VISCOSITY; 
#ifdef LIQ_DEM_SIMPLE_DRAG
            const double corrLift = 1.0;
#else
            const double beta = 0.5*Res/Re;
            const double corrLift = 0.0524*sqrt(beta*Re);
#endif
            const double Clift = 4.1126*corrLift/sqrt(Res);
            fd += DENS*PI*pow(DEM_RADIUS,3)*Clift*cross(dv,fvort);
#endif
         }


#ifdef LIQ_DEM_ADDED_MASS
         const double coeffAM = 0.5*DEM_VOL*DENS;
         fd += coeffAM*ff;
         p.f = p.f*(p.mass/(p.mass + coeffAM));
#ifdef LIQ_DEM_ONE_WAY_COUPLE
         pmass += 0.5*DEM_VOL*DENS;
#else
         pmass += 0.5*DEM_VOL*DENS + 0.5*DEM_VOL*DEM_DENS*(1-p.porosity);
#endif
#endif
         //return (fdens/DEM_DENS)*(fp+fv) + (DENS/p.mass)*fd;
         if (p.porosity==0.0) {
            cout << "porosity zero for a dem particle experiencing drag. p.shepSum= "<<p.shepSum<<endl;
         }
         return (DENS*DEM_VOL*p.porosity/pmass)*fpfvfb + (DENS/pmass)*fd;
      }


      inline void calcDEMWallContacts(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         int n = neighbrs.size();
         Cparticle *nearestBoundary;
         double minLen = RMAX[0]-RMIN[0];
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam!=boundary)&&(pn->iam!=sphBoundary)) continue;
            cout<<"anything!!!!!!"<<endl;
            const vect dx = p.r-pn->r;
            const double r = len(dx);
            const vect dv = p.vhat - pn->vhat;
            if (r < minLen) {
               nearestBoundary = pn;
               minLen = r;
            }
         }
         if (minLen < (RMAX[0]-RMIN[0])) {
            const vect dx = p.r-nearestBoundary->r;
            const double r = len(dx);
            const vect dv = p.vhat - nearestBoundary->vhat;
#ifdef _3D_
            const vect tang = nearestBoundary->norm3;
#else
            const vect tang = 0.0;
#endif
            vect fWall = demWallContact(dx,dv,nearestBoundary->norm1,tang);
            if (any(nearestBoundary->norm2 != 0)) {
               fWall += demWallContact(dx,dv,nearestBoundary->norm2,tang);
            }

            double pmass = p.mass;
#ifdef LIQ_DEM_ADDED_MASS
            if (p.shepSum> 0.5) {
               pmass += 0.5*DEM_VOL*DENS;
            }
#endif
            fWall /= pmass;
            p.fb += fWall;       
            p.f += fWall;
         }
      }

      inline void calcDEMContacts(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         int n = neighbrs.size();
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam!=dem)&&(pn->iam!=demBoundary)) cout <<"crap crap crap"<<endl;
            const vect dx = p.r-pn->r;
            const double r = len(dx);
            const vect dv = p.vhat - pn->vhat;
            const vect normal = dx/r;
            const double overlap = 2.0*DEM_RADIUS-r;
            if (overlap>0) {
               const double overlap_dot = -dot(dv,normal);
               double pmass = p.mass;
#ifdef LIQ_DEM_ADDED_MASS
               if (p.shepSum> 0.5) {
                  pmass += 0.5*DEM_VOL*DENS;
               }
#endif
               const vect fNorm = demNormal(overlap,overlap_dot,normal)/pmass;
               //const double reducedRadius = 0.5*DEM_RADIUS; 
               //p.dVortDt += dem_tangential(tdv[0],normal,reducedRadius,DEM_K_SLIDING,DEM_GAMMA_SLIDING,genFtest(DEM_MU_STATIC,DEM_K_STATIC,fNorm,overlap),genFtest(DEM_MU_SLIDING,DEM_K_SLIDING,fNorm,overlap),p.eta[0],
               //const vect fslid = dem_tangential(dv - normal*(dot(normal,dv)),   );
               p.fp += fNorm;
               p.f += fNorm;
            }
         }
      }

      inline void calcFPI(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         double sum = 0;
         double fdens = 0;
         //vect fp = 0.0;
         vect ff = 0.0;
         vect fpfvfb = 0.0;
         //vect fv = 0.0;
         p.fvel = 0.0;
         p.fdrag = 0.0;
         vect fdrag = 0.0;
         vect fvort = 0.0;
         p.porosity = 0.0;
         p.shepSum= 0.0;
         int n = neighbrs.size();
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if (pn->iam==dem) cout <<"crap crap crpa"<<endl;
            const vect dx = p.r-pn->r;
            const double r = len(dx);
            const vect dv = p.vhat - pn->vhat;
            const double dvol = pn->mass/pn->dens;
            const double q = r/pn->h;
            const vect gradWa = gradW(p,*pn,dx,q,pn->h);
            const double Wab = W(q,pn->h);
            const double dvWab = dvol*Wab;
            //fp += dvWab*(pn->fp+pn->fb);
            fpfvfb += dvWab*(pn->fp+pn->fv+pn->fb);
            ff += dvWab*(pn->f);
            //fv += dvWab*pn->fv;
            fdrag += dvWab*pn->fdrag;
            p.fvel += dvWab*pn->vhat;
            p.porosity += dvWab*pn->porosity;
            fdens += Wab*pn->mass;
            fvort += dvol*cross(gradWa,dv);
            sum += dvWab;
            const double dvWabCouple = dvol*W(r*(2.0/LIQ_DEM_COUPLING_RADIUS),LIQ_DEM_COUPLING_RADIUS/2.0);
            p.shepSum += dvWabCouple;
         }
         if (p.shepSum>0.5) {
            double rsum = 1/sum;
            //fp *= rsum;
            //fv *= rsum;
            ff *= rsum;
            fpfvfb *= rsum;
            fdrag *= rsum;
            p.fvel *= rsum;
            fdens *= rsum;
            p.porosity *= rsum;
            p.fdrag = calcParticleDrag(p,ff,fpfvfb,fdrag,p.fvel,fdens,fvort);
            p.f += p.fdrag;
         }
      }

     
      inline void updateGvect(Cparticle &p, vector<Cparticle *> &neighbrs,CglobalVars &g) {
         vect fluidAccel = 0.0;
         int n = neighbrs.size();
         double sum = 0;
         for (int i=0;i<n;i++) {
            Cparticle *pn = neighbrs[i];
            if ((pn->iam != sph)&&(pn->iam != sphBoundary)) continue;
            const double vol = pn->mass/pn->dens;
            const double hav = 0.5*(p.h+pn->h);
            const double Wab = W(len(p.r-pn->r)/hav,hav);
            const double dvWab = Wab*vol;
            fluidAccel += dvWab*p.f;
            sum += dvWab;
         }
         if (sum!=0) {
            fluidAccel /= sum;
         } else {
            fluidAccel = 0.0;
         }
         p.gHead++;
         //cout <<" gHead = " <<p.gHead<<" fluidAccel = "<<fluidAccel<<" p.f = "<<p.f<<endl;
         if (p.gHead >= TWIN_N) p.gHead = 0;
         p.gVect[p.gHead] = fluidAccel-p.f;
      }
            
      
#endif

      inline void calcHFromDens(Cparticle &p,CglobalVars &g) {
         p.h = HFAC*pow(p.mass/p.dens,1.0/NDIM);
      }
      
      inline bool ifSph(Cparticle &p) {
         return p.iam == sph;
      }
      inline bool ifDem(Cparticle &p) {
         return p.iam == dem;
      }
      inline bool ifDemOrDemBoundary(Cparticle &p) {
         return p.iam == dem;
      }
      inline bool ifSphOrDem(Cparticle &p) {
         return (p.iam == sph)||(p.iam == dem);
      }
      inline bool ifSphOrBoundary(Cparticle &p) {
         return (p.iam == sph)||(p.iam == boundary);
      }
      inline bool ifSphOrSphBoundaryOrBoundary(Cparticle &p) {
         return (p.iam == sph)||(p.iam == boundary)||(p.iam == sphBoundary);
      }
      inline bool ifSphOrDemOrDemBoundary(Cparticle &p) {
         return (p.iam == sph)||(p.iam == dem)||(p.iam == demBoundary);
      }
      inline bool ifSphOrSphBoundaryOrDemOrDemBoundary(Cparticle &p) {
         return (p.iam == sph)||(p.iam == sphBoundary)||(p.iam == dem)||(p.iam == demBoundary);
      }
      inline bool ifSphBoundary(Cparticle &p) {
         return p.iam == sphBoundary;
      }
      inline bool ifSphOrSphBoundary(Cparticle &p) {
         return (p.iam==sph)||(p.iam==sphBoundary);
      }
      inline bool ifSphOrSphBoundaryOrDem(Cparticle &p) {
         return (p.iam==sph)||(p.iam==sphBoundary)||(p.iam==dem);
      }
      //inline bool ifNotBoundary(Cparticle &p) {
      //   return (p.iam!=boundary);
      //}
      inline bool ifSphOrSphBoundaryOrGhost(Cparticle &p) {
         return (p.iam==sph)||(p.iam==sphBoundary)||(p.iam==ghost);
      }
      inline bool ifSphOrGhost(Cparticle &p) {
         return (p.iam == sph)||(p.iam == ghost);
      }
      inline bool ifBoundary(Cparticle &p) {
         return (p.iam == boundary)||((p.iam >= boundaryobj1)&&(p.iam <= boundaryobj6));
      }
      inline bool ifBoundaryOrSphBoundary(Cparticle &p) {
         return (p.iam == boundary)||(p.iam == sphBoundary);
      }

}
	
#endif

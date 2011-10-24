#include "customConstants.h"
#include "particle.h"
#include "io_data_vtk.h"
#include "misc.h"
#include "sph.h"
#include "sphIncompress.h"
#include "customOutput.h"
#include "dataLL.h"
#include <vector>
#include <ctime>
#include <iostream>

#define CHEB_N 65

int main(int argc, char *argv[]) {
   if (argc != 2) {
      cout << "Usage: setup outfilename" << endl;
      return(-1);
   }
   string filename = argv[1];

   vector<Cparticle> ps;
   Cparticle p;
   cout << "Creating box with sides: rmax = ["<<BMAX[0]<<" "<<BMAX[1]<<"] rmin = ["<<BMIN[0]<<" "<<BMIN[1]<<"]"<<endl;
   cout << "Reynolds Number = "<<REYNOLDS_NUMBER<<endl;
   cout << "Density = "<<DENS<<endl;
   cout << "number of particles on side = "<<NX<<endl;
   cout << "alpha = "<<ALPHA<<endl;
   cout << "viscosity = "<<VISCOSITY<<endl;
   cout << "maxtime = "<<MAXTIME<<endl;
 
   double tsc = Nsph::courantCondition(H,2*SPSOUND);
   double tsv = Nsph::viscDiffusionCondition(H,VISCOSITY);
   cout <<"simulation will take "<<int((MAXTIME/tsc)+1)<<" steps according to Courant condition, "<<int((MAXTIME/tsv)+1)<<" steps according to visc diffusion condition"<<endl;

   for (int i=-3;i<=NX+3;i++) {
         cout << "\rParticle ("<<i<<","<<"0"<<"). Generation "<<((i+2)*(NY+4))/double((NX+4)*(NY+4))*100<<"\% complete"<<flush;
      for (int j=-3;j<=NY+3;j++) {
         p.tag = ps.size()+1;
         p.r = i*PSEP+BMIN[0],j*PSEP+BMIN[1];
         p.dens = DENS;
         p.mass = PSEP*PSEP*DENS;
         p.h = H;
         p.v = 0.0,0.0;

         if ((j<=0)||(j>=NY)||(i<=0)||(i>=NX)) {
            p.iam = sphBoundary;
            if (j>=NY) {
               p.v = VREF,0.0;
               p.vhat = p.v;
            }
         } else {
            p.iam = sph;
         }
         p.alpha = ALPHA;
         ps.push_back(p);
      }
   }

   cout << "Total number of particles = " << ps.size() << endl;

   CglobalVars globals;


   vector<vector<double> > vprocDomain(globals.mpiSize);
   vector<Array<int,NDIM> > vprocNeighbrs(globals.mpiSize);
   vector<particleContainer > vps;
   vectInt split;
   split = 1,1;
   particleContainer pps;
   for (int i=0;i<ps.size();i++) {
      pps.push_back(ps[i]);
   }
   Nmisc::splitDomain(pps,split,vps,vprocDomain,vprocNeighbrs);

   cout << "Opening files for writing..."<<endl;
   Cio_data_vtk ioFile(filename.c_str(),&globals);
   cout << "Calculating Output stuff.."<<endl;
   //sph.calcOutputVars();
   //customOutput.calcOutput(0,&customSim,&ioFile);
   cout << "Writing Restart data to file..."<<endl;
   int nProc = product(split);
   for (int i=0;i<nProc;i++) {
      globals.mpiRank = i;
      ioFile.setFilename(filename.c_str(),&globals);
      ioFile.writeGlobals(0,&globals);
      ioFile.writeRestart(0,vps[i],&globals);
      globals.mpiRank = 0;
   }
   cout << "Writing Global data to file..."<<endl;
   //ioFile.writeGlobals(0,&globals);
   ioFile.writeDomain(0,vprocDomain,vprocNeighbrs);


   //Write restart file for John
   /*ofstream fo("restartJohn.dat");
   fo <<"h = "<<H<<" mass = "<<ps[0].mass<<" dens = "<<ps[0].dens<<" psep = "<<PSEP<<" Re = "<<REYNOLDS_NUMBER<<" kinematic viscosity = "<<VISCOSITY<<" n = "<<ps.size()<<endl;
   fo <<"r_x r_y v_x v_y flag(0=fluid,1=boundary)"<<endl;
   for (int i=0;i<ps.size();i++) {
      fo << ps[i].r[0]<<' '<<ps[i].r[1]<<' '<<ps[i].v[0]<<' '<<ps[i].v[1]<<' '<<ps[i].iam<<endl;
   }*/



}


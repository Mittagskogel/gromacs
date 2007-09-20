/*
 * $Id$
 * 
 *                This source code is part of
 * 
 *                 G   R   O   M   A   C   S
 * 
 *          GROningen MAchine for Chemical Simulations
 * 
 *                        VERSION 3.3.2
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2007, The GROMACS development team,
 * check out http://www.gromacs.org for more information.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * If you want to redistribute modifications, please consider that
 * scientific software is very special. Version control is crucial -
 * bugs must be traceable. We will be happy to consider code for
 * inclusion in the official distribution, but derived work must not
 * be called official GROMACS. Details are found in the README & COPYING
 * files - if they are missing, get the official version at www.gromacs.org.
 * 
 * To help us fund GROMACS development, we humbly ask that you cite
 * the papers on the package - you can find them in the top README file.
 * 
 * For more info, check our website at http://www.gromacs.org
 * 
 * And Hey:
 * Groningen Machine for Chemical Simulation
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include "sysstuff.h"
#include "smalloc.h"
#include "physics.h"
#include "typedefs.h"
#include "vec.h"
#include "gmx_random.h"
#include "random.h"


void low_mspeed(real tempi,int nrdf,int nat,atom_id a[],
		t_atoms *atoms,rvec v[], gmx_rng_t rng)
{
  int  i,j,m;
  real boltz,sd;
  real ekin,temp,mass,scal;

  boltz=BOLTZ*tempi;
  ekin=0.0;
  for (i=0; (i<nat); i++) {
    j=a[i];
    mass=atoms->atom[j].m;
    if (mass > 0) {
      sd=sqrt(boltz/mass);
      for (m=0; (m<DIM); m++) {
	v[j][m]=sd*gmx_rng_gaussian_real(rng);
	ekin+=0.5*mass*v[j][m]*v[j][m];
      }
    }
  }
  temp=(2.0*ekin)/(nrdf*BOLTZ);
  if (temp > 0) {
    scal=sqrt(tempi/temp);
    for(i=0; (i<nat); i++)
      for(m=0; (m<DIM); m++)
	v[a[i]][m]*=scal;
  }
  fprintf(stderr,"Velocities were taken from a Maxwell distribution at %g K\n",
	  tempi);
  if (debug) {
    fprintf(debug,
	    "Velocities were taken from a Maxwell distribution\n"
	    "Initial generated temperature: %12.5e (scaled to: %12.5e)\n",
	    temp,tempi);
  }
}

void grp_maxwell(t_block *grp,real tempi[],int nrdf[],int seed,
		 t_atoms *atoms,rvec v[])
{
  int i,s,n;
  gmx_rng_t rng;
  bool bFirst=TRUE;

  if(bFirst) {
    bFirst=FALSE;
    rng = gmx_rng_init(seed);
  }

  for(i=0; (i<grp->nr); i++) {
    s=grp->index[i];
    n=grp->index[i+1]-s;
    low_mspeed(tempi[i],nrdf[i],n,&(grp->a[s]),atoms,v,rng);
  }
  gmx_rng_destroy(rng);
}


void maxwell_speed(real tempi,int nrdf,int seed,t_atoms *atoms, rvec v[])
{
  atom_id *dummy;
  int     i;
  gmx_rng_t rng;
  bool bFirst=TRUE;
  
  if (seed == -1) {
    seed = make_seed();
    fprintf(stderr,"Using random seed %d for generating velocities\n",seed);
  }

  if(bFirst) {
    bFirst=FALSE;
    rng = gmx_rng_init(seed);
  }

  snew(dummy,atoms->nr);
  for(i=0; (i<atoms->nr); i++)
    dummy[i]=i;
  low_mspeed(tempi,nrdf,atoms->nr,dummy,atoms,v,rng);
  sfree(dummy);
  gmx_rng_destroy(rng);
}

real calc_cm(FILE *log,int natoms,real mass[],rvec x[],rvec v[],
	     rvec xcm,rvec vcm,rvec acm,matrix L)
{
  rvec dx,a0;
  real tm,m0;
  int  i,m;

  clear_rvec(xcm);
  clear_rvec(vcm);
  clear_rvec(acm);
  tm=0.0;
  for(i=0; (i<natoms); i++) {
    m0=mass[i];
    tm+=m0;
    oprod(x[i],v[i],a0);
    for(m=0; (m<DIM); m++) {
      xcm[m]+=m0*x[i][m]; /* c.o.m. position */
      vcm[m]+=m0*v[i][m]; /* c.o.m. velocity */
      acm[m]+=m0*a0[m];   /* rotational velocity around c.o.m. */
    }
  }
  oprod(xcm,vcm,a0);
  for(m=0; (m<DIM); m++) {
    xcm[m]/=tm;
    vcm[m]/=tm;
    acm[m]-=a0[m]/tm;
  }

#define PVEC(str,v) fprintf(log,\
			    "%s[X]: %10.5e  %s[Y]: %10.5e  %s[Z]: %10.5e\n", \
			   str,v[0],str,v[1],str,v[2])
#ifdef DEBUG
  PVEC("xcm",xcm);
  PVEC("acm",acm);
  PVEC("vcm",vcm);
#endif
  
  clear_mat(L);
  for(i=0; (i<natoms); i++) {
    m0=mass[i];
    for(m=0; (m<DIM); m++)
      dx[m]=x[i][m]-xcm[m];
    L[XX][XX]+=dx[XX]*dx[XX]*m0;
    L[XX][YY]+=dx[XX]*dx[YY]*m0;
    L[XX][ZZ]+=dx[XX]*dx[ZZ]*m0;
    L[YY][YY]+=dx[YY]*dx[YY]*m0;
    L[YY][ZZ]+=dx[YY]*dx[ZZ]*m0;
    L[ZZ][ZZ]+=dx[ZZ]*dx[ZZ]*m0;
  }
#ifdef DEBUG
  PVEC("L-x",L[XX]);
  PVEC("L-y",L[YY]);
  PVEC("L-z",L[ZZ]);
#endif

  return tm;
}

void stop_cm(FILE *log,int natoms,real mass[],rvec x[],rvec v[])
{
  rvec   xcm,vcm,acm;
  tensor L;
  int    i,m;

#ifdef DEBUG
  fprintf(log,"stopping center of mass motion...\n");
#endif
  (void)calc_cm(log,natoms,mass,x,v,xcm,vcm,acm,L);
  
  /* Subtract center of mass velocity */
  for(i=0; (i<natoms); i++)
    for(m=0; (m<DIM); m++)
      v[i][m]-=vcm[m];

#ifdef DEBUG
  (void)calc_cm(log,natoms,mass,x,v,xcm,vcm,acm,L);
#endif
}

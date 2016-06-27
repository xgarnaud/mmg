/* =============================================================================
**  This file is part of the mmg software package for the tetrahedral
**  mesh modification.
**  Copyright (c) Bx INP/Inria/UBordeaux/UPMC, 2004- .
**
**  mmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mmg distribution only if you accept them.
** =============================================================================
*/
/**
 * \file mmg2d/movpt_2d.c
 * \brief Node relocation routines
 * \author Charles Dapogny (UPMC)
 * \author Cécile Dobrzynski (Bx INP/Inria/UBordeaux)
 * \author Pascal Frey (UPMC)
 * \author Algiane Froehly (Inria/UBordeaux)
 * \version 5
 * \date 01 2014
 * \copyright GNU Lesser General Public License.
 **/
#include "mmg2d.h"

//extern char ddb;

/* Relocate boundary vertex whose ball is passed */
int _MMG2_movedgpt(MMG5_pMesh mesh,MMG5_pSol met,int ilist,int *list, char improve) {
  MMG5_pTria         pt,pt0;
  MMG5_pPoint        p0,p1,p2,ppt;
  double             step,ll1,ll2,o[2],no[2],calold,calnew;
  int                k,iel,ip0,ip1,ip2,it1,it2;
  char               i,i1,i2;
    
  pt0 = &mesh->tria[0];
  step = 0.1;
  ip1 = ip2 = it1 = it2 = 0;
  calold = calnew = DBL_MAX;
  
  /* First step: retrieve the two endpoints connected to p0 */
  for (k=0; k<ilist; k++) {
    iel = list[k] / 3;
    i   = list[k] % 3;
    i1 = _MMG5_inxt2[i];
    i2 = _MMG5_iprv2[i];
    
    pt = &mesh->tria[iel];
    calold = MG_MIN(caltri_iso(mesh,NULL,pt),calold);
 
    if ( MG_EDG(pt->tag[i1]) ) {
      if ( ip1 == 0 ) {
        ip1 = pt->v[i2];
        it1 = 3*iel+i1;
      }
      else if ( ip1 != pt->v[i2] ) {
        if ( ip2 == 0 ) {
          ip2 = pt->v[i2];
          it2 = 3*iel+i1;
        }
        else if ( ip2 != pt->v[i2] ) {
          printf("   *** Function movedgpt: three edges connected at point %d - abort.\n",pt->v[i]);
          exit(EXIT_FAILURE);
        }
      }
    }
    
    if ( MG_EDG(pt->tag[i2]) ) {
      if ( ip1 == 0 ) {
        ip1 = pt->v[i1];
        it1 = 3*iel+i2;
      }
      else if ( ip1 != pt->v[i1] ) {
        if ( ip2 == 0 ) {
          ip2 = pt->v[i1];
          it2 = 3*iel+i2;
        }
        else if ( ip2 != pt->v[i1] ) {
          printf("   *** Function movedgpt: three edges connected at point %d - abort.\n",pt->v[i]);
          exit(EXIT_FAILURE);
        }
      }
    }
  }
  
  /* Check that there are exactly two boundary points connected at p0 */
  if ( ip1 == 0 || ip2 == 0 ) {
    printf("   *** Function movedgpt: no two edges connected at edge, non singular point %d - abort.\n",pt->v[i]);
    exit(EXIT_FAILURE);
  }
  
  ip0 = pt->v[i];
  p0 = &mesh->point[ip0];
  p1 = &mesh->point[ip1];
  p2 = &mesh->point[ip2];
  
  /* Calculate length of both edges */
  ll1 = (p1->c[0]-p0->c[0])*(p1->c[0]-p0->c[0]) + (p1->c[1]-p0->c[1])*(p1->c[1]-p0->c[1]);
  ll2 = (p2->c[0]-p0->c[0])*(p2->c[0]-p0->c[0]) + (p2->c[1]-p0->c[1])*(p2->c[1]-p0->c[1]);
  
  /* Relocate p0 slightly towards p1 */
  if ( ll1 > ll2 ) {
    iel = it1 / 3;
    i   = it1 % 3;
  }
  /* Relocate p0 slightly towards p2 */
  else {
    iel = it2 / 3;
    i   = it2 % 3;
  }
  
  i1 = _MMG5_inxt2[i];
  i2 = _MMG5_iprv2[i];
  
  pt = &mesh->tria[iel];
  
  /* step = distance of the relocated position from ip0
     bezierCurv = distance s from inxt2[i] */
  if ( pt->v[i1] == ip0 ) _MMG2_bezierCurv(mesh,iel,i,step,o,no);
  else _MMG2_bezierCurv(mesh,iel,i,1.0-step,o,no);
  
  /* Evaluate resulting configuration */
  ppt = &mesh->point[0];
  ppt->c[0] = o[0];
  ppt->c[1] = o[1];
  ppt->n[0] = no[0];
  ppt->n[1] = no[1];
  
  for (k=0; k<ilist; k++) {
    iel = list[k] / 3;
    i   = list[k] % 3;
    pt  = &mesh->tria[iel];
    memcpy(pt0,pt,sizeof(MMG5_Tria));
    pt0->v[i] = 0;
    
    calnew = MG_MIN(caltri_iso(mesh,NULL,pt0),calnew);
  }
  
  if (calold < _MMG2_NULKAL && calnew <= calold) return(0);
  else if (calnew < _MMG2_NULKAL) return(0);
  else if ( improve && calnew < 1.02 * calold ) return(0);
  else if ( calnew < 0.3 * calold ) return(0);
  
  /* Update of the coordinates and normal of the point */
  p0 = &mesh->point[pt->v[i]];
  p0->c[0] = o[0];
  p0->c[1] = o[1];
  
  p0->n[0] = no[0];
  p0->n[1] = no[1];
  
  return(1);
}

/* Relocate internal vertex whose ball is passed */
int _MMG2_movintpt(MMG5_pMesh mesh,MMG5_pSol met,int ilist,int *list,char improve) {
  MMG5_pTria        pt,pt0;
  MMG5_pPoint       p0,p1,p2,ppt0;
  double            calold,calnew,vol,volbal,b[2];
  int               k,iel;
  char              i,i1,i2;
  
  ppt0 = &mesh->point[0];
  pt0  = &mesh->tria[0];
  
  volbal = 0.0;
  b[0] = b[1] = 0.0;
  calold = calnew = DBL_MAX;
  
  /* Calculate the now position for vertex, as well as the quality of the previous configuration */
  for (k=0; k<ilist; k++) {
    iel = list[k] / 3;
    i   = list[k] % 3;
    i1 = _MMG5_inxt2[i];
    i2 = _MMG5_iprv2[i];
    
    pt = &mesh->tria[iel];
        
    /* Volume of iel */
    p0 = &mesh->point[pt->v[i]];
    p1 = &mesh->point[pt->v[i1]];
    p2 = &mesh->point[pt->v[i2]];
    vol = 0.5* fabs((p1->c[0]-p0->c[0])*(p2->c[1]-p0->c[1]) - (p1->c[1]-p0->c[1])*(p2->c[0]-p0->c[0]));
    
    volbal += vol;
    
    /* Add coordinates of the centre of mass of iel, weighted by its volume */
    b[0] += _MMG5_ATHIRD*vol*(p0->c[0]+p1->c[0]+p2->c[0]);
    b[1] += _MMG5_ATHIRD*vol*(p0->c[1]+p1->c[1]+p2->c[1]);
    
    /* Quality of pt */
    calold = MG_MIN(caltri_iso(mesh,NULL,pt),calold);
  }
  
  if ( volbal < _MMG5_EPSD ) return(0);
  volbal = 1.0 / volbal;
  b[0] *= volbal;
  b[1] *= volbal;
  
  /* Check the quality of the resulting configuration */
  ppt0->c[0] = b[0];
  ppt0->c[1] = b[1];

  for (k=0; k<ilist; k++) {
    iel = list[k] / 3;
    i   = list[k] % 3;
    pt  = &mesh->tria[iel];
    memcpy(pt0,pt,sizeof(MMG5_Tria));
    pt0->v[i] = 0;
    
    calnew = MG_MIN(caltri_iso(mesh,NULL,pt0),calnew);
  }
    
  if (calold < _MMG2_NULKAL && calnew <= calold) return(0);
  else if (calnew < _MMG2_NULKAL) return(0);
  else if ( improve && calnew < 1.02 * calold ) return(0);
  else if ( calnew < 0.3 * calold ) return(0);
  
  /* Update of the coordinates of the point */
  p0 = &mesh->point[pt->v[i]];
  p0->c[0] = b[0];
  p0->c[1] = b[1];
  
  return(1);
}

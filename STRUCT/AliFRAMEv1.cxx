///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  Space frame class                                                          //
//                                                                           //
//Begin_Html
/*
<img src="gif/AliFRAMEClass.gif">
*/
//End_Html
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "AliFRAMEv1.h"
#include "AliRun.h"
#include "AliMC.h"
 
ClassImp(AliFRAMEv1)
 
//_____________________________________________________________________________
AliFRAMEv1::AliFRAMEv1() : AliFRAME()
{
  //
  // Default constructor for space frame
  //
}
 
//_____________________________________________________________________________
AliFRAMEv1::AliFRAMEv1(const char *name, const char *title)
       : AliFRAME(name,title)
{
  //
  // Standard constructor for space frame
  //
}
 
//_____________________________________________________________________________
void AliFRAMEv1::CreateGeometry()
{
  //
  // Create space frame geometry
  //
  //Begin_Html
  /*
    <img src="gif/AliFRAME.gif">
  */
  //End_Html
  //Begin_Html
  /*
    <img src="gif/AliFRAMETree.gif">
  */
  //End_Html
  AliMC* pMC = AliMC::GetMC();
  
  Int_t *idtmed = gAlice->Idtmed();

  Float_t dphi, dz_small, zpos, ddphi;
  Float_t tspar[5];
  Float_t tsparl[5];
  Float_t par[50], dz_long;
  
  //     R_IN : INNER RADIUS 
  //     R_OU : OUTER RADIUS 
  //     DR   : WALL THICKNESS 
  //   2*Z_IN : FREE SPACE IN Z FOR PHOS 
  //   2*Z_RI : FREE SPACE IN Z FOR RICH 
  //   2*Z_OU : LENGTH 
  //   2*DZ   : WIDTH OF FRAME ELEMENTS 
  Float_t r_in = 395;
  Float_t r_ou = 420;
  Float_t dr   = 0.3;
  Float_t z_in = 130;
  Float_t z_ou = 350;
  Float_t z_ri = 236.25;
  Float_t dz   = 8.75;
  
  
  //     Space Frame 
  
  par[0] = r_in;
  par[1] = r_ou;
  par[2] = z_ou;
  pMC->Gsvolu("BFMO", "TUBE", idtmed[1214], par, 3);
  
  //     Rings perpendicular to the beam 
  
  //     full rings at the ends 
  
  par[0] = r_in;
  par[1] = r_ou;
  par[2] = dz;
  pMC->Gsvolu("BRO1", "TUBE", idtmed[1218], par, 3);
  par[0] = r_in +dr;
  par[1] = r_ou -dr;
  par[2] = dz - dr;
  pMC->Gsvolu("BRI1", "TUBE", idtmed[1214], par, 3);
  pMC->Gspos("BRI1", 1, "BRO1", 0., 0., 0., 0, "ONLY");
  zpos = z_ou - dz;
  pMC->Gspos("BRO1", 1, "BFMO", 0., 0.,-zpos, 0, "ONLY");
  pMC->Gspos("BRO1", 2, "BFMO", 0., 0., zpos, 0, "ONLY");
  
  //     space for the HMPID 
  
  tspar[0] = r_in;
  tspar[1] = r_ou;
  tspar[2] = dz;
  tspar[3] = -240.;
  tspar[4] = 60.;
  pMC->Gsvolu("BRO2", "TUBS", idtmed[1218], tspar, 5);
  tspar[0] = r_in + dr;
  tspar[1] = r_ou - dr;
  tspar[2] = dz - dr;
  pMC->Gsvolu("BRI2", "TUBS", idtmed[1214], tspar, 5);
  pMC->Gspos("BRI2", 1, "BRO2", 0., 0., 0., 0, "ONLY");
  zpos = z_in + dz;
  pMC->Gspos("BRO2", 1, "BFMO", 0., 0.,-zpos, 0, "ONLY");
  pMC->Gspos("BRO2", 2, "BFMO", 0., 0., zpos, 0, "ONLY");
  
  
  
  tspar[0] = r_in;
  tspar[1] = r_ou;
  tspar[2] = dz;
  tspar[3] = 60.;
  tspar[4] = 120.;
  pMC->Gsvolu("BRO3", "TUBS", idtmed[1218], tspar, 5);
  tspar[0] = r_in + dr;
  tspar[1] = r_ou - dr;
  tspar[2] = dz - dr;
  pMC->Gsvolu("BRI3", "TUBS", idtmed[1214], tspar, 5);
  pMC->Gspos("BRI3", 1, "BRO3", 0., 0., 0., 0, "ONLY");
  zpos = 245.;
  pMC->Gspos("BRO3", 1, "BFMO", 0., 0.,-zpos, 0, "ONLY");
  pMC->Gspos("BRO3", 2, "BFMO", 0., 0., zpos, 0, "ONLY");
  
  //     longitudinal beams 
  
  //     outside phi=60-120 
  
  //     virtual elements 
  
  dz_small  = (z_ou - z_in -4*dz)/2;
  dz_long   = z_in;
  tsparl[0] = r_in;
  tsparl[1] = r_ou;
  tsparl[2] = dz_small;
  
  //     left and right 
  
  tsparl[3] = -240.;
  tsparl[4] = 60.;
  pMC->Gsvolu("BLO1", "TUBS", idtmed[1214], tsparl, 5);
  pMC->Gsdvt("BLD1", "BLO1", 20., 2, idtmed[1214], 15);
  
  //     central, leaving space for rich and phos 
  
  tsparl[2] = dz_long;
  tsparl[3] = -20.;
  tsparl[4] = 60.;
  pMC->Gsvolu("BLO2", "TUBS", idtmed[1214], tsparl, 5);
  pMC->Gsdvt("BLD2", "BLO2", 20., 2, idtmed[1214], 5);
  tsparl[3] = 120.;
  tsparl[4] = 200.;
  pMC->Gsvolu("BLO3", "TUBS", idtmed[1214], tsparl, 5);
  pMC->Gsdvt("BLD3", "BLO3", 20., 2, idtmed[1214], 5);
  
  //     real elements 
  
  dphi  = dz/(TMath::Pi()*(r_in + r_ou))*360;
  ddphi = dphi * dr/dz;
  tspar[0] = tsparl[0];
  tspar[1] = tsparl[1];
  tspar[2] = dz_small;
  tspar[3] = 10. - dphi;
  tspar[4] = 10.;
  pMC->Gsvolu("BL01", "TUBS", idtmed[1218], tspar, 5);
  
  tspar[0] = tsparl[0] + dr;
  tspar[1] = tsparl[1] - dr;
  tspar[3] = 10. - dphi + ddphi;
  tspar[4] = 10. - ddphi;
  pMC->Gsvolu("BL02", "TUBS", idtmed[1214], tspar, 5);
  pMC->Gspos("BL02", 1, "BL01", 0., 0., 0., 0, "ONLY");
  

  tspar[0] = tsparl[0];
  tspar[1] = tsparl[1];
  tspar[2] = dz_long;
  tspar[3] = 10. - dphi;
  tspar[4] = 10.;
  pMC->Gsvolu("BL11", "TUBS", idtmed[1218], tspar, 5);
  
  tspar[0] = tsparl[0] + dr;
  tspar[1] = tsparl[1] - dr;
  tspar[3] = 10. - dphi + ddphi;
  tspar[4] = 10. - ddphi;
  pMC->Gsvolu("BL12", "TUBS", idtmed[1214], tspar, 5);
  pMC->Gspos("BL12", 1, "BL11", 0., 0., 0., 0, "ONLY");
  
  pMC->Gspos("BL01", 1, "BLD1", 0., 0., 0., 0, "ONLY");
  pMC->Gspos("BL11", 1, "BLD2", 0., 0., 0., 0, "ONLY");
  pMC->Gspos("BL11", 2, "BLD3", 0., 0., 0., 0, "ONLY");
  
  zpos = z_in +2*dz + dz_small;
  pMC->Gspos("BLO1", 1, "BFMO", 0., 0.,-zpos, 0, "ONLY");
  pMC->Gspos("BLO1", 2, "BFMO", 0., 0., zpos, 0, "ONLY");
  pMC->Gspos("BLO2", 1, "BFMO", 0., 0.,   0., 0, "ONLY");
  pMC->Gspos("BLO3", 1, "BFMO", 0., 0.,   0., 0, "ONLY");
  
  //     PHI=60-120 (RICH) 
  
  tsparl[0] = r_in;
  tsparl[1] = r_ou;
  tsparl[2] = (z_ou - z_ri -4*dz)/2;
  tsparl[3] = 60.;
  tsparl[4] = 120.;
  pMC->Gsvolu("BLO4", "TUBS", idtmed[1214], tsparl, 5);
  pMC->Gsdvt("BLD4", "BLO4", 20., 2, idtmed[1214], 3);
  tspar[0] = tsparl[0];
  tspar[1] = tsparl[1];
  tspar[2] = tsparl[2];
  tspar[3] = 10. - dphi;
  tspar[4] = 10.;
  pMC->Gsvolu("BL03", "TUBS", idtmed[1218], tspar, 5);
  
  tspar[0] = tsparl[0] + dr;
  tspar[1] = tsparl[1] - dr;
  tspar[2] = tsparl[2];
  tspar[3] = 10. - dphi + ddphi;
  tspar[4] = 10. - ddphi;
  pMC->Gsvolu("BL04", "TUBS", idtmed[1214], tspar, 5);
  pMC->Gspos("BL04", 1, "BL03", 0., 0., 0., 0, "ONLY");
  
  pMC->Gspos("BL03", 1, "BLD4", 0., 0., 0., 0, "ONLY");
  
  pMC->Gspos("BLO4", 1, "BFMO", 0., 0., 293.125, 0, "ONLY");
  pMC->Gspos("BLO4", 2, "BFMO", 0., 0.,-293.125, 0, "ONLY");
  
  pMC->Gspos("BFMO", 1, "ALIC", 0., 0., 0., 0, "ONLY");
  pMC->Gsatt("BFMO", "SEEN", 0);
}

//_____________________________________________________________________________
void AliFRAMEv1::DrawDetector()
{
  //
  // Draw a shaded view of the space frame
  //

  AliMC* pMC = AliMC::GetMC();
  
  // Set everything unseen
  pMC->Gsatt("*", "seen", -1);
  // 
  // Set ALIC mother transparent
  pMC->Gsatt("ALIC","SEEN",0);
  //
  // Set the volumes visible
  pMC->Gsatt("BFMO","seen",0);
  pMC->Gsatt("BRO1","seen",1);
  pMC->Gsatt("BRI1","seen",0);
  pMC->Gsatt("BRO2","seen",1);
  pMC->Gsatt("BRI2","seen",0);
  pMC->Gsatt("BRO3","seen",1);
  pMC->Gsatt("BRI3","seen",0);
  pMC->Gsatt("BLO1","seen",0);
  pMC->Gsatt("BLD1","seen",0);
  pMC->Gsatt("BLO2","seen",0);
  pMC->Gsatt("BLD2","seen",0);
  pMC->Gsatt("BLO3","seen",0);
  pMC->Gsatt("BLD3","seen",0);
  pMC->Gsatt("BL01","seen",1);
  pMC->Gsatt("BL02","seen",1);
  pMC->Gsatt("BL11","seen",1);
  pMC->Gsatt("BL12","seen",1);
  pMC->Gsatt("BLO4","seen",0);
  pMC->Gsatt("BLD4","seen",0);
  pMC->Gsatt("BL03","seen",1);
  pMC->Gsatt("BL04","seen",1);
  //
  pMC->Gdopt("hide", "on");
  pMC->Gdopt("shad", "on");
  pMC->Gsatt("*", "fill", 7);
  pMC->SetClipBox(".");
  pMC->DefaultRange();
  pMC->Gdraw("alic", 40, 30, 0, 10, 10, .015, .015);
  pMC->Gdhead(1111, "Space Frame");
  pMC->Gdman(18, 4, "MAN");
}

//_____________________________________________________________________________
void AliFRAMEv1::CreateMaterials()
{

  //
  // Create materials for the space frame
  //
  
  Int_t   ISXFLD = gAlice->Field()->Integ();
  Float_t SXMGMX = gAlice->Field()->Max();
  
  Float_t asteel[4] = { 55.847,51.9961,58.6934,28.0855 };
  Float_t zsteel[4] = { 26.,24.,28.,14. };
  Float_t wsteel[4] = { .715,.18,.1,.005 };
  
  Float_t epsil, stmin, deemax, tmaxfd, stemax;
  
  //     STEEL 


  // --- Define the various materials for GEANT --- 
  AliMaterial(15, "AIR$      ", 14.61, 7.3, .001205, 30423.24, 67500);
  AliMixture(19, "STAINLESS STEEL$", asteel, zsteel, 7.88, 4, wsteel);
  
  // **************** 
  //     Defines tracking media parameters. 
  //     Les valeurs sont commentees pour laisser le defaut 
  //     a GEANT (version 3-21, page CONS200), f.m. 
  epsil  = .001; // Tracking precision, 
  stemax = -1.;  // Maximum displacement for multiple scat 
  tmaxfd = -20.; // Maximum angle due to field deflection 
  deemax = -.3;  // Maximum fractional energy loss, DLS 
  stmin  = -.8;
  // *************** 
  //ifield0 = 0;  // Field off 
  //ifield1 = 2;  // 1.0 T. FIELD (DIPOLE) 
  //ifield3 = 3;
  
  //    Air 
  
  // 0.2 T. FIELD (L3) 
  AliMedium(1215, "AIR_L3_US        ", 15, 0, ISXFLD, SXMGMX, tmaxfd, stemax, deemax, epsil, stmin);
  
  //    Steel 
  
  AliMedium(1219, "ST_L3_US          ", 19, 0, ISXFLD, SXMGMX, tmaxfd, stemax, deemax, epsil, stmin);
}












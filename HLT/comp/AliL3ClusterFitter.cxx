// @(#) $Id$

// Author: Anders Vestbo <mailto:vestbo@fi.uib.no>
//*-- Copyright &copy ALICE HLT Group

#include "AliL3StandardIncludes.h"

#include "AliL3Logging.h"
#include "AliL3ClusterFitter.h"
#include "AliL3FitUtilities.h"
#include "AliL3DigitData.h"
#include "AliL3ModelTrack.h"
#include "AliL3TrackArray.h"
#include "AliL3MemHandler.h"
#include "AliL3HoughTrack.h"
#include "AliL3SpacePointData.h"
#include "AliL3Compress.h"

#if GCCVERSION == 3
using namespace std;
#endif

//_____________________________________________________________
//
//  AliL3ClusterFitter
//


ClassImp(AliL3ClusterFitter)

Int_t AliL3ClusterFitter::fBadFitError=0;
Int_t AliL3ClusterFitter::fFitError=0;
Int_t AliL3ClusterFitter::fResultError=0;
Int_t AliL3ClusterFitter::fFitRangeError=0;

AliL3ClusterFitter::AliL3ClusterFitter()
{
  plane=0;
  fNmaxOverlaps = 3;
  fChiSqMax=12;
  fRowMin=-1;
  fRowMax=-1;
  fFitted=0;
  fFailed=0;
  fYInnerWidthFactor=1;
  fZInnerWidthFactor=1;
  fYOuterWidthFactor=1;
  fZOuterWidthFactor=1;
  fSeeds=0;
  fProcessTracks=0;
  fClusters=0;
  fNMaxClusters=0;
  fNClusters=0;
}

AliL3ClusterFitter::AliL3ClusterFitter(Char_t *path)
{
  strcpy(fPath,path);
  plane=0;
  fNmaxOverlaps = 3;
  fChiSqMax=12;
  fRowMin=-1;
  fRowMax=-1;
  fFitted=0;
  fFailed=0;
  fYInnerWidthFactor=1;
  fZInnerWidthFactor=1;
  fYOuterWidthFactor=1;
  fZOuterWidthFactor=1;
  fSeeds=0;
  fProcessTracks=0;
  fNMaxClusters=100000;
  fClusters=0;
  fNClusters=0;
}

AliL3ClusterFitter::~AliL3ClusterFitter()
{
  if(fSeeds)
    delete fSeeds;
  if(fClusters)
    delete [] fClusters;
}

void AliL3ClusterFitter::Init(Int_t slice,Int_t patch,Int_t *rowrange,AliL3TrackArray *tracks)
{
  //Assuming tracklets found by the line transform

  fSlice=slice;
  fPatch=patch;
  
  if(rowrange[0] > AliL3Transform::GetLastRow(patch) || rowrange[1] < AliL3Transform::GetFirstRow(patch))
    cerr<<"AliL3ClusterFitter::Init : Wrong rows "<<rowrange[0]<<" "<<rowrange[1]<<endl;
  fRowMin=rowrange[0];
  fRowMax=rowrange[1];

  if(fRowMin < 0)
    fRowMin = 0;
  if(fRowMax > AliL3Transform::GetLastRow(fPatch))
    fRowMax = AliL3Transform::GetLastRow(fPatch);
  
  fFitted=fFailed=0;
  
  Int_t ntimes = AliL3Transform::GetNTimeBins()+1;
  Int_t npads = AliL3Transform::GetNPads(AliL3Transform::GetLastRow(fPatch))+1;//Max num of pads.
  Int_t bounds = ntimes*npads;
  if(fRow)
    delete [] fRow;
  fRow = new Digit[bounds];
  if(fTracks)
    delete fTracks;
  
  fTracks = new AliL3TrackArray("AliL3ModelTrack");
  
  for(Int_t i=0; i<tracks->GetNTracks(); i++)
    {
      AliL3HoughTrack *track = (AliL3HoughTrack*)tracks->GetCheckedTrack(i);
      if(!track) continue;
      AliL3ModelTrack *mtrack = (AliL3ModelTrack*)fTracks->NextTrack();
      mtrack->Init(slice,patch);
      mtrack->SetTgl(track->GetTgl());
      mtrack->SetRowRange(rowrange[0],rowrange[1]);
      for(Int_t j=fRowMin; j<=fRowMax; j++)
	{
	  Float_t hit[3];
	  track->GetLineCrossingPoint(j,hit);
	  hit[0] += AliL3Transform::Row2X(track->GetFirstRow());
	  Float_t R = sqrt(hit[0]*hit[0] + hit[1]*hit[1]);
	  hit[2] = R*track->GetTgl();
	  Int_t se,ro;
	  AliL3Transform::Slice2Sector(slice,j,se,ro);
	  AliL3Transform::Local2Raw(hit,se,ro);
	  if(hit[1]<0 || hit[1]>=AliL3Transform::GetNPads(j) || hit[2]<0 || hit[2]>=AliL3Transform::GetNTimeBins())
	    {
	      mtrack->SetPadHit(j,-1);
	      mtrack->SetTimeHit(j,-1);
	      continue;
	    }
	  mtrack->SetPadHit(j,hit[1]);
	  mtrack->SetTimeHit(j,hit[2]);
	  mtrack->SetCrossingAngleLUT(j,fabs(track->GetPsiLine() - AliL3Transform::Pi()/2));
	  //if(mtrack->GetCrossingAngleLUT(j) > AliL3Transform::Deg2Rad(20))
	  //  cout<<"Angle "<<mtrack->GetCrossingAngleLUT(j)<<" psiline "<<track->GetPsiLine()*180/3.1415<<endl;
	  mtrack->CalculateClusterWidths(j);
	}
    }
  //  cout<<"Copied "<<fTracks->GetNTracks()<<" tracks "<<endl;
}

void AliL3ClusterFitter::Init(Int_t slice,Int_t patch)
{
  fSlice=slice;
  fPatch=patch;

  fRowMin=AliL3Transform::GetFirstRow(patch);
  fRowMax=AliL3Transform::GetLastRow(patch);
  
  fFitted=fFailed=0;
  
  Int_t ntimes = AliL3Transform::GetNTimeBins()+1;
  Int_t npads = AliL3Transform::GetNPads(AliL3Transform::GetLastRow(fPatch))+1;//Max num of pads.
  Int_t bounds = ntimes*npads;
  if(fRow)
    delete [] fRow;
  fRow = new Digit[bounds];
  if(fTracks)
    delete fTracks;
  fTracks = new AliL3TrackArray("AliL3ModelTrack");  

}


void AliL3ClusterFitter::LoadSeeds(Int_t *rowrange,Bool_t offline)
{
  
  cout<<"Loading the seeds"<<endl;
  Char_t fname[1024];
  
  if(offline)
    sprintf(fname,"%s/offline/tracks_%d.raw",fPath,0);
  else
    sprintf(fname,"%s/hough/tracks_%d.raw",fPath,0);
  
  cout<<"AliL3ClusterFitter::LoadSeeds : Loading input tracks from "<<fname<<endl;
  
  AliL3MemHandler tfile;
  tfile.SetBinaryInput(fname);
  
  if(fSeeds)
    delete fSeeds;
  fSeeds = new AliL3TrackArray("AliL3ModelTrack");
  tfile.Binary2TrackArray(fSeeds);
  tfile.CloseBinaryInput();

  //if(!offline)
  //fSeeds->QSort();
  
  Int_t clustercount=0;
  for(Int_t i=0; i<fSeeds->GetNTracks(); i++)
    {
      AliL3ModelTrack *track = (AliL3ModelTrack*)fSeeds->GetCheckedTrack(i);
      if(!track) continue;

      if(!offline)
	{
	  /*
	  if(track->GetNHits() < 10 || track->GetPt() < 0.08) 
	    {
	      fSeeds->Remove(i);
	      continue;
	    }
	  */
	}
      clustercount += track->GetNHits();
      track->CalculateHelix();
      
      Int_t nhits = track->GetNHits();
      UInt_t *hitids = track->GetHitNumbers();

      Int_t origslice = (hitids[nhits-1]>>25)&0x7f;//Slice of innermost point

      track->Init(origslice,-1);
      Int_t slice = origslice;
      
      //for(Int_t j=rowrange[1]; j>=rowrange[0]; j--)
      for(Int_t j=rowrange[0]; j<=rowrange[1]; j++)
	{
	  
	  //Calculate the crossing point between track and padrow
	  Float_t angle = 0; //Perpendicular to padrow in local coordinates
	  AliL3Transform::Local2GlobalAngle(&angle,slice);
	  if(!track->CalculateReferencePoint(angle,AliL3Transform::Row2X(j)))
	    {
	      cerr<<"No crossing in slice "<<slice<<" padrow "<<j<<endl;
	      continue;
	      //track->Print();
	      //exit(5);
	    }
	  Float_t xyz_cross[3] = {track->GetPointX(),track->GetPointY(),track->GetPointZ()};
	  
	  Int_t sector,row;
	  AliL3Transform::Slice2Sector(slice,j,sector,row);
	  AliL3Transform::Global2Raw(xyz_cross,sector,row);
	  //cout<<"Examining slice "<<slice<<" row "<<j<<" pad "<<xyz_cross[1]<<" time "<<xyz_cross[2]<<endl;
	  if(xyz_cross[1] < 0 || xyz_cross[1] >= AliL3Transform::GetNPads(j)) //Track leaves the slice
	    {
	    newslice:
	      
	      Int_t tslice=slice;
	      Float_t lastcross=xyz_cross[1];
	      if(xyz_cross[1] > 0)
		{
		  if(slice == 17)
		    slice=0;
		  else if(slice == 35)
		    slice = 18;
		  else
		    slice += 1;
		}
	      else
		{
		  if(slice == 0)
		    slice = 17;
		  else if(slice==18)
		    slice = 35;
		  else
		    slice -= 1;
		}
	      if(slice < 0 || slice>35)
		{
		  cerr<<"Wrong slice "<<slice<<" on row "<<j<<endl;
		  exit(5);
		}
	      //cout<<"Track leaving, trying slice "<<slice<<endl;
	      angle=0;
	      AliL3Transform::Local2GlobalAngle(&angle,slice);
	      if(!track->CalculateReferencePoint(angle,AliL3Transform::Row2X(j)))
		{
		  cerr<<"No crossing in slice "<<slice<<" padrow "<<j<<endl;
		  continue;
		  //track->Print();
		  //exit(5);
		}
	      xyz_cross[0] = track->GetPointX();
	      xyz_cross[1] = track->GetPointY();
	      xyz_cross[2] = track->GetPointZ();
	      Int_t sector,row;
	      AliL3Transform::Slice2Sector(slice,j,sector,row);
	      AliL3Transform::Global2Raw(xyz_cross,sector,row);
	      if(xyz_cross[1] < 0 || xyz_cross[1] >= AliL3Transform::GetNPads(j)) //track is in the borderline
		{
		  if(xyz_cross[1] > 0 && lastcross > 0 || xyz_cross[1] < 0 && lastcross < 0)
		    goto newslice;
		  else
		    {
		      slice = tslice;//Track is on the border of two slices
		      continue;
		    }
		}
	    }
	  
	  if(xyz_cross[2] < 0 || xyz_cross[2] >= AliL3Transform::GetNTimeBins())//track goes out of range
	    continue;
	  
	  if(xyz_cross[1] < 0 || xyz_cross[1] >= AliL3Transform::GetNPads(j))
	    {
	      cerr<<"Slice "<<slice<<" padrow "<<j<<" pad "<<xyz_cross[1]<<" time "<<xyz_cross[2]<<endl;
	      track->Print();
	      exit(5);
	    }
	  
	  track->SetPadHit(j,xyz_cross[1]);
	  track->SetTimeHit(j,xyz_cross[2]);
	  angle=0;
	  AliL3Transform::Local2GlobalAngle(&angle,slice);
	  Float_t crossingangle = track->GetCrossingAngle(j,slice);
	  track->SetCrossingAngleLUT(j,crossingangle);
	  
	  track->CalculateClusterWidths(j);
	  
	  track->GetClusterModel(j)->fSlice = slice;
	  
	}
      memset(track->GetHitNumbers(),0,159*sizeof(UInt_t));
      track->SetNHits(0);
    }
  fSeeds->Compress();
  
  AliL3Compress *c = new AliL3Compress(-1,-1,fPath);
  c->WriteFile(fSeeds,"tracks_before.raw");
  delete c;
  
  cout<<"Loaded "<<fSeeds->GetNTracks()<<" seeds and "<<clustercount<<" clusters"<<endl;
}

void AliL3ClusterFitter::FindClusters()
{
  if(!fTracks)
    {
      cerr<<"AliL3ClusterFitter::Process : No tracks"<<endl;
      return;
    }
  if(!fRowData)
    {
      cerr<<"AliL3ClusterFitter::Process : No data "<<endl;
      return;
    }
  
  AliL3DigitRowData *rowPt = fRowData;
  AliL3DigitData *digPt=0;

  Int_t pad,time;
  Short_t charge;
  
  if(fRowMin < 0)
    {
      fRowMin = AliL3Transform::GetFirstRow(fPatch);
      fRowMax = AliL3Transform::GetLastRow(fPatch);
    }
  for(Int_t i=AliL3Transform::GetFirstRow(fPatch); i<=AliL3Transform::GetLastRow(fPatch); i++)
    {
      if((Int_t)rowPt->fRow < fRowMin)
	{
	  AliL3MemHandler::UpdateRowPointer(rowPt);
	  continue;
	}
      else if((Int_t)rowPt->fRow > fRowMax)
	break;
      else if((Int_t)rowPt->fRow != i)
	{
	  cerr<<"AliL3ClusterFitter::FindClusters : Mismatching row numbering "<<i<<" "<<rowPt->fRow<<endl;
	  exit(5);
	}
      fCurrentPadRow = i;
      memset((void*)fRow,0,(AliL3Transform::GetNTimeBins()+1)*(AliL3Transform::GetNPads(i)+1)*sizeof(Digit));
      digPt = (AliL3DigitData*)rowPt->fDigitData;

      for(UInt_t j=0; j<rowPt->fNDigit; j++)
	{
	  pad = digPt[j].fPad;
	  time = digPt[j].fTime;
	  charge = digPt[j].fCharge;
	  if(charge > 1024)
	    charge -= 1024;
	  fRow[(AliL3Transform::GetNTimeBins()+1)*pad+time].fCharge = charge;
	  fRow[(AliL3Transform::GetNTimeBins()+1)*pad+time].fUsed = kFALSE;
	  //cout<<"Row "<<i<<" pad "<<pad<<" time "<<time<<" charge "<<charge<<endl;
	}
      
      for(Int_t it=0; it<2; it++)
	{
	  if(it==0)
	    {
	      fProcessTracks = fSeeds;
	      fSeeding = kTRUE;
	    }
	  else
	    {
	      fProcessTracks = fTracks;
	      fSeeding = kFALSE;
	    }
	  if(!fProcessTracks)
	    continue;
	  
	  for(Int_t k=0; k<fProcessTracks->GetNTracks(); k++)
	    {
	      AliL3ModelTrack *track = (AliL3ModelTrack*)fProcessTracks->GetCheckedTrack(k);
	      if(!track) continue;
	      
	      if(fSeeding)
		if(track->GetClusterModel(i)->fSlice != fSlice) continue;
	      
	      if(track->GetPadHit(i) < 0 || track->GetPadHit(i) > AliL3Transform::GetNPads(i)-1 ||
		 track->GetTimeHit(i) < 0 || track->GetTimeHit(i) > AliL3Transform::GetNTimeBins()-1)
		{
		  track->SetCluster(i,0,0,0,0,0,0);
		  continue;
		}
	      
	      if(CheckCluster(k) == kFALSE)
		fFailed++;
	    }
	}
      //FillZeros(rowPt);
      AliL3MemHandler::UpdateRowPointer(rowPt);
    }
  
  fSeeding = kTRUE;
  AddClusters();
  fSeeding = kFALSE;
  AddClusters();
    
  cout<<"Fitted "<<fFitted<<" clusters, failed "<<fFailed<<endl;
  cout<<"Distribution:"<<endl;
  cout<<"Bad fit "<<fBadFitError<<endl;
  cout<<"Fit error "<<fFitError<<endl;
  cout<<"Result error "<<fResultError<<endl;
  cout<<"Fit range error "<<fFitRangeError<<endl;

}

Bool_t AliL3ClusterFitter::CheckCluster(Int_t trackindex)
{
  //Check if this is a single or overlapping cluster
  
  AliL3ModelTrack *track = (AliL3ModelTrack*)fProcessTracks->GetCheckedTrack(trackindex);
  
  Int_t row = fCurrentPadRow;
  
  if(track->IsSet(row)) //A fit has already be performed on this one
    return kTRUE;
  
  //Define the cluster region of this hit:
  Int_t padr[2]={999,-1};
  Int_t timer[2]={999,-1};
  
  if(!SetFitRange(track,padr,timer))
    {
      track->SetCluster(fCurrentPadRow,0,0,0,0,0,0);
      
      if(fDebug)
	cout<<"Failed to fit cluster at row "<<row<<" pad "<<(Int_t)rint(track->GetPadHit(row))<<" time "
	    <<(Int_t)rint(track->GetTimeHit(row))<<" hitcharge "
	    <<fRow[(AliL3Transform::GetNTimeBins()+1)*(Int_t)rint(track->GetPadHit(row))+(Int_t)rint(track->GetTimeHit(row))].fCharge<<endl;
      fFitRangeError++;
      return kFALSE;
    }

  //Check if any other track contributes to this cluster:
  //This is done by checking if the tracks are overlapping within
  //the range defined by the track parameters
  for(Int_t t=trackindex+1; t<fProcessTracks->GetNTracks(); t++)
    {
      AliL3ModelTrack *tr = (AliL3ModelTrack*)fProcessTracks->GetCheckedTrack(t);
      if(!tr) continue;
      if(fSeeding)
	if(tr->GetClusterModel(row)->fSlice != fSlice) continue;
      Int_t xyw = (Int_t)ceil(sqrt(tr->GetParSigmaY2(row))) + 1;
      Int_t zw = (Int_t)ceil(sqrt(tr->GetParSigmaZ2(row))) + 1;
      if( 
	 (tr->GetPadHit(row) - xyw > padr[0] && tr->GetPadHit(row) - xyw < padr[1] &&
	  tr->GetTimeHit(row) - zw > timer[0] && tr->GetTimeHit(row) - zw < timer[1]) ||
	 
	 (tr->GetPadHit(row) + xyw > padr[0] && tr->GetPadHit(row) + xyw < padr[1] &&
	  tr->GetTimeHit(row) - zw > timer[0] && tr->GetTimeHit(row) - zw < timer[1]) ||
	 
	 (tr->GetPadHit(row) - xyw > padr[0] && tr->GetPadHit(row) - xyw < padr[1] &&
	  tr->GetTimeHit(row) + zw > timer[0] && tr->GetTimeHit(row) + zw < timer[1]) ||
	 
	 (tr->GetPadHit(row) + xyw > padr[0] && tr->GetPadHit(row) + xyw < padr[1] &&
	  tr->GetTimeHit(row) + zw > timer[0] && tr->GetTimeHit(row) + zw < timer[1]) 
	 )
	{
	  if(SetFitRange(tr,padr,timer)) //Expand the cluster fit range
	    track->SetOverlap(row,t);    //Set overlap
	}
    }

  if(fDebug)
    cout<<"Fitting cluster with "<<track->GetNOverlaps(fCurrentPadRow)<<" overlaps"<<endl;
  FitClusters(track,padr,timer);
  return kTRUE;
}

Bool_t AliL3ClusterFitter::SetFitRange(AliL3ModelTrack *track,Int_t *padrange,Int_t *timerange)
{
  Int_t row = fCurrentPadRow;
  Int_t nt = AliL3Transform::GetNTimeBins()+1;
  
  Int_t nsearchbins=0;
  if(row < 63)
    nsearchbins=25;
  else
    nsearchbins=49;
  Int_t padloop[49] = {0,0,0,-1,1,-1,1,-1,1,0,0,-1,1,-1,1 ,2,-2,2,-2,2,-2,2,-2,2,-2
		       ,0,1,2,3,3,3,3,3,3,3
		       ,2,1,0,-1,-2,-3
		       ,-3,-3,-3,-3,-3,-3,-1,-1};
  Int_t timeloop[49] = {0,1,-1,0,0,1,1,-1,-1,2,-2,2,2,-2,-2 ,0,0,1,1,-1,-1,2,2,-2,-2
			,-3,-3,-3,-3,-2,-1,0,1,2,3
			,3,3,3,3,3,3,2,1,0,-1,-2,-3,-3,-3};
  
  Int_t padhit = (Int_t)rint(track->GetPadHit(row));
  Int_t timehit = (Int_t)rint(track->GetTimeHit(row));
  Int_t padmax=-1;
  Int_t timemax=-1;

  for(Int_t index=0; index<nsearchbins; index++)
    {
      if(IsMaximum(padhit + padloop[index],timehit + timeloop[index])) 
	{
	  padmax = padhit + padloop[index];
	  timemax = timehit + timeloop[index];
	  break;
	}
    }

  /*
  if(IsMaximum(padhit,timehit))
    {
      padmax = padhit;
      timemax = timehit;
    }
  */
  //Define the cluster region of this hit:
  //The region we look for, is centered at the local maxima
  //and expanded around using the parametrized cluster width
  //according to track parameters.
  Int_t xyw = (Int_t)ceil(sqrt(track->GetParSigmaY2(row)))+1; 
  Int_t zw = (Int_t)ceil(sqrt(track->GetParSigmaZ2(row)))+1;  
  if(padmax>=0 && timemax>=0)
    {
      if(fDebug)
	{
	  cout<<"Expanding cluster range using expected cluster widths: "<<xyw<<" "<<zw
	      <<" and setting local maxima pad "<<padmax<<" time "<<timemax<<endl;
	  if(xyw > 10 || zw > 10)
	    track->Print();
	}
      
      //Set the hit to the local maxima of the cluster.
      //Store the maxima in the cluster model structure,
      //-only temporary, it will be overwritten when calling SetCluster.
      
      track->GetClusterModel(row)->fDPad = padmax;
      track->GetClusterModel(row)->fDTime = timemax;

      for(Int_t i=padmax-xyw; i<=padmax+xyw; i++)
	{
	  for(Int_t j=timemax-zw; j<=timemax+zw; j++)
	    {
	      if(i<0 || i>=AliL3Transform::GetNPads(row) || j<0 || j>=AliL3Transform::GetNTimeBins()) continue;
	      if(fRow[nt*i+j].fCharge)
		{
		  if(i < padrange[0]) padrange[0]=i;
		  if(i > padrange[1]) padrange[1]=i;
		  if(j < timerange[0]) timerange[0]=j;
		  if(j > timerange[1]) timerange[1]=j;
		}
	    }
	}
      if(fDebug)
	cout<<"New padrange "<<padrange[0]<<" "<<padrange[1]<<" "<<" time "<<timerange[0]<<" "<<timerange[1]<<endl;
      return kTRUE;
    }
  return kFALSE;
}

Bool_t AliL3ClusterFitter::IsMaximum(Int_t pad,Int_t time)
{
  if(pad<0 || pad >= AliL3Transform::GetNPads(fCurrentPadRow) ||
     time<0 || time >= AliL3Transform::GetNTimeBins())
    return kFALSE;
  Int_t nt = AliL3Transform::GetNTimeBins()+1;
  if(fRow[nt*pad+time].fUsed == kTRUE) return kFALSE; //Peak has been assigned before
  Int_t charge = fRow[nt*pad+time].fCharge;
  if(charge == 1023 || charge==0) return kFALSE;
  
  fRow[nt*pad+time].fUsed = kTRUE;
  return kTRUE;

  //if(charge < fRow[nt*(pad-1)+(time-1)].fCharge) return kFALSE;
  if(charge < fRow[nt*(pad)+(time-1)].fCharge) return kFALSE;
  //if(charge < fRow[nt*(pad+1)+(time-1)].fCharge) return kFALSE;
  if(charge < fRow[nt*(pad-1)+(time)].fCharge) return kFALSE;
  if(charge < fRow[nt*(pad+1)+(time)].fCharge) return kFALSE;
  //if(charge < fRow[nt*(pad-1)+(time+1)].fCharge) return kFALSE;
  if(charge < fRow[nt*(pad)+(time+1)].fCharge) return kFALSE;
  //if(charge < fRow[nt*(pad+1)+(time+1)].fCharge) return kFALSE;
  fRow[nt*pad+time].fUsed = kTRUE;
  return kTRUE;
}

void AliL3ClusterFitter::FitClusters(AliL3ModelTrack *track,Int_t *padrange,Int_t *timerange)
{
  //Handle single and overlapping clusters
    
  //Check whether this cluster has been set before:
  
  Int_t size = FIT_PTS;
  Int_t max_tracks = FIT_MAXPAR/NUM_PARS;
  if(track->GetNOverlaps(fCurrentPadRow) > max_tracks)
    {
      cerr<<"AliL3ClusterFitter::FitOverlappingClusters : Too many overlapping tracks"<<endl;
      return;
    }
  Int_t *overlaps = track->GetOverlaps(fCurrentPadRow);
  
  //Check if at least one cluster is not already fitted
  Bool_t all_fitted=kTRUE;
  
  Int_t k=-1;
  while(k < track->GetNOverlaps(fCurrentPadRow))
    {
      AliL3ModelTrack *tr=0;
      if(k==-1)
 	tr = track;
      else
	tr = (AliL3ModelTrack*)fProcessTracks->GetCheckedTrack(overlaps[k]);
      k++;
      if(!tr) continue;
      if(!tr->IsSet(fCurrentPadRow) && !tr->IsPresent(fCurrentPadRow))//cluster has not been set and is not present
	{
	  all_fitted = kFALSE;
	  break;
	}
    }
  if(all_fitted)
    {
      if(fDebug)
	cout<<"But all the clusters were already fitted on row "<<fCurrentPadRow<<endl;
      return;
    }
  
  //Allocate fit parameters array; this is interface to the C code
  plane = new DPOINT[FIT_PTS];
  memset(plane,0,FIT_PTS*sizeof(DPOINT));

  Double_t x[FIT_PTS],y[FIT_PTS],s[FIT_PTS];
  
  //Fill the fit parameters:
  Double_t a[FIT_MAXPAR];
  Int_t lista[FIT_MAXPAR];
  Double_t dev[FIT_MAXPAR],chisq_f;
  
  Int_t fit_pars=0;
  
  Int_t n_overlaps=0;
  k=-1;
  
  //Fill the overlapping tracks:
  while(k < track->GetNOverlaps(fCurrentPadRow))
    {
      AliL3ModelTrack *tr=0;
      if(k==-1)
	tr = track;
      else
	tr = (AliL3ModelTrack*)fProcessTracks->GetCheckedTrack(overlaps[k]);
      k++;
      if(!tr) continue;
      
      if(tr->IsSet(fCurrentPadRow) && !tr->IsPresent(fCurrentPadRow)) continue;//Cluster fit failed before
      
      //Use the local maxima as the input to the fitting routine.
      //The local maxima is temporary stored in the cluster model:
      Int_t hitpad = (Int_t)rint(tr->GetClusterModel(fCurrentPadRow)->fDPad);  //rint(tr->GetPadHit(fCurrentPadRow));
      Int_t hittime = (Int_t)rint(tr->GetClusterModel(fCurrentPadRow)->fDTime); //rint(tr->GetTimeHit(fCurrentPadRow));
      Int_t charge = fRow[(AliL3Transform::GetNTimeBins()+1)*hitpad + hittime].fCharge;
      
      if(fDebug)
	cout<<"Fitting track cluster, pad "<<tr->GetPadHit(fCurrentPadRow)<<" time "
	    <<tr->GetTimeHit(fCurrentPadRow)<<" charge "<<charge<<" at local maxima in pad "<<hitpad
	    <<" time "<<hittime<<" xywidth "<<sqrt(tr->GetParSigmaY2(fCurrentPadRow))
	    <<" zwidth "<<sqrt(tr->GetParSigmaZ2(fCurrentPadRow))<<endl;
      
      if(charge==0)
	{
	  cerr<<"Charge still zero!"<<endl;
	  exit(5);
	}
            
      a[n_overlaps*NUM_PARS+2] = hitpad;
      a[n_overlaps*NUM_PARS+4] = hittime;
      
      if(!tr->IsSet(fCurrentPadRow)) //Cluster is not fitted before
	{
	  a[n_overlaps*NUM_PARS+1] = charge;
	  a[n_overlaps*NUM_PARS+3] = sqrt(tr->GetParSigmaY2(fCurrentPadRow)) * GetYWidthFactor();
	  a[n_overlaps*NUM_PARS+5] = sqrt(tr->GetParSigmaZ2(fCurrentPadRow)) * GetZWidthFactor();
	  a[n_overlaps*NUM_PARS+6] = sqrt(tr->GetParSigmaZ2(fCurrentPadRow)) * GetZWidthFactor();
	  lista[n_overlaps*NUM_PARS + 1] = 1;
	  lista[n_overlaps*NUM_PARS + 2] = 1;
	  lista[n_overlaps*NUM_PARS + 3] = 0;
	  lista[n_overlaps*NUM_PARS + 4] = 1;
	  lista[n_overlaps*NUM_PARS + 5] = 0;
	  lista[n_overlaps*NUM_PARS + 6] = 0;
	  fit_pars             += 3;
	}
      else  //Cluster was fitted before
	{
	  if(!tr->IsPresent(fCurrentPadRow))
	    {
	      cerr<<"AliL3ClusterFitter::FindClusters : Cluster not present; there is a bug here"<<endl;
	      exit(5);
	    }
	  Int_t charge;
	  Float_t xywidth,zwidth,pad,time;
	  tr->GetPad(fCurrentPadRow,pad);
	  tr->GetTime(fCurrentPadRow,time);
	  tr->GetClusterCharge(fCurrentPadRow,charge);
	  xywidth = sqrt(tr->GetParSigmaY2(fCurrentPadRow));
	  zwidth = sqrt(tr->GetParSigmaZ2(fCurrentPadRow));
	  if(fDebug)
	    cout<<"Cluster had been fitted before, pad "<<pad<<" time "<<time<<" charge "<<charge<<" width "<<xywidth<<" "<<zwidth<<endl;
	  
	  a[n_overlaps*NUM_PARS+2] = pad;
	  a[n_overlaps*NUM_PARS+4] = time;
	  a[n_overlaps*NUM_PARS+1] = charge;
	  a[n_overlaps*NUM_PARS+3] = sqrt(xywidth) * GetYWidthFactor();
	  a[n_overlaps*NUM_PARS+5] = sqrt(zwidth) * GetZWidthFactor();
	  a[n_overlaps*NUM_PARS+6] = sqrt(zwidth) * GetZWidthFactor();

	  lista[n_overlaps*NUM_PARS + 1] = 1;
	  lista[n_overlaps*NUM_PARS + 2] = 0;
	  lista[n_overlaps*NUM_PARS + 3] = 0;
	  lista[n_overlaps*NUM_PARS + 4] = 0;
	  lista[n_overlaps*NUM_PARS + 5] = 0;
	  lista[n_overlaps*NUM_PARS + 6] = 0;
	  fit_pars             += 1;
	}
      n_overlaps++;
    }
  
  if(n_overlaps==0) //No clusters here
    {
      delete [] plane;
      return;
    }

  Int_t pad_num=0;
  Int_t time_num_max=0;
  Int_t ndata=0;
  Int_t tot_charge=0;
  if(fDebug)
    cout<<"Padrange "<<padrange[0]<<" "<<padrange[1]<<" timerange "<<timerange[0]<<" "<<timerange[1]<<endl;
  for(Int_t i=padrange[0]; i<=padrange[1]; i++)
    {
      Int_t max_charge = 0;
      Int_t time_num=0;
      for(Int_t j=timerange[0]; j<=timerange[1]; j++)
	{
	  Int_t charge = fRow[(AliL3Transform::GetNTimeBins()+1)*i + j].fCharge;
	  
	  if(charge <= 0) continue;

	  time_num++;
	  if(charge > max_charge)
	    {
	      max_charge = charge;
	      //time_num++;
	    }
	  if(fDebug)
	    cout<<"Filling padrow "<<fCurrentPadRow<<" pad "<<i<<" time "<<j<<" charge "<<charge<<endl;
	  tot_charge += charge;
	  ndata++;
	  if(ndata >= size)
	    {
	      cerr<<"Too many points; row "<<fCurrentPadRow<<" padrange "<<padrange[0]<<" "<<padrange[1]<<" timerange "
		  <<timerange[0]<<" "<<timerange[1]<<endl;
	      exit(5);
	    }

	  plane[ndata].u = (Double_t)i;
	  plane[ndata].v = (Double_t)j;
	  x[ndata]=ndata;
	  y[ndata]=charge;
	  s[ndata]= 1 + sqrt((Double_t)charge);
	}
      if(max_charge) //there was charge on this pad
	pad_num++;
      if(time_num_max < time_num)
	time_num_max = time_num;
    }
  
  if(pad_num <= 1 || time_num_max <=1 || n_overlaps > fNmaxOverlaps || ndata <= fit_pars) //too few to do fit
    {
      SetClusterfitFalse(track);
      if(fDebug)
	cout<<"Too few digits or too many overlaps: "<<pad_num<<" "<<time_num_max<<" "<<n_overlaps<<" ndata "<<ndata<<" fit_pars "<<fit_pars<<endl;
      delete [] plane;
      return;
    }

  
  Int_t npars = n_overlaps * NUM_PARS;
  if(fDebug)
    cout<<"Number of overlapping clusters "<<n_overlaps<<endl;
  Int_t ret = lev_marq_fit( x, y, s, ndata, a, lista, dev, npars, &chisq_f, f2gauss5 );
  
  if(ret<0)
    {
      SetClusterfitFalse(track);
      fFailed++;
      fFitError++;
      delete [] plane;
      return;
      //exit(5);
    }

  chisq_f /= (ndata-fit_pars);
  if(fDebug)
    cout<<"Chisq "<<chisq_f<<endl;
  
  k=-1;
  n_overlaps=0;
  while(k < track->GetNOverlaps(fCurrentPadRow))
    {
      AliL3ModelTrack *tr=0;
      if(k==-1)
	tr = track;
      else
	tr = (AliL3ModelTrack*)fProcessTracks->GetCheckedTrack(overlaps[k]);
      k++;
      if(!tr) continue;
      if(!tr->IsPresent(fCurrentPadRow))
	{
	  if(tr->IsSet(fCurrentPadRow)) continue;//This cluster has been set before
	  
	  if(chisq_f < fChiSqMax)//cluster fit is good enough
	    {
	      tot_charge = (Int_t)(a[n_overlaps*NUM_PARS+1] * a[n_overlaps*NUM_PARS+3] * a[n_overlaps*NUM_PARS+5]);
	      Float_t fpad = a[n_overlaps*NUM_PARS+2];
	      Float_t ftime = a[n_overlaps*NUM_PARS+4];
	      if(tot_charge < 0 || fpad < -1 || fpad > AliL3Transform::GetNPads(fCurrentPadRow) || 
		 ftime < -1 || ftime > AliL3Transform::GetNTimeBins())
		{
		  if(fDebug)
		    cout<<"AliL3ClusterFitter::Fatal result(s) in fit; in slice "<<fSlice<<" row "<<fCurrentPadRow
			<<"; pad "<<fpad<<" time "<<ftime<<" charge "<<tot_charge<<" xywidth "<<a[n_overlaps*NUM_PARS+3]
			<<" zwidth "<<a[n_overlaps*NUM_PARS+5]<<" peakcharge "<<a[n_overlaps*NUM_PARS+1]<<endl;
		  tr->SetCluster(fCurrentPadRow,0,0,0,0,0,0);
		  fFailed++;
		  fResultError++;
		  continue;
		}
	      
	      tr->SetCluster(fCurrentPadRow,fpad,ftime,tot_charge,0,0,pad_num);
	      if(fDebug)
		cout<<"Setting cluster in pad "<<a[n_overlaps*NUM_PARS+2]<<" time "<<a[n_overlaps*NUM_PARS+4]<<" charge "<<tot_charge<<endl;
	      /*
	      //Set the digits to used:
	      for(Int_t i=padrange[0]; i<=padrange[1]; i++)
	      for(Int_t j=timerange[0]; j<=timerange[1]; j++)
	      fRow[(AliL3Transform::GetNTimeBins()+1)*i + j].fUsed = kTRUE;
	      */
	      fFitted++;
	    }
	  else //fit was too bad
	    {
	      if(fDebug)
		cout<<"Cluster fit was too bad"<<endl;
	      tr->SetCluster(fCurrentPadRow,0,0,0,0,0,0);
	      fBadFitError++;
	      fFailed++;
	    }
	}
      n_overlaps++;
    }
  
  delete [] plane;
}

void AliL3ClusterFitter::SetClusterfitFalse(AliL3ModelTrack *track)
{
  //Cluster fit failed, so set the clusters to all the participating
  //tracks to zero.
  
  Int_t i=-1;
  Int_t *overlaps = track->GetOverlaps(fCurrentPadRow);
  while(i < track->GetNOverlaps(fCurrentPadRow))
    {
      AliL3ModelTrack *tr=0;
      if(i==-1)
	tr = track;
      else
	tr = (AliL3ModelTrack*)fProcessTracks->GetCheckedTrack(overlaps[i]);
      i++;
      if(!tr) continue;
      
      tr->SetCluster(fCurrentPadRow,0,0,0,0,0,0);
    }
}


void AliL3ClusterFitter::AddClusters()
{
  if(!fClusters)
    {
      fClusters = new AliL3SpacePointData[fNMaxClusters];
      fNClusters=0;
    }
  
  if(fDebug)
    cout<<"Writing cluster in slice "<<fSlice<<" patch "<<fPatch<<endl;
  
  AliL3TrackArray *tracks=0;
  if(fSeeding==kTRUE)
    tracks = fSeeds;
  else
    tracks = fTracks;
  
  if(!tracks)
    return;
  
  for(Int_t i=0; i<tracks->GetNTracks(); i++)
    {
      AliL3ModelTrack *tr = (AliL3ModelTrack*)tracks->GetCheckedTrack(i);
      if(!tr) continue;
      
      UInt_t *hitids = tr->GetHitNumbers();
      Int_t nhits = tr->GetNHits();
      for(Int_t i=fRowMax; i>=fRowMin; i--)
	{
	  if(fSeeding)
	    if(tr->GetClusterModel(i)->fSlice != fSlice) continue;
	  if(!tr->IsPresent(i)) continue;
	  fCurrentPadRow = i;
	  Float_t pad,time,xywidth,zwidth;
	  Int_t charge;
	  tr->GetPad(i,pad);
	  tr->GetTime(i,time);
	  tr->GetClusterCharge(i,charge);

	  if(pad < -1 || pad >= AliL3Transform::GetNPads(i) || 
	     time < -1 || time >= AliL3Transform::GetNTimeBins())
	    {
	      continue;
	      cout<<"slice "<<fSlice<<" row "<<i<<" pad "<<pad<<" time "<<time<<endl;
	      tr->Print();
	      exit(5);
	    }

	  tr->CalculateClusterWidths(i,kTRUE); //Parametrize errors
	  
	  tr->GetXYWidth(i,xywidth);
	  tr->GetZWidth(i,zwidth);
	  Float_t xyz[3];
	  Int_t sector,row;
	  AliL3Transform::Slice2Sector(fSlice,i,sector,row);
	  
	  AliL3Transform::Raw2Global(xyz,sector,row,pad,time);
	  
	  if(fNClusters >= fNMaxClusters)
	    {
	      cerr<<"AliL3ClusterFitter::AddClusters : Too many clusters "<<fNClusters<<endl;
	      exit(5);
	    }
	  fClusters[fNClusters].fX = xyz[0];
	  fClusters[fNClusters].fY = xyz[1];
	  fClusters[fNClusters].fZ = xyz[2];
	  fClusters[fNClusters].fCharge = charge;
	  fClusters[fNClusters].fPadRow = i;
	  Int_t pa = AliL3Transform::GetPatch(i);
	  if(xywidth==0 || zwidth==0)
	    cerr<<"AliL3ClusterFitter::AddClusters : Cluster with zero width"<<endl;
	  if(xywidth>0)
	    fClusters[fNClusters].fSigmaY2 = xywidth*pow(AliL3Transform::GetPadPitchWidth(pa),2);
	  else
	    fClusters[fNClusters].fSigmaY2 = 1;
	  if(zwidth>0)
	    fClusters[fNClusters].fSigmaZ2 = zwidth*pow(AliL3Transform::GetZWidth(),2);
	  else
	    fClusters[fNClusters].fSigmaZ2 = 1;
	  Int_t pat=fPatch;
	  if(fPatch==-1)
	    pat=0;
	  fClusters[fNClusters].fID = fNClusters + ((fSlice&0x7f)<<25)+((pat&0x7)<<22);
	  
	  if(nhits >= 159)
	    {
	      cerr<<"AliL3ClusterFitter::AddClusters : Cluster counter of out range "<<nhits<<endl;
	      exit(5);
	    }
	  hitids[nhits++] = fClusters[fNClusters].fID;
	  
#ifdef do_mc
	  Int_t trackID[3];
	  Int_t fpad = (Int_t)rint(pad);
	  Int_t ftime = (Int_t)rint(time);
	  if(fpad < 0)
	    fpad=0;
	  if(fpad >= AliL3Transform::GetNPads(i))
	    fpad = AliL3Transform::GetNPads(i)-1;
	  if(ftime<0)
	    ftime=0;
	  if(ftime >= AliL3Transform::GetNTimeBins())
	    ftime = AliL3Transform::GetNTimeBins()-1;
	  GetTrackID(fpad,ftime,trackID);
	  fClusters[fNClusters].fTrackID[0] = trackID[0];
	  fClusters[fNClusters].fTrackID[1] = trackID[1];
	  fClusters[fNClusters].fTrackID[2] = trackID[2];
#endif  
	  //cout<<"Setting id "<<trackID[0]<<" on pad "<<pad<<" time "<<time<<" row "<<i<<endl;
	  fNClusters++;
	}
      
      //Copy back the number of assigned clusters
      tr->SetNHits(nhits);
    }
}

void AliL3ClusterFitter::WriteTracks()
{
  if(!fSeeds)
    return;
  
  AliL3Compress *c = new AliL3Compress(-1,-1,fPath);
  c->WriteFile(fSeeds,"tracks_after.raw");
  delete c;
  
  Int_t clustercount=0;
  for(Int_t i=0; i<fSeeds->GetNTracks(); i++)
    {
      AliL3ModelTrack *tr = (AliL3ModelTrack*)fSeeds->GetCheckedTrack(i);
      if(!tr) continue;
      if(tr->GetNHits()==0)
	fSeeds->Remove(i);
      clustercount += tr->GetNHits();
      /*
	if(tr->GetPt() > 1 && tr->GetNPresentClusters() < 150) 
	tr->Print();
      */
    }
  cout<<"Writing "<<clustercount<<" clusters"<<endl;
  fSeeds->Compress();
  AliL3MemHandler mem;
  Char_t filename[1024];
  sprintf(filename,"%s/fitter/tracks_0.raw",fPath);
  mem.SetBinaryOutput(filename);
  mem.TrackArray2Binary(fSeeds);
  mem.CloseBinaryOutput();
  
}

void AliL3ClusterFitter::WriteClusters()
{
  AliL3MemHandler mem;
  if(fDebug)
    cout<<"Write "<<fNClusters<<" clusters to file"<<endl;
  Char_t filename[1024];
  sprintf(filename,"%s/fitter/points_0_%d_%d.raw",fPath,fSlice,fPatch);
  mem.SetBinaryOutput(filename);
  mem.Memory2Binary(fNClusters,fClusters);
  mem.CloseBinaryOutput();
  mem.Free();
  
  delete [] fClusters;
  fClusters=0;
  fNClusters=0;
}

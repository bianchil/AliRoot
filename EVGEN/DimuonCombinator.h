#ifndef _DimuonCombinator_H
#define _DimuonCombinator_H
#include "GParticle.h"
#include <TBrowser.h>
#include <TList.h>
#include <TTree.h>
#include <TROOT.h>


class DimuonCombinator:
    public TObject 
{
public:
    DimuonCombinator(TClonesArray* Partarray){
	fPartArray=Partarray;
	fNParticle=fPartArray->GetEntriesFast();
	
	fimuon1 =0;
	fimuon2 =0;
	fmuon1  =0;
	fmuon2  =0;
	fimin1  = 0;
	fimin2  = 0;
	fimax1  = fNParticle;
	fimax2  = fNParticle;
	fPtMin  =0;
	fEtaMin =-10;
	fEtaMax =-10;
	fRate1=1.;
	fRate2=1.;
    }
//    
//  Iterators    
    GParticle* FirstMuon();
    GParticle* NextMuon();
    GParticle* FirstMuonSelected();
    GParticle* NextMuonSelected();
    
    void FirstMuonPair(GParticle* & muon1, GParticle* & muon2);
    void NextMuonPair(GParticle* & muon1, GParticle* & muon2);
    void FirstMuonPairSelected(GParticle* & muon1, GParticle* & muon2);
    void NextMuonPairSelected(GParticle* & muon1, GParticle* & muon2);
    void ResetRange();
    void SetFirstRange (Int_t from, Int_t to);
    void SetSecondRange(Int_t from, Int_t to);    
//  Cuts
    void SetPtMin(Float_t ptmin){fPtMin=ptmin;}
    void SetEtaCut(Float_t etamin, Float_t etamax){fEtaMin=etamin; fEtaMax=etamax;}    
    Int_t Selected(GParticle* part);
    Int_t Selected(GParticle* part1, GParticle* part2);
// Kinematics
    Float_t Mass(GParticle* part1, GParticle* part);
    Float_t PT(GParticle* part1, GParticle* part);
    Float_t Pz(GParticle* part1, GParticle* part);
    Float_t Y(GParticle* part1, GParticle* part);
// Response
    void SmearGauss(Float_t width, Float_t & value);
    
// Weight
    void    SetRate(Float_t rate){fRate1=rate;}
    void    SetRate(Float_t rate1, Float_t rate2 ){fRate1=rate1; fRate2=rate2;}
    Float_t Weight(GParticle* part);
    Float_t Weight(GParticle* part1, GParticle* part);

 private:
    void FirstPartner();
    void NextPartner();
    void FirstPartnerSelected();
    void NextPartnerSelected();

    GParticle* Partner();
    Int_t Type(GParticle *part){return part->GetKF();}
private:
    TClonesArray *fPartArray;
    Int_t fNParticle;
    Int_t fimuon1;
    Int_t fimuon2;
    Int_t fimin1;
    Int_t fimin2;
    Int_t fimax1;
    Int_t fimax2;
    Float_t fRate1;
    Float_t fRate2;
    GParticle *fmuon1;
    GParticle *fmuon2;
    Float_t fPtMin;
    Float_t fEtaMin;
    Float_t fEtaMax;
    ClassDef(DimuonCombinator,1)
};
#endif





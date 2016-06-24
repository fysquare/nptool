/*****************************************************************************
 * Copyright (C) 2009-2014    this file is part of the NPTool Project        *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien MATTA  contact address: a.matta@surrey.ac.uk      *
 *                                                                           *
 * Creation Date  : march 2025                                               *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 * Class describing the property of an Analysis object                       *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
#include<iostream>
using namespace std;
#include"Analysis.h"
#include"NPAnalysisFactory.h"
#include"NPDetectorManager.h"
#include"NPOptionManager.h"
#include"RootOutput.h"
#include"RootInput.h"
////////////////////////////////////////////////////////////////////////////////
Analysis::Analysis(){
}
////////////////////////////////////////////////////////////////////////////////
Analysis::~Analysis(){
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::Init(){
    Lassa = (TLassaPhysics*) m_DetectorManager->GetDetector("LASSAArray");
    InitialConditions = new TInitialConditions();
    InitOutputBranch();
    InitInputBranch();
    
    f_proton = new TF1("f_proton","1 - TMath::Exp(-(x-0.0918309)/27.7746)",0,10);
    f_deuton = new TF1("f_deuton","1 - TMath::Exp(-(x-0.0434552)/21.134)",0,10);
    f_triton = new TF1("f_triton","1 - TMath::Exp(-(x-0.0353106)/17.9059)",0,10);
    
    //	Energy loss table: the G4Table are generated by the simulation
    Proton_CsI = EnergyLoss("proton_CsI.G4table","G4Table",100 );
    Deuton_CsI = EnergyLoss("deuteron_CsI.G4table","G4Table",100 );
    Triton_CsI = EnergyLoss("triton_CsI.G4table","G4Table",100 );
    He3_CsI = EnergyLoss("He3_CsI.G4table","G4Table",100 );

    
    proton = new NPL::Nucleus("1H");
    deuton = new NPL::Nucleus("2H");
    triton = new NPL::Nucleus("3H");
    helium3 = new NPL::Nucleus("3He");
    alpha = new NPL::Nucleus("4He");
    beam = new NPL::Nucleus("112Sn");
    target = new NPL::Nucleus("112Sn");
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::TreatEvent(){
    // Reinitiate calculated variable
    ReInitValue();
    
    totalEvents++;

    double BeamEnergy = 120*beam->GetA();
    
    TVector3 fImpulsionLab_beam             = TVector3(0,0,sqrt(BeamEnergy*BeamEnergy + 2*BeamEnergy*beam->Mass()));
    TLorentzVector fEnergyImpulsionLab_beam = TLorentzVector(fImpulsionLab_beam,beam->Mass()+BeamEnergy);
    
    TVector3 fImpulsionLab_target             = TVector3(0,0,0);
    TLorentzVector fEnergyImpulsionLab_target = TLorentzVector(fImpulsionLab_target,target->Mass());
    
    TLorentzVector fEnergyImpulsionLab_total;
    fEnergyImpulsionLab_total = fEnergyImpulsionLab_beam + fEnergyImpulsionLab_target;
    
    double BetaCM = fEnergyImpulsionLab_total.Beta();

    
    InitialEnergy = InitialConditions->GetKineticEnergy(0);
    double EDelta = 2.0;

    if(Lassa->ThickSi_E.size()>0) InitialEnergy_Lassa = InitialEnergy;
    double phi_in = acos(InitialConditions->GetMomentumDirectionX(0)/sin(InitialConditions->GetThetaCM(0)*deg));
    
    ECM_initial = alpha->GetEnergyCM(InitialEnergy, InitialConditions->GetThetaCM(0)*deg, phi_in, BetaCM);
    
    if(Lassa->ThickSi_E.size()>0){
        ECM_initial_Lassa = alpha->GetEnergyCM(InitialEnergy_Lassa, InitialConditions->GetThetaCM(0)*deg, phi_in, BetaCM);
    }
    else ECM_initial_Lassa = -100;
    ///////////////////////////LOOP on Lassa Hit//////////////////////////////////
    if(Lassa->ThickSi_E.size() == 1){
        detectedEvents++;
        
        //for(unsigned int countLassa = 0 ; countLassa < Lassa->ThickSi_E.size(); countLassa++){
        TelescopeNumber = Lassa->TelescopeNumber[0];

        X = Lassa->GetPositionOfInteraction(0).X();
        Y = Lassa->GetPositionOfInteraction(0).Y();
        Z = Lassa->GetPositionOfInteraction(0).Z();
  
        TVector3 PositionOnLassa = TVector3(X,Y,Z);
        TVector3 ZUnit = TVector3(0,0,1);

        double X_target = InitialConditions->GetIncidentPositionX();
        double Y_target = InitialConditions->GetIncidentPositionY();
        double Z_target = InitialConditions->GetIncidentPositionZ();

        TVector3 PositionOnTarget = TVector3(0,0,0);
        //TVector3 PositionOnTarget = TVector3(X_target,Y_target,Z_target);
        TVector3 HitDirection = PositionOnLassa-PositionOnTarget;
        TVector3 HitDirectionUnit = HitDirection.Unit();

        TVector3 BeamDirection = TVector3(0,0,1);
        //TVector3 BeamDirection = InitialConditions->GetBeamDirection();
        //double XBeam = BeamDirection.X();
        //double YBeam = BeamDirection.Y();
        //double ZBeam = BeamDirection.Z();

        ThetaLab = BeamDirection.Angle(HitDirection);
        PhiLab = HitDirection.Phi();

        E_ThickSi = Lassa->ThickSi_E[0];
      
        ELab = E_ThickSi;
        ELab_nucl = E_ThickSi;
        
        //R_alpha = (4.59+1.192*pow(E_ThickSi,1.724));//*cos(ThetaLab);//Lise
        R_alpha = (4.90+1.17883*pow(E_ThickSi,1.73497));//*cos(ThetaLab);//Geant4
        
        if(Lassa->CsI_E.size()==1){
            E_CsI = Lassa->CsI_E[0];
            ELab += E_CsI;
            
            PID = pow(E_ThickSi+E_CsI,1.78)-pow(E_CsI,1.78);

            //Try to simulate the nuclear reaction loss
            //ThicknessCsI = Proton_CsI.EvaluateMaterialThickness(0*MeV, Lassa->CsI_E[0]*MeV, 200*millimeter, 0.1*millimeter);
            //ThicknessCsI = Deuton_CsI.EvaluateMaterialThickness(0*MeV, Lassa->CsI_E[0]*MeV, 200*millimeter, 0.1*millimeter);
            //ThicknessCsI = Triton_CsI.EvaluateMaterialThickness(0*MeV, Lassa->CsI_E[0]*MeV, 200*millimeter, 0.1*millimeter);
            
            //double eval = f_proton->Eval(ThicknessCsI/10);
            //double eval = f_deuton->Eval(ThicknessCsI/10);
            /*double eval = f_triton->Eval(ThicknessCsI/10);
            double Random_value = Rand.Uniform(0,1);
            
            if(Random_value>eval) ELab_nucl += E_CsI;
            else ELab_nucl += Rand.Uniform(0,E_CsI);*/
            
        }
    
        if(fabs(InitialEnergy-ELab)>EDelta){
            ELab = -100;
        }

        if(fabs(InitialEnergy-ELab_nucl)>EDelta) ELab_nucl = -100;
        
        if(ELab>0){
            ECM = alpha->GetEnergyCM(ELab, ThetaLab, PhiLab, BetaCM);
        }
        else ECM = -100;
        
        ThetaLab = ThetaLab/deg;
        PhiLab = PhiLab/deg;
    }
}
////////////////////////////////////////////////////////////////////////////////
void Analysis::End(){

  /*geomEff = 100*((double)detectedEvents)/((double)totalEvents);

  peakEff = 100*((double)peakEvents)/((double)detectedEvents);

  cout << endl;
  cout << "Total Events: " << totalEvents << endl;
  cout << "Detected Events: " << detectedEvents << endl;
  cout << "PeakEvents: " << peakEvents << endl;

  cout << "Geometric Efficiency: " << geomEff << endl;
  cout << "Peak Efficiency: " << peakEff << endl;*/

}

////////////////////////////////////////////////////////////////////////////////
void Analysis::InitOutputBranch() {
    RootOutput::getInstance()->GetTree()->Branch("ThicknessCsI",&ThicknessCsI,"ThicknessCsI/D");
    RootOutput::getInstance()->GetTree()->Branch("ELab",&ELab,"ELab/D");
    RootOutput::getInstance()->GetTree()->Branch("ECM",&ECM,"ECM/D");
    RootOutput::getInstance()->GetTree()->Branch("ELab_nucl",&ELab_nucl,"ELab_nucl/D");
    RootOutput::getInstance()->GetTree()->Branch("ThetaLab",&ThetaLab,"ThetaLab/D");
    RootOutput::getInstance()->GetTree()->Branch("PhiLab",&PhiLab,"PhiLab/D");
    RootOutput::getInstance()->GetTree()->Branch("InitialEnergy",&InitialEnergy,"InitialEnergy/D");
    RootOutput::getInstance()->GetTree()->Branch("InitialEnergy_Lassa",&InitialEnergy_Lassa,"InitialEnergy_Lassa/D");
    RootOutput::getInstance()->GetTree()->Branch("ECM_initial",&ECM_initial,"ECM_initial/D");
    RootOutput::getInstance()->GetTree()->Branch("ECM_initial_Lassa",&ECM_initial_Lassa,"ECM_initial_Lassa/D");
    RootOutput::getInstance()->GetTree()->Branch("E_ThickSi",&E_ThickSi,"E_ThickSi/D");
    RootOutput::getInstance()->GetTree()->Branch("E_CsI",&E_CsI,"E_CsI/D");
    RootOutput::getInstance()->GetTree()->Branch("R_alpha",&R_alpha,"R_alpha/D");
    RootOutput::getInstance()->GetTree()->Branch("PID",&PID,"PID/D");
    //  RootOutput::getInstance()->GetTree()->Branch("peakEvents",&peakEvents,"peakEvents/I");
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::InitInputBranch(){
  RootInput:: getInstance()->GetChain()->SetBranchStatus("InitialConditions",true );
  RootInput:: getInstance()->GetChain()->SetBranchStatus("fIC_*",true );
  RootInput:: getInstance()->GetChain()->SetBranchAddress("InitialConditions",&InitialConditions);
}

////////////////////////////////////////////////////////////////////////////////     
void Analysis::ReInitValue(){
    E_ThickSi = -100;
    E_CsI =-100;
    ELab = -100;
    ECM = -100;
    ELab_nucl = -100;
    ThetaLab = -100;
    PhiLab = -100;
    X = -100;
    Y = -100;
    Z = -100;
    TelescopeNumber = -1;
    InitialEnergy = -100;
    InitialEnergy_Lassa = -100;
    ECM_initial_Lassa = -100;
    ECM_initial = -100;
    ThicknessCsI = -1;
    R_alpha = -100;
    PID = -1;
}

////////////////////////////////////////////////////////////////////////////////
//            Construct Method to be pass to the DetectorFactory              //
////////////////////////////////////////////////////////////////////////////////
NPL::VAnalysis* Analysis::Construct(){
  return (NPL::VAnalysis*) new Analysis();
}

////////////////////////////////////////////////////////////////////////////////
//            Registering the construct method to the factory                 //
////////////////////////////////////////////////////////////////////////////////
extern "C"{
    class proxy{
    public:
        proxy(){
            NPL::AnalysisFactory::getInstance()->SetConstructor(Analysis::Construct);
        }
    };
    proxy p;
}


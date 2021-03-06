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
    Hira = (THiraPhysics*) m_DetectorManager->GetDetector("HiraTelescope");
    InitialConditions=new TInitialConditions();
    InitOutputBranch();
    InitInputBranch();
    Rand = TRandom3();
    //TransferReaction= new NPL::Reaction("8B(p,4He)5Be@400");
    //TransferReaction->ReadConfigurationFile(NPOptionManager::getInstance()->GetReactionFile());
    
    TargetThickness = m_DetectorManager->GetTargetThickness()*micrometer;
    //	Energy loss table: the G4Table are generated by the simulation
    Triton_CD2 = EnergyLoss("triton_CD2.G4table","G4Table",100 );
    Triton_CH2 = EnergyLoss("triton_CD2.G4table","G4Table",100 );
    Deuteron_CH2 = EnergyLoss("deuteron_CH2.G4table","G4Table",100 );
    He3_CD2 = EnergyLoss("He3_CD2.G4table","G4Table",100 );
    He4_CH2 = EnergyLoss("alpha_CH2.G4table","G4Table",100 );
    
    f_proton = new TF1("f_proton","1 - TMath::Exp(-(x-0.0918309)/27.7746)",0,10);
    f_deuton = new TF1("f_deuton","1 - TMath::Exp(-(x-0.0434552)/21.134)",0,10);
    f_triton = new TF1("f_triton","1 - TMath::Exp(-(x-0.0353106)/17.9059)",0,10);
    f_3He = new TF1("f_3He","1 - TMath::Exp(-(x-0.0463954)/20.8906)",0,10);
    
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
    beam = new NPL::Nucleus("48Ca");
    target = new NPL::Nucleus("124Sn");
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::TreatEvent(){
    // Reinitiate calculated variable
    ReInitValue();
    
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
    
    if(Hira->EventMultiplicity==1) InitialEnergy_Hira = InitialEnergy;
    else InitialEnergy_Hira = -100;
    double phi_in = acos(InitialConditions->GetMomentumDirectionX(0)/sin(InitialConditions->GetThetaCM(0)*deg));
    
    ECM_initial = proton->GetEnergyCM(InitialEnergy, InitialConditions->GetThetaCM(0)*deg, phi_in, BetaCM);
    if(Hira->ThickSi_E.size()==1){
        ECM_initial_Hira = proton->GetEnergyCM(InitialEnergy_Hira, InitialConditions->GetThetaCM(0)*deg, phi_in, BetaCM);
    }
    else ECM_initial_Hira = -100;
    //TransferReaction->SetBeamEnergy(BeamEnergy);
    //////////////////////////// LOOP on Hira Hit //////////////////
    if(Hira -> EventMultiplicity == 1){
        for(unsigned int countHira = 0 ; countHira < Hira->EventMultiplicity ; countHira++){
            event += 1;
            TelescopeNumber = Hira->TelescopeNumber[countHira];
            
            TargetThickness = m_DetectorManager->GetTargetThickness();
            
            X = Hira->GetPositionOfInteraction(countHira).X();
            Y = Hira->GetPositionOfInteraction(countHira).Y();
            Z = Hira->GetPositionOfInteraction(countHira).Z();
            
            TVector3 PositionOnHira = TVector3(X,Y,Z);
            TVector3 ZUnit = TVector3(0,0,1);
            
            X_target	= InitialConditions->GetIncidentPositionX();// + Rand.Gaus(0,2.);
            Y_target    = InitialConditions->GetIncidentPositionY();// + Rand.Gaus(0,2.);
            double Z_target	= InitialConditions->GetIncidentPositionZ();
            
            //TVector3 PositionOnTarget = TVector3(X_target,Y_target,Z_target);
            TVector3 PositionOnTarget = TVector3(0,0,0);
            TVector3 HitDirection	= PositionOnHira - PositionOnTarget;
            TVector3 HitDirectionUnit = HitDirection.Unit();
            TVector3 BeamDirection = TVector3(0,0,1);
            
            ThetaLab = BeamDirection.Angle(HitDirection);
            PhiLab = HitDirection.Phi();
            
            
            E_ThickSi = Hira->ThickSi_E[countHira];
            //E_ThinSi = Hira->ThinSi_E[countHira];
            
            ELab = E_ThickSi;
            if(Hira->CsI_E[countHira]>0){
                E_CsI = Hira->CsI_E[countHira];
                ELab += E_CsI;
                
                PID = pow(E_ThickSi+E_CsI,1.78)-pow(E_CsI,1.78);
                
                //Try to simulate the nuclear reaction loss
                //ThicknessCsI = Proton_CsI.EvaluateMaterialThickness(0*MeV, Hira->CsI_E[0]*MeV, 200*millimeter, 0.1*millimeter);
                //ThicknessCsI = Deuton_CsI.EvaluateMaterialThickness(0*MeV, Hira->CsI_E[0]*MeV, 200*millimeter, 0.1*millimeter);
                //ThicknessCsI = Triton_CsI.EvaluateMaterialThickness(0*MeV, Hira->CsI_E[0]*MeV, 200*millimeter, 0.1*millimeter);
                
                
                //double eval = f_proton->Eval(ThicknessCsI/10);
                //double eval = f_deuton->Eval(ThicknessCsI/10);
                //double eval = f_triton->Eval(ThicknessCsI/10);
                //double Random_value = Rand.Uniform(0,1);
                 
            }
            
            if(fabs(InitialEnergy-ELab)>EDelta){
                ELab = -100;
            }
            
            
            ////////////////////////////////////////////////////////////////////////
            // Calculation of the center of mass energy depending on the particle //
            ////////////////////////////////////////////////////////////////////////
            ECM = proton->GetEnergyCM(ELab, ThetaLab, PhiLab, BetaCM);
            ThetaCM = proton->GetThetaCM(ELab, ThetaLab, PhiLab, BetaCM)/deg;
            /*if(PID>100 && PID<175){
                ECM = proton->GetEnergyCM(ELab, ThetaLab, PhiLab, BetaCM);
                ThetaCM = proton->GetThetaCM(ELab, ThetaLab, PhiLab, BetaCM)/deg;
                Particle = 1;
            }
            else if(PID>175 && PID<265){
                ECM = deuton->GetEnergyCM(ELab, ThetaLab, PhiLab, BetaCM);
                ThetaCM = deuton->GetThetaCM(ELab, ThetaLab, PhiLab, BetaCM)/deg;
                Particle = 2;
            }
            else if(PID>265 && PID<350){
                ECM = triton->GetEnergyCM(ELab, ThetaLab, PhiLab, BetaCM);
                ThetaCM = triton->GetThetaCM(ELab, ThetaLab, PhiLab, BetaCM)/deg;
                Particle = 3;
            }
            else if(PID>1100 && PID<1370){
                ECM = helium3->GetEnergyCM(ELab, ThetaLab, PhiLab, BetaCM);
                ThetaCM = helium3->GetThetaCM(ELab, ThetaLab, PhiLab, BetaCM)/deg;
                Particle = 4;
            }
            else if(PID>1370 && PID<1700){
                ECM = alpha->GetEnergyCM(ELab, ThetaLab, PhiLab, BetaCM);
                ThetaCM = alpha->GetThetaCM(ELab, ThetaLab, PhiLab, BetaCM)/deg;
                Particle = 5;
            }
            else{
                ECM = -100;
                ThetaCM = -100;
                Particle = -1;
            }*/
            ////////////////////////////////////////////////////////////////////////
            
            double ImpulsionLab    = sqrt(ELab*ELab + 2*ELab*proton->Mass());
            double ImpulsionLabX   = ImpulsionLab*sin(ThetaLab)*cos(PhiLab);
            double ImpulsionLabY   = ImpulsionLab*sin(ThetaLab)*sin(PhiLab);
            double ImpulsionLabZ   = ImpulsionLab*cos(ThetaLab);
            
            TVector3 VImpulsionLAB           = TVector3(ImpulsionLabX, ImpulsionLabY, ImpulsionLabZ);
            TLorentzVector LVEnergyImpulsionLAB    = TLorentzVector(VImpulsionLAB,proton->Mass()+ELab);
            
            TLorentzVector LVEnergyImpulsionCM     = LVEnergyImpulsionLAB;
            LVEnergyImpulsionCM.Boost(0,0,-BetaCM);
            
            Rapidity_Lab = LVEnergyImpulsionLAB.Rapidity();
            Rapidity_CM = LVEnergyImpulsionCM.Rapidity();
            Pper = LVEnergyImpulsionCM.Pt();
            
            /*double Px_Lab = LVEnergyImpulsionLAB.Px();
            double Py_Lab = LVEnergyImpulsionLAB.Py();
            double Pz_Lab = LVEnergyImpulsionLAB.Pz();
            
            double Px_CM = LVEnergyImpulsionCM.Px();
            double Py_CM = LVEnergyImpulsionCM.Py();
            double Pz_CM = LVEnergyImpulsionCM.Pz();
            
            double Pper_Lab = sqrt(Px_Lab*Px_Lab+Py_Lab*Py_Lab);
            double Pper_CM = sqrt(Px_CM*Px_CM+Py_CM*Py_CM);
            
            double test_Lab = 0.5*log((proton->Mass()+ELab + Pz_Lab)/(proton->Mass()+ELab - Pz_Lab));
            double test_CM = 0.5*log((proton->Mass()+ECM + Pz_CM)/(proton->Mass()+ECM - Pz_CM));
            
            cout << "********************************" << endl;
            cout << Pper_Lab << " " << Pper_CM << endl;
            cout << Rapidity_Lab << " " << test_Lab << endl;
            cout << Rapidity_CM << " " << test_CM << endl;
            cout << endl;*/
            
            ThetaLab = ThetaLab/deg;
            PhiLab = PhiLab/deg;
            
        }//end loop Hira
    }//end if Hira
    
    
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::End(){
    
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::InitOutputBranch() {
    RootOutput::getInstance()->GetTree()->Branch("ThicknessCsI",&ThicknessCsI,"ThicknessCsI/D");
    RootOutput::getInstance()->GetTree()->Branch( "ThetaLab" , &ThetaLab , "ThetaLab/D" );
    RootOutput::getInstance()->GetTree()->Branch( "PhiLab" , &PhiLab , "PhiLab/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch("ThetaCM", &ThetaCM,"ThetaCM/D") 	;
    RootOutput::getInstance()->GetTree()->Branch( "E_ThinSi" , &E_ThinSi , "E_ThinSi/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "E_ThickSi" , &E_ThickSi , "E_ThickSi/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "E_CsI" , &E_CsI , "E_CsI/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "ELab" , &ELab , "ELab/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "ECM" , &ECM , "ECM/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "ECM_initial" , &ECM_initial , "ECM_initial/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "ECM_initial_Hira" , &ECM_initial_Hira , "ECM_initial_Hira/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch("ExcitationEnergy", &ExcitationEnergy,"ExcitationEnergy/D") ;
    RootOutput::getInstance()->GetTree()->Branch( "X" , &X , "X/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "Y" , &Y , "Y/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "Z" , &Z , "Z/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "TelescopeNumber" , &TelescopeNumber , "TelescopeNumber/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "X_target" , &X_target , "X_target/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch( "Y_target" , &Y_target , "Y_target/D" )  ;
    RootOutput::getInstance()->GetTree()->Branch("InitialEnergy",&InitialEnergy,"InitialEnergy/D");
    RootOutput::getInstance()->GetTree()->Branch("InitialEnergy_Hira",&InitialEnergy_Hira,"InitialEnergy_Hira/D");
    RootOutput::getInstance()->GetTree()->Branch("Rapidity_CM",&Rapidity_CM,"Rapidity_CM/D");
    RootOutput::getInstance()->GetTree()->Branch("Rapidity_Lab",&Rapidity_Lab,"Rapidity_Lab/D");
    RootOutput::getInstance()->GetTree()->Branch("Pper",&Pper,"Pper/D");
    RootOutput::getInstance()->GetTree()->Branch("PID",&PID,"PID/D");
    RootOutput::getInstance()->GetTree()->Branch("Particle",&Particle,"Particle/I");
    //RootOutput::getInstance()->GetTree()-> Branch("InteractionCoordinates","TInteractionCoordinates",&InteractionCoordinates);
    //RootOutput::getInstance()->GetTree()->Branch("InitialConditions","TInitialConditions",&InitialConditions);
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::InitInputBranch(){
    RootInput:: getInstance()->GetChain()->SetBranchStatus("InitialConditions",true );
    RootInput:: getInstance()->GetChain()->SetBranchStatus("fIC_*",true );
    RootInput:: getInstance()->GetChain()->SetBranchAddress("InitialConditions",&InitialConditions);
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::ReInitValue(){
    ThicknessCsI= -100;
    E_ThinSi    = -100;
    E_ThickSi   = -100;
    E_CsI       = -100;
    ELab        = -100;
    ECM_initial = -100;
    ECM_initial_Hira = -100;
    ECM         = -100;
    ThetaLab    = -100;
    PhiLab      = -100;
    ThetaCM     = -100;
    ExcitationEnergy = -100;
    X           = -100;
    Y           = -100;
    Z           = -100;
    TelescopeNumber = -1;
    X_target    = -100;
    Y_target    = -100;
    InitialEnergy = -100;
    InitialEnergy_Hira = -100;
    Rapidity_Lab = -100;
    Rapidity_CM = -100;
    Pper = -100;
    PID = -100;
    Particle = -1;
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
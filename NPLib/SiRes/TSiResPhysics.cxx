/*****************************************************************************
 * Copyright (C) 2009-2013   this file is part of the NPTool Project         *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien MATTA  contact address: matta@ipno.in2p3.fr       *
 *                                                                           *
 * Creation Date  : november 2009                                            *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *  This class hold SiRes  Physics                                         *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/

//   NPL
#include "TSiResPhysics.h"
#include "../include/RootOutput.h"
#include "../include/RootInput.h"

//   STL
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <stdlib.h>
using namespace std;

//   ROOT
#include "TChain.h"

//   tranform an integer to a string
string itoa(int value)
{
   char buffer [33];
   sprintf(buffer,"%d",value);
   return buffer;
}

ClassImp(TSiResPhysics)
///////////////////////////////////////////////////////////////////////////
TSiResPhysics::TSiResPhysics()
   {      
      NumberOfDetector = 0 ;
      EventData = new TSiResData ;
      EventPhysics = this ;
      m_SiRes_E_Threshold   = 0;   
      m_SiRes_RAW_Threshold   = 0;     
      m_SiRes_EBack_Threshold   = 0;   
      m_SiRes_RAWBack_Threshold   = 0;     
   }
   
///////////////////////////////////////////////////////////////////////////
TSiResPhysics::~TSiResPhysics()
   {}
   
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::Clear()
   {
      DetectorNumber.clear() ;
      ChannelNumber.clear() ;
      Energy.clear() ;
      Time.clear() ;
      EnergyBack.clear() ;
      x.clear() ;
      y.clear() ;
   }
   
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::ReadConfiguration(string Path) 
   {
      ifstream ConfigFile           ;
      ConfigFile.open(Path.c_str()) ;
      string LineBuffer             ;
      string DataBuffer             ;

      bool check_Theta = false          ;
      bool check_Phi  = false           ;
      bool check_R     = false          ;
      bool check_Thickness = false      ;
      bool check_Radius = false         ;
      bool check_LeadThickness = false  ;
      bool check_Scintillator = false   ;
      bool check_Height = false         ;
      bool check_Width = false          ;
      bool check_Shape = false          ;
      bool check_X = false              ;
      bool check_Y = false              ;
      bool check_Z = false              ;      
      bool ReadingStatus = false        ;

    while (!ConfigFile.eof()) 
       {
         
         getline(ConfigFile, LineBuffer);

         //   If line is a Start Up SiRes bloc, Reading toggle to true      
         if (LineBuffer.compare(0, 5, "SiRes") == 0) 
            {
               cout << "///" << endl ;
               cout << "Platic found: " << endl ;        
               ReadingStatus = true ;
            }
            
         //   Else don't toggle to Reading Block Status
         else ReadingStatus = false ;
         
         //   Reading Block
         while(ReadingStatus)
            {
               // Pickup Next Word 
               ConfigFile >> DataBuffer ;

               //   Comment Line 
               if (DataBuffer.compare(0, 1, "%") == 0) {   ConfigFile.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );}

                  //   Finding another telescope (safety), toggle out
               else if (DataBuffer.compare(0, 5, "SiRes") == 0) {
                  cout << "WARNING: Another Detector is find before standard sequence of Token, Error may occured in Telecope definition" << endl ;
                  ReadingStatus = false ;
               }
                              
                                    //Angle method
               else if (DataBuffer=="THETA=") {
                  check_Theta = true;
                  ConfigFile >> DataBuffer ;
                  cout << "Theta:  " << atof(DataBuffer.c_str()) << "deg" << endl;
               }

               else if (DataBuffer=="PHI=") {
                  check_Phi = true;
                  ConfigFile >> DataBuffer ;
                  cout << "Phi:  " << atof( DataBuffer.c_str() ) << "deg" << endl;
               }

               else if (DataBuffer=="R=") {
                  check_R = true;
                  ConfigFile >> DataBuffer ;
                  cout << "R:  " << atof( DataBuffer.c_str() ) << "mm" << endl;
               }
               
               //Position method
               else if (DataBuffer=="X=") {
                  check_X = true;
                  ConfigFile >> DataBuffer ;
                  cout << "X:  " << atof( DataBuffer.c_str() ) << "mm" << endl;
               }

               else if (DataBuffer=="Y=") {
                  check_Y = true;
                  ConfigFile >> DataBuffer ;
                  cout << "Y:  " << atof( DataBuffer.c_str() ) << "mm"<< endl;
               }

               else if (DataBuffer=="Z=") {
                  check_Z = true;
                  ConfigFile >> DataBuffer ;
                  cout << "Z:  " << atof( DataBuffer.c_str() ) << "mm" << endl;
               }
               
               
               //General
               else if (DataBuffer=="Shape=") {
                  check_Shape = true;
                  ConfigFile >> DataBuffer ;
                  cout << "Shape:  " << DataBuffer << endl;
               }
               
               // Cylindrical shape
               else if (DataBuffer== "Radius=") {
                  check_Radius = true;
                  ConfigFile >> DataBuffer ;
                  cout << "SiRes Radius:  " << atof( DataBuffer.c_str() ) << "mm" << endl;
               }
               
               // Squared shape
               else if (DataBuffer=="Width=") {
                  check_Width = true;
                  ConfigFile >> DataBuffer ;
                  cout << "SiRes Width:  " <<atof( DataBuffer.c_str() ) << "mm" << endl;
               }
               
               else if (DataBuffer== "Height=") {
                  check_Height = true;
                  ConfigFile >> DataBuffer ;
                  cout << "SiRes Height:  " << atof( DataBuffer.c_str() ) << "mm" << endl;
               }
               
               // Common
               else if (DataBuffer=="Thickness=") {
                  check_Thickness = true;
                  ConfigFile >> DataBuffer ;
                  cout << "SiRes Thickness:  " << atof( DataBuffer.c_str() ) << "mm" << endl;
               }
               
               else if (DataBuffer== "Scintillator=") {
                  check_Scintillator = true ;
                  ConfigFile >> DataBuffer ;
                  cout << "SiRes Scintillator type:  " << DataBuffer << endl;
               }
               
               else if (DataBuffer=="LeadThickness=") {
                  check_LeadThickness = true;
                  ConfigFile >> DataBuffer ;
                  cout << "Lead Thickness :  " << atof( DataBuffer.c_str() ) << "mm" << endl;
               }
                                                
               ///////////////////////////////////////////////////
               //   If no Detector Token and no comment, toggle out
               else 
                  {ReadingStatus = false; cout << "Wrong Token Sequence: Getting out " << DataBuffer << endl ;}
               
                  /////////////////////////////////////////////////
                  //   If All necessary information there, toggle out
               
               if ( check_Theta && check_Phi && check_R && check_Thickness && check_Radius &&   check_LeadThickness && check_Scintillator &&   check_Height &&   check_Width && check_Shape && check_X && check_Y && check_Z ) 
                  { 
                     NumberOfDetector++;
                     
                     //   Reinitialisation of Check Boolean  
                     check_Theta = false          ;
                     check_Phi  = false           ;
                     check_R     = false          ;
                     check_Thickness = false      ;
                     check_Radius = false         ;
                     check_LeadThickness = false  ;
                     check_Scintillator = false   ;
                     check_Height = false         ;
                     check_Width = false          ;
                     check_Shape = false          ;
                     check_X = false              ;
                     check_Y = false              ;
                     check_Z = false              ;
                     ReadingStatus = false        ;   
                     cout << "///"<< endl         ;                
                  }
            }
      }
   }

///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::AddParameterToCalibrationManager()
   {
      CalibrationManager* Cal = CalibrationManager::getInstance();
      
      for(int i = 0 ; i < NumberOfDetector ; i++)
         {
            for( int j = 0 ; j < 4 ; j++)
               {
                  Cal->AddParameter("SiRes", "Detector"+itoa(i+1)+"_E","SiRes_Detector"+itoa(i+1)+"_E")   ;
               }
               Cal->AddParameter("SiRes", "Detector"+itoa(i+1)+"_EBack","SiRes_Detector"+itoa(i+1)+"_EBack")   ;   
               Cal->AddParameter("SiRes", "Detector"+itoa(i+1)+"_T","SiRes_Detector"+itoa(i+1)+"_T")   ;   
      
         }
   }
   
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::InitializeRootInputRaw() 
   {
      TChain* inputChain = RootInput::getInstance()->GetChain()     ;
      inputChain->SetBranchStatus ( "SiRes"       , true )        ;
      inputChain->SetBranchStatus ( "fSiRes_*"    , true )        ;
      inputChain->SetBranchAddress( "SiRes"       , &EventData )  ;
   }
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::InitializeRootInputPhysics()
   {
      TChain* inputChain = RootInput::getInstance()->GetChain();
      inputChain->SetBranchStatus ( "SiRes", true );
      inputChain->SetBranchStatus ( "DetectorNumber", true );
      inputChain->SetBranchStatus ( "ChannelNumber", true );
      inputChain->SetBranchStatus ( "Energy", true );
      inputChain->SetBranchStatus ( "EnergyBack", true );
      inputChain->SetBranchStatus ( "Time", true );
      inputChain->SetBranchAddress( "SiRes", &EventPhysics );
   }
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::InitializeRootOutput()
   {
      TTree* outputTree = RootOutput::getInstance()->GetTree()            ;
      outputTree->Branch( "SiRes" , "TSiResPhysics" , &EventPhysics ) ;
   }
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::InitSpectra(){  
   m_Spectra = new TSiResSpectra(NumberOfDetector);
}

///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::FillSpectra(){  
   m_Spectra -> FillRawSpectra(EventData);
   m_Spectra -> FillPreTreatedSpectra(PreTreatedData);
   m_Spectra -> FillPhysicsSpectra(EventPhysics);
}
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::CheckSpectra(){  
  m_Spectra->CheckSpectra();  
}
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::ClearSpectra(){  
  // To be done
}
///////////////////////////////////////////////////////////////////////////
map< vector<string> , TH1*> TSiResPhysics::GetSpectra() {
  if(m_Spectra)
    return m_Spectra->GetMapHisto();
  else{
    map< vector<string> , TH1*> empty;
    return empty;
  }
} 
///////////////////////////////////////////////////////////////////////////

void TSiResPhysics::PreTreat(){  

  //   X
  //   E
  ClearPreTreatedData();
  double E,T;
  E=-1000; T=-1000;
  int N=-1000;

  for(unsigned int i = 0 ; i < EventData->GetEnergyMult() ; ++i)
    {
    	if( EventData->GetEEnergy(i)>m_SiRes_RAW_Threshold )
    	{
		E=CalibrationManager::getInstance()->ApplyCalibration("SiRes/Detector" + itoa( EventData->GetEDetectorNumber(i) ) +"_Channel"+itoa( EventData->GetEChannelNumber(i) )+"_E",EventData->GetEEnergy(i));
    		if(E>m_SiRes_E_Threshold)
    		{
        		N=EventData->GetEDetectorNumber(i);
        		PreTreatedData->SetEDetectorNumber( N );
        		PreTreatedData->SetEChannelNumber( EventData->GetEChannelNumber(i) );
        		PreTreatedData->SetEEnergy( E );
    		}
    	}
    }
  for(unsigned int i = 0 ; i < EventData->GetEEnergyBackMult() ; ++i)
    {
	if( EventData->GetEEnergyBack(i)>m_SiRes_RAWBack_Threshold && EventData->GetEEnergyBackDetectorNumber(i)==N )
	{
		E=CalibrationManager::getInstance()->ApplyCalibration("SiRes/Detector"+itoa(EventData->GetEEnergyBackDetectorNumber(i))+"_EBack",EventData->GetEEnergyBack(i) ) ;  
		if(E>m_SiRes_EBack_Threshold)
		{    
			PreTreatedData->SetEEnergyBackDetectorNumber( EventData->GetEEnergyBackDetectorNumber(i) );
			PreTreatedData->SetEEnergyBack( E );
			if(EventData->GetTimeMult()>0)
			{
				T=CalibrationManager::getInstance()->ApplyCalibration("SiRes/Detector"+itoa(EventData->GetTDetectorNumber(i))+"_T",EventData->GetTTime(i) ) ;      
			}            
			PreTreatedData->SetTTime( T );
		}
	}
    }  

}
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::BuildPhysicalEvent()
   {
      BuildSimplePhysicalEvent()   ;
   }


///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::BuildSimplePhysicalEvent()
   {
	PreTreat();
      for(unsigned int i = 0 ; i < PreTreatedData->GetEnergyMult() ; i++)
         {
            DetectorNumber.push_back( PreTreatedData->GetEDetectorNumber(i) )   ;
            ChannelNumber.push_back( PreTreatedData->GetEChannelNumber(i) )   ;
            Energy.push_back( PreTreatedData->GetEEnergy(i) );
          }
       for(unsigned int i = 0 ; i < EventData->GetTimeMult() ; i++)
         {
            Time.push_back( PreTreatedData->GetTTime(i) );
         }
       for(unsigned int i = 0 ; i < EventData->GetEEnergyBackMult() ; i++)
         {
            EnergyBack.push_back( PreTreatedData->GetEEnergyBack(i) );
         }
     if(PreTreatedData->GetEnergyMult()==4)Treat();

   }
///////////////////////////////////////////////////////////////////////////
void TSiResPhysics::Treat()
   {
      double E1=-1000; double E2=-1000; double E3=-1000; double E4=-1000; 
      for(unsigned int i = 0 ; i < EventData->GetEnergyMult() ; i++)
         {
         	if(ChannelNumber[i]==0)E1=Energy[i];//DH
         	if(ChannelNumber[i]==1)E2=Energy[i];//DB
         	if(ChannelNumber[i]==2)E3=Energy[i];//GB
         	if(ChannelNumber[i]==3)E4=Energy[i];//GH
         }
      	x.push_back( 1+(E1+E2-E3-E4) / (E1+E2+E3+E4) ) ;
      	y.push_back( 1+(E1+E4-E2-E3) / (E1+E2+E3+E4) ) ;
   }
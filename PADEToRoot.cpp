// g++ -Wall -o PADEToRoot.exe `root-config --cflags --glibs` functions.cc WCFunctions.cc dict.o PADEToRoot.cpp

#include "functions.h"
#include "WCFunctions.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

#include "TFile.h"
#include "TTree.h"

bool analyze_spill(std::vector<std::map<int,std::pair<int,int> > >* WCSpillDecodedData, std::vector<int>* data, const bool& verbosity);



int main(int argc, char** argv)
{
  int nFib = 64;
  int nCryst = 9;
  
  
  
  //----------------
  // read parameters
  
  std::string inFileName(argv[1]);
  
  int withCrystals = 1;
  int withWireChambers = 1;
  float table_x = 0.;
  float table_y = 0.;
  if( argc >= 3 ) withCrystals = atoi(argv[2]);
  if( argc >= 4 ) withWireChambers = atoi(argv[3]);
  if( argc >= 5 ) table_x = atof(argv[4]);
  if( argc >= 6 ) table_y = atof(argv[5]);
  
  
  
  //------------------------
  // open infile and outfile
  
  std::ifstream inFile(Form("data/rec_capture_%s.txt",inFileName.c_str()),std::ios::in);
  std::ifstream inFile2(Form("data/rec_capture_%s.txt",inFileName.c_str()),std::ios::in);
  std::ifstream inFileWC(Form("data/WC_capture_%s.txt",inFileName.c_str()),std::ios::in);  
  
  std::string outFileName = std::string("ntuples/tot_capture_") + inFileName.c_str();
  if( withWireChambers ) outFileName += "_WC";
  outFileName += ".root";
  TFile* outFile = TFile::Open(outFileName.c_str(),"RECREATE");
  std::map<int, std::map<int,TTree*> > trees;
  
  
  
  //-----------------
  // define variables
  
  std::map<int, std::map<int,int> > t_entry;
  std::map<int,int> total_entry;
  
  std::map<int,float> t_total_integral_pedSub;
  std::map<int,float> t_total_posIntegral_pedSub;
  std::map<int,float> t_total_maximum_pedSub;
  
  std::map<int,float> t_integral;
  std::map<int,float> t_maximum;
  
  std::map<int,float> t_integral_pedSub;
  std::map<int,float> t_posIntegral_pedSub;
  std::map<int,float> t_maximum_pedSub;
  
  std::map<int,std::vector<float>* > t_waveform;
  std::map<int,std::vector<float>* > t_waveform_pedSub;
  for(int fibIt = 0; fibIt < nFib+nCryst+2; ++fibIt)
  {
    t_waveform[fibIt] = new std::vector<float>;
    t_waveform_pedSub[fibIt] = new std::vector<float>;
  }
  
  
  
  //------------------------
  // read wire-chambers data
  //------------------------
  std::cout << ">>> reading file " << Form("data/WC_capture_%s.txt",inFileName.c_str()) << std::endl;
  
  std::string line;
  std::string line2;
  std::string str;
  
  int spill = 0;
  int nSpills = 0;
  
  std::map<int,std::vector<int>* > WCRawData;
  std::map<int,std::vector<std::map<int,std::pair<int,int> > >* > WCDecodedData;
  
  while(1)
  {
    getline(inFileWC,line,'\n');
    if( !inFileWC.good() ) break;
    
    std::stringstream ss(line);
    
    if( line.at(0) == '*' )
    {
      ss >> str >> str >> str >> str >> spill;
      WCRawData[spill] = new std::vector<int>;
      WCDecodedData[spill] = new std::vector<std::map<int,std::pair<int,int> > >;
      continue;
    }
    
    int token;
    while( ss >> std::hex >> token )
    {
      int val = int(token);
      WCRawData[spill] -> push_back(val);
    }
  }
  
  
  
  //---------------
  // read PADE data
  //---------------
  std::cout << ">>> reading file " << Form("data/rec_capture_%s.txt",inFileName.c_str()) << std::endl;
  
  int dummy;
  long int stamp, stamp2;
  int PADEId, PADEId2;
  int chId, chId2;
  int evtId[2], evtId2[2];
  int waveform[1000];
  
  int currentEvent = -1;
  getline(inFile2,line2,'\n');
    
  std::map<int,int> isGoodSpill;
  
  while(1)
  {
    getline(inFile,line,'\n');
    getline(inFile2,line2,'\n');
    if( !inFile.good() ) break;
    
    
    //----------------
    // read spill line
    if( line.at(0) == '*' )
    {
      // read line
      std::stringstream ss;
      ss << line;
      ss >> str >> str >> str >> str >> spill;
      ++nSpills;
      
      
      // print out spil summary
      if( spill != 1 )
      {
        int nTriggers_16  = (t_entry[spill-1])[16]+1;
        int nTriggers_103 = (t_entry[spill-1])[103]+1;
        int nTriggers_28  = (t_entry[spill-1])[28]+1;
        int nTriggers_64  = (t_entry[spill-1])[64];
        
        if( nTriggers_103 == nTriggers_16 ) isGoodSpill[spill-1] = 1; else isGoodSpill[spill-1] = 0;
        if( (nTriggers_28 != nTriggers_16) && (withCrystals == 1) ) isGoodSpill[spill-1] = 0;
        if( (nTriggers_64 != nTriggers_16) && (withWireChambers == 1) ) isGoodSpill[spill-1] = 0;
        
        std::cout << "spill " << std::fixed << std::setw(3) << spill-1
                  << "  --  " << std::fixed << std::setw(3) << nTriggers_16  << " triggers (PADE16)"
                  << "  --  " << std::fixed << std::setw(3) << nTriggers_103 << " triggers (PADE103)"
                  << "  --  " << std::fixed << std::setw(3) << nTriggers_28  << " triggers (PADE28)"
                  << "  --  " << std::fixed << std::setw(3) << nTriggers_64  << " triggers (WC)"
                  << "  --  good: " << isGoodSpill[spill-1]
                  << std::endl;
      }
      
      
      // define trees and set branch addresses
      (trees[spill])[16]  = new TTree(Form("tree_PADE16_spill%d",spill), Form("tree_PADE16_spill%d",spill));
      (trees[spill])[103] = new TTree(Form("tree_PADE103_spill%d",spill),Form("tree_PADE103_spill%d",spill));
      (trees[spill])[28]  = new TTree(Form("tree_PADE28_spill%d",spill), Form("tree_PADE28_spill%d",spill));
      
      (trees[spill])[16]  -> Branch("entry", &(t_entry[spill])[16], "entry/I");
      (trees[spill])[103] -> Branch("entry", &(t_entry[spill])[103],"entry/I");
      (trees[spill])[28]  -> Branch("entry", &(t_entry[spill])[28], "entry/I");
      
      (trees[spill])[16]  -> Branch("total_integral_pedSub",&t_total_integral_pedSub[16], "total_integral_pedSub/F");
      (trees[spill])[103] -> Branch("total_integral_pedSub",&t_total_integral_pedSub[103],"total_integral_pedSub/F");
      (trees[spill])[28]  -> Branch("total_integral_pedSub",&t_total_integral_pedSub[28], "total_integral_pedSub/F");
      (trees[spill])[16]  -> Branch("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[16], "total_posIntegral_pedSub/F");
      (trees[spill])[103] -> Branch("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[103],"total_posIntegral_pedSub/F");
      (trees[spill])[28]  -> Branch("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[28], "total_posIntegral_pedSub/F");
      (trees[spill])[16]  -> Branch("total_maximum_pedSub", &t_total_maximum_pedSub[16], "total_maximum_pedSub/F");
      (trees[spill])[103] -> Branch("total_maximum_pedSub", &t_total_maximum_pedSub[103],"total_maximum_pedSub/F");
      (trees[spill])[28]  -> Branch("total_maximum_pedSub", &t_total_maximum_pedSub[28], "total_maximum_pedSub/F");
      
      for(int fibIt = 0; fibIt < nFib; ++fibIt)
      {
        int PADEId = 16;
        if( fibIt >= 32 ) PADEId = 103;
        
        (trees[spill])[PADEId] -> Branch(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
        (trees[spill])[PADEId] -> Branch(Form("fib%02d_integral",fibIt),&t_integral[fibIt],Form("fib%02d_integral/F",fibIt));
        (trees[spill])[PADEId] -> Branch(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt],  Form("fib%02d_maximum/F",fibIt));
        
        (trees[spill])[PADEId] -> Branch(Form("fib%02d_waveform_pedSub",fibIt),   &t_waveform_pedSub[fibIt]);
        (trees[spill])[PADEId] -> Branch(Form("fib%02d_integral_pedSub",fibIt),   &t_integral_pedSub[fibIt],      Form("fib%02d_integral_pedSub/F",fibIt));
        (trees[spill])[PADEId] -> Branch(Form("fib%02d_posIntegral_pedSub",fibIt),&t_posIntegral_pedSub[fibIt],Form("fib%02d_posIntegral_pedSub/F",fibIt));
        (trees[spill])[PADEId] -> Branch(Form("fib%02d_maximum_pedSub", fibIt),   &t_maximum_pedSub[fibIt],        Form("fib%02d_maximum_pedSub/F",fibIt));
      }
      
      for(int crystIt = 0; crystIt < nCryst; ++crystIt)
      {
        (trees[spill])[28] -> Branch(Form("cryst%01d_waveform",crystIt),&t_waveform[nFib+crystIt]);
        (trees[spill])[28] -> Branch(Form("cryst%01d_integral",crystIt),&t_integral[nFib+crystIt],Form("cryst%01d_integral/F",crystIt));
        (trees[spill])[28] -> Branch(Form("cryst%01d_maximum", crystIt),&t_maximum[nFib+crystIt],  Form("cryst%01d_maximum/F",crystIt));
        
        (trees[spill])[28] -> Branch(Form("cryst%01d_waveform_pedSub",crystIt),&t_waveform_pedSub[nFib+crystIt]);
        (trees[spill])[28] -> Branch(Form("cryst%01d_integral_pedSub",crystIt),&t_integral_pedSub[nFib+crystIt],Form("cryst%01d_integral_pedSub/F",crystIt));
        (trees[spill])[28] -> Branch(Form("cryst%01d_posIntegral_pedSub",crystIt),&t_posIntegral_pedSub[nFib+crystIt],Form("cryst%01d_posIntegral_pedSub/F",crystIt));
        (trees[spill])[28] -> Branch(Form("cryst%01d_maximum_pedSub", crystIt),&t_maximum_pedSub[nFib+crystIt],  Form("cryst%01d_maximum_pedSub/F",crystIt));
      }
      
      (trees[spill])[28] -> Branch("outCerAna_waveform",&t_waveform[nFib+nCryst]);
      (trees[spill])[28] -> Branch("outCerAna_integral_pedSub",   &t_integral_pedSub[nFib+nCryst]);
      (trees[spill])[28] -> Branch("outCerAna_posIntegral_pedSub",&t_posIntegral_pedSub[nFib+nCryst]);
      (trees[spill])[28] -> Branch("outCerAna_maximum_pedSub",    &t_maximum_pedSub[nFib+nCryst]);
      
      (trees[spill])[28] -> Branch("outCerLog_waveform",&t_waveform[nFib+nCryst+1]);
      (trees[spill])[28] -> Branch("outCerLog_integral_pedSub",   &t_integral_pedSub[nFib+nCryst+1]);
      (trees[spill])[28] -> Branch("outCerLog_posIntegral_pedSub",&t_posIntegral_pedSub[nFib+nCryst+1]);
      (trees[spill])[28] -> Branch("outCerLog_maximum_pedSub",    &t_maximum_pedSub[nFib+nCryst+1]);
      
      // get WC data
      analyze_spill(WCDecodedData[spill],WCRawData[spill],false);
      (t_entry[spill])[64] = WCDecodedData[spill] -> size();
      total_entry[64] += WCDecodedData[spill] -> size();
      
      
      // initialize variables
      currentEvent = -1;
      (t_entry[spill])[16]  = -1;
      (t_entry[spill])[103] = -1;
      (t_entry[spill])[28]  = -1;
    }
    
    
    //-------------------
    // read channels line
    else
    {
      std::stringstream ss;
      ss << line;
      ss >> std::dec >> stamp;
      ss >> std::hex >> dummy >> dummy;
      ss >> std::hex >> PADEId;
      ss >> std::hex >> dummy >> dummy >> dummy;
      ss >> std::hex >> chId;
      ss >> std::hex >> evtId[0] >> std::hex >> evtId[1];
      
      std::stringstream ss2;
      ss2 << line2;
      ss2 >> std::dec >> stamp2;
      ss2 >> std::hex >> dummy >> dummy;
      ss2 >> std::hex >> PADEId2;
      ss2 >> std::hex >> dummy >> dummy >> dummy;
      ss2 >> std::hex >> chId2;
      ss2 >> std::hex >> evtId2[0] >> std::hex >> evtId2[1];
      
      if( evtId[1] != currentEvent )
      {
        t_total_integral_pedSub[PADEId]    = 0.;
        t_total_posIntegral_pedSub[PADEId] = 0.;
        t_total_maximum_pedSub[PADEId]     = 0.;
        
        for(int fibIt = 0; fibIt < nFib+nCryst+2; ++fibIt)
        {
          t_waveform[fibIt] -> clear();
          t_integral[fibIt] = 0.;
          t_maximum[fibIt]  = 0.;
          
          t_waveform_pedSub[fibIt] -> clear();
          t_integral_pedSub[fibIt]    = 0.;
          t_posIntegral_pedSub[fibIt] = 0.;
          t_maximum_pedSub[fibIt]     = 0.;
        }
        
        t_waveform[nFib+nCryst]   -> clear();
        t_waveform[nFib+nCryst+1] -> clear();
        
        ++(t_entry[spill])[PADEId];
        ++total_entry[PADEId];
        currentEvent = evtId[1];
      }
      
      
      int n = 0;
      while(ss)
      {
        ss >> std::hex >> waveform[n];
        ++n;
      }
      
      //std::cout << std::setw(12) << std::dec << stamp    << " "
      //          << std::setw(3) << std::dec << PADEId   << " "
      //          << std::setw(2) << std::dec << chId     << " "
      //          << std::setw(3) << std::dec << evtId[0] << " "
      //          << std::setw(3) << std::dec << evtId[1] << " ";
      ////for(int it = 0; it < n; ++it) std::cout << std::hex << waveform[it] << " ";
      //std::cout << std::endl;
      //std::cout << "t_entry: " << (t_entry[spill])[PADEId] << std::endl;
      
      
      int fibId = -1;
      if( (int(PADEId) == 16) || (int(PADEId) == 103) ) fibId = GetFiberId(chId,int(PADEId));
      
      int cryId = -1;
      if( int(PADEId) == 28 ) cryId = GetCrystalId(chId,int(PADEId));
      
      if( ( (int(PADEId) == 16)  && (fibId != -1) ) ||
          ( (int(PADEId) == 103) && (fibId != -1) ) ||
          ( (int(PADEId) == 28)  && (cryId != -1) ) )
      {
        if( int(PADEId) == 28 ) fibId = nFib+cryId;
        
        float integral = 0.;
        float maximum = -999999.;
        for(int it = 0; it < n-4; ++it)
        {
          float val = float(waveform[it]);
          t_waveform[fibId]->push_back(val);
          
          integral += val;
          if( val > maximum ) maximum = val;
        }
        
        float ped = CalculatePedestal(t_waveform[fibId]);
        
        float integral_pedSub = 0.;
        float posIntegral_pedSub = 0.;
        float maximum_pedSub = -999999.;
        
        for(unsigned int it = 0; it < t_waveform[fibId]->size(); ++it)
        {
          float val = t_waveform[fibId]->at(it) - ped;
          t_waveform_pedSub[fibId]->push_back(val);
          
          integral_pedSub += val;
          if( val > 0. ) posIntegral_pedSub += val;
          if( val > maximum_pedSub ) maximum_pedSub = val;
        }
        
        t_integral[fibId] = integral;
        t_maximum[fibId] = maximum;
        
        t_integral_pedSub[fibId] = integral_pedSub;
        t_posIntegral_pedSub[fibId] = posIntegral_pedSub;
        t_maximum_pedSub[fibId] = maximum_pedSub;
        
        if( (int(PADEId) != 28) ||
            (int(PADEId) == 28 && cryId < 9) )
        {
          t_total_integral_pedSub[int(PADEId)] += integral_pedSub;
          t_total_posIntegral_pedSub[int(PADEId)] += posIntegral_pedSub;
          t_total_maximum_pedSub[int(PADEId)] += maximum_pedSub;
        }
      }
      
      
      //--------------
      // fill the tree
      
      if( (PADEId2 == PADEId && evtId2[1] != evtId[1]) || // next event of same PADE
          (PADEId2 != PADEId && evtId2[1] == 0) ||        // first event of new PADE
          (PADEId2 == PADEId && chId2 == chId) )          // new spill or end of file
      {
        (trees[spill])[int(PADEId)] -> Fill();
        currentEvent = -1;
        
        //std::cout << "***************" << std::endl;
        //std::cout << "fill tree - currentEvent " << currentEvent << "   t_entry:" << (t_entry[spill])[int(PADEId)] << std::endl;
        //std::cout << "***************" << std::endl;      
      }
    }
  }
  
  int nTriggers_16  = (t_entry[spill])[16]+1;
  int nTriggers_103 = (t_entry[spill])[103]+1;
  int nTriggers_28  = (t_entry[spill])[28]+1;
  int nTriggers_64  = (t_entry[spill])[64];
  
  if( nTriggers_103 == nTriggers_16 ) isGoodSpill[spill] = 1; else isGoodSpill[spill] = 0;
  if( (nTriggers_28 != nTriggers_16) && (withCrystals == 1) ) isGoodSpill[spill] = 0;
  if( (nTriggers_64 != nTriggers_16) && (withWireChambers == 1) ) isGoodSpill[spill] = 0;
  
  std::cout << "spill " << std::fixed << std::setw(3) << spill
            << "  --  " << std::fixed << std::setw(3) << nTriggers_16  << " triggers (PADE16)"
            << "  --  " << std::fixed << std::setw(3) << nTriggers_103 << " triggers (PADE103)"
            << "  --  " << std::fixed << std::setw(3) << nTriggers_28  << " triggers (PADE28)"
            << "  --  " << std::fixed << std::setw(3) << nTriggers_64  << " triggers (WC)"
            << "  --  good: " << isGoodSpill[spill]
            << std::endl;
  
  std::cout << std::endl;
  std::cout << total_entry[16]  << " total triggers (PADE16) -- "
            << total_entry[103] << " total triggers (PADE103) -- "
            << total_entry[28]  << " total triggers (PADE28) -- "
            << total_entry[64]  << " total triggers (WC)"
            << std::endl;
  std::cout << std::endl;
  
  
  
  //------------------
  // create final tree
  std::cout << ">>> creating the final tree" << std::endl;
  
  outFile -> cd();
  TTree* tree = new TTree("tree","tree");
  
  int nEntriesMerged = 0;
  tree  -> Branch("spill", &spill,          "spill/I");
  tree  -> Branch("entry", &nEntriesMerged, "entry/I");
  
  tree  -> Branch("total_integral_pedSub",   &t_total_integral_pedSub[16],   "total_integral_pedSub/F");
  tree  -> Branch("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[16],"total_integral_pedSub/F");
  tree  -> Branch("total_maximum_pedSub",    &t_total_maximum_pedSub[16],     "total_maximum_pedSub/F");
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    tree -> Branch(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
    tree -> Branch(Form("fib%02d_integral",fibIt),&t_integral[fibIt],Form("fib%02d_integral/F",fibIt));
    tree -> Branch(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt],  Form("fib%02d_maximum/F",fibIt));
    
    tree -> Branch(Form("fib%02d_waveform_pedSub",fibIt),   &t_waveform_pedSub[fibIt]);
    tree -> Branch(Form("fib%02d_integral_pedSub",fibIt),   &t_integral_pedSub[fibIt],      Form("fib%02d_integral_pedSub/F",fibIt));
    tree -> Branch(Form("fib%02d_posIntegral_pedSub",fibIt),&t_posIntegral_pedSub[fibIt],Form("fib%02d_posIntegral_pedSub/F",fibIt));
    tree -> Branch(Form("fib%02d_maximum_pedSub", fibIt),   &t_maximum_pedSub[fibIt],        Form("fib%02d_maximum_pedSub/F",fibIt));    
  }
  
  tree  -> Branch("total_cryst_integral_pedSub",   &t_total_integral_pedSub[28],      "total_cryst_integral_pedSub/F");
  tree  -> Branch("total_cryst_posIntegral_pedSub",&t_total_posIntegral_pedSub[28],"total_cryst_posIntegral_pedSub/F");
  tree  -> Branch("total_cryst_maximum_pedSub",    &t_total_maximum_pedSub[28],        "total_cryst_maximum_pedSub/F");
  
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    tree -> Branch(Form("cryst%01d_waveform",crystIt),&t_waveform[nFib+crystIt]);
    tree -> Branch(Form("cryst%01d_integral",crystIt),&t_integral[nFib+crystIt],Form("cryst%01d_integral/F",crystIt));
    tree -> Branch(Form("cryst%01d_maximum", crystIt),&t_maximum[nFib+crystIt],  Form("cryst%01d_maximum/F",crystIt));
    
    tree -> Branch(Form("cryst%01d_waveform_pedSub",crystIt),   &t_waveform_pedSub[nFib+crystIt]);
    tree -> Branch(Form("cryst%01d_integral_pedSub",crystIt),   &t_integral_pedSub[nFib+crystIt],      Form("cryst%01d_integral_pedSub/F",crystIt));
    tree -> Branch(Form("cryst%01d_posIntegral_pedSub",crystIt),&t_posIntegral_pedSub[nFib+crystIt],Form("cryst%01d_posIntegral_pedSub/F",crystIt));
    tree -> Branch(Form("cryst%01d_maximum_pedSub", crystIt),   &t_maximum_pedSub[nFib+crystIt],        Form("cryst%01d_maximum_pedSub/F",crystIt));
  }
  
  tree -> Branch("outCerAna_waveform",&t_waveform[nFib+nCryst]);
  tree -> Branch("outCerAna_integral_pedSub",   &t_integral_pedSub[nFib+nCryst]);
  tree -> Branch("outCerAna_posIntegral_pedSub",&t_posIntegral_pedSub[nFib+nCryst]);
  tree -> Branch("outCerAna_maximum_pedSub",    &t_maximum_pedSub[nFib+nCryst]);
  
  tree -> Branch("outCerLog_waveform",&t_waveform[nFib+nCryst+1]);
  tree -> Branch("outCerLog_integral_pedSub",   &t_integral_pedSub[nFib+nCryst+1]);
  tree -> Branch("outCerLog_posIntegral_pedSub",&t_posIntegral_pedSub[nFib+nCryst+1]);
  tree -> Branch("outCerLog_maximum_pedSub",    &t_maximum_pedSub[nFib+nCryst+1]);  
  
  float WC1_x = 9999.; float WC1_y = 9999.;
  float WC2_x = 9999.; float WC2_y = 9999.;
  float WC3_x = 9999.; float WC3_y = 9999.;
  float WC4_x = 9999.; float WC4_y = 9999.;
  tree -> Branch("table_x",&table_x,"table_x/F");
  tree -> Branch("table_y",&table_y,"table_y/F");
  tree -> Branch("WC1_x",&WC1_x,"WC1_x/F");
  tree -> Branch("WC1_y",&WC1_y,"WC1_y/F");
  tree -> Branch("WC2_x",&WC2_x,"WC2_x/F");
  tree -> Branch("WC2_y",&WC2_y,"WC2_x/F");
  tree -> Branch("WC3_x",&WC3_x,"WC3_x/F");
  tree -> Branch("WC3_y",&WC3_y,"WC3_y/F");
  tree -> Branch("WC4_x",&WC4_x,"WC4_x/F");
  tree -> Branch("WC4_y",&WC4_y,"WC4_y/F");  
  
  
  //----------------
  // merge the trees
  std::cout << ">>> merging the three PADEs" << std::endl;
  
  for(spill = 1; spill <= nSpills; ++spill)
  {
    if( isGoodSpill[spill] == false ) continue;
    
    (trees[spill])[16]  -> SetBranchAddress("entry", &t_entry[16]);
    (trees[spill])[103] -> SetBranchAddress("entry", &t_entry[103]);
    (trees[spill])[28]  -> SetBranchAddress("entry", &t_entry[28]);
    
    (trees[spill])[16]  -> SetBranchAddress("total_integral_pedSub",&t_total_integral_pedSub[16]);
    (trees[spill])[103] -> SetBranchAddress("total_integral_pedSub",&t_total_integral_pedSub[103]);
    (trees[spill])[28]  -> SetBranchAddress("total_integral_pedSub",&t_total_integral_pedSub[28]);
    (trees[spill])[16]  -> SetBranchAddress("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[16]);
    (trees[spill])[103] -> SetBranchAddress("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[103]);
    (trees[spill])[28]  -> SetBranchAddress("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[28]);
    (trees[spill])[16]  -> SetBranchAddress("total_maximum_pedSub", &t_total_maximum_pedSub[16]);
    (trees[spill])[103] -> SetBranchAddress("total_maximum_pedSub", &t_total_maximum_pedSub[103]);
    (trees[spill])[28]  -> SetBranchAddress("total_maximum_pedSub", &t_total_maximum_pedSub[28]);
    
    for(int fibIt = 0; fibIt < nFib; ++fibIt)
    {
      int PADEId = 16;
      if( fibIt >= 32 ) PADEId = 103;
      
      (trees[spill])[PADEId] -> SetBranchAddress(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
      (trees[spill])[PADEId] -> SetBranchAddress(Form("fib%02d_integral",fibIt),&t_integral[fibIt]);
      (trees[spill])[PADEId] -> SetBranchAddress(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt]);
      
      (trees[spill])[PADEId] -> SetBranchAddress(Form("fib%02d_waveform_pedSub",fibIt),   &t_waveform_pedSub[fibIt]);
      (trees[spill])[PADEId] -> SetBranchAddress(Form("fib%02d_integral_pedSub",fibIt),   &t_integral_pedSub[fibIt]);
      (trees[spill])[PADEId] -> SetBranchAddress(Form("fib%02d_posIntegral_pedSub",fibIt),&t_posIntegral_pedSub[fibIt]);
      (trees[spill])[PADEId] -> SetBranchAddress(Form("fib%02d_maximum_pedSub", fibIt),   &t_maximum_pedSub[fibIt]);
    }
    
    for(int crystIt = 0; crystIt < nCryst; ++crystIt)
    {
      (trees[spill])[28] -> SetBranchAddress(Form("cryst%01d_waveform",crystIt),&t_waveform[nFib+crystIt]);
      (trees[spill])[28] -> SetBranchAddress(Form("cryst%01d_integral",crystIt),&t_integral[nFib+crystIt]);
      (trees[spill])[28] -> SetBranchAddress(Form("cryst%01d_maximum", crystIt),&t_maximum[nFib+crystIt]);
      
      (trees[spill])[28] -> SetBranchAddress(Form("cryst%01d_waveform_pedSub",crystIt),&t_waveform_pedSub[nFib+crystIt]);
      (trees[spill])[28] -> SetBranchAddress(Form("cryst%01d_integral_pedSub",crystIt),&t_integral_pedSub[nFib+crystIt]);
      (trees[spill])[28] -> SetBranchAddress(Form("cryst%01d_posIntegral_pedSub",crystIt),&t_posIntegral_pedSub[nFib+crystIt]);
      (trees[spill])[28] -> SetBranchAddress(Form("cryst%01d_maximum_pedSub", crystIt),&t_maximum_pedSub[nFib+crystIt]);
    }
    
    (trees[spill])[28] -> SetBranchAddress("outCerAna_waveform",&t_waveform[nFib+nCryst]);
    (trees[spill])[28] -> SetBranchAddress("outCerAna_integral_pedSub",   &t_integral_pedSub[nFib+nCryst]);
    (trees[spill])[28] -> SetBranchAddress("outCerAna_posIntegral_pedSub",&t_posIntegral_pedSub[nFib+nCryst]);
    (trees[spill])[28] -> SetBranchAddress("outCerAna_maximum_pedSub",    &t_maximum_pedSub[nFib+nCryst]);
    
    (trees[spill])[28] -> SetBranchAddress("outCerLog_waveform",&t_waveform[nFib+nCryst+1]);
    (trees[spill])[28] -> SetBranchAddress("outCerLog_integral_pedSub",   &t_integral_pedSub[nFib+nCryst+1]);
    (trees[spill])[28] -> SetBranchAddress("outCerLog_posIntegral_pedSub",&t_posIntegral_pedSub[nFib+nCryst+1]);
    (trees[spill])[28] -> SetBranchAddress("outCerLog_maximum_pedSub",    &t_maximum_pedSub[nFib+nCryst+1]);
    
    for(int entry = 0; entry < (trees[spill])[16]->GetEntriesFast(); ++entry)
    {
      std::cout << "merging entry " << nEntriesMerged << " / " << total_entry[16] << "\r" << std::flush;
      
      if( (trees[spill])[16]->GetEntriesFast() != (trees[spill])[103]->GetEntriesFast() )
        std::cout << "!!! ERROR: PADE16 and PADE103 do not match" << std::endl;
      if( (withCrystals) && ( (trees[spill])[16]->GetEntriesFast() != (trees[spill])[28]->GetEntriesFast() ) )
        std::cout << "!!! ERROR: PADE28 do not match with PADE16/103" << std::endl;
      if( (withWireChambers) && ( (trees[spill])[16]->GetEntriesFast() != int(WCDecodedData[spill]->size()) ) )
        std::cout << "!!! ERROR: WC do not match with PADE16/103" << std::endl;
      
      (trees[spill])[16]  -> GetEntry(entry);
      (trees[spill])[103] -> GetEntry(entry);
      if( withCrystals ) (trees[spill])[28]  -> GetEntry(entry);
      
      t_total_integral_pedSub[16]    += t_total_integral_pedSub[103];
      t_total_posIntegral_pedSub[16] += t_total_posIntegral_pedSub[103];
      t_total_maximum_pedSub[16]     += t_total_maximum_pedSub[103];
      
      if( withWireChambers )
      {
        std::map<int,std::pair<int,int> > WCCoords = WCDecodedData[spill]->at(entry);
        WC1_x = WCCoords[1].first - 63.5; WC1_y = WCCoords[1].second - 63.5;
        WC2_x = WCCoords[2].first - 63.5; WC2_y = WCCoords[2].second - 63.5;
        WC3_x = WCCoords[3].first - 63.5; WC3_y = WCCoords[3].second - 63.5;
        WC4_x = WCCoords[4].first - 63.5; WC4_y = WCCoords[4].second - 63.5;
        //std::cout << "WC1 - x: " << WCCoords[1].first << " y: " << WCCoords[1].second << std::endl;
        //std::cout << "WC2 - x: " << WCCoords[2].first << " y: " << WCCoords[2].second << std::endl;
        //std::cout << "WC3 - x: " << WCCoords[3].first << " y: " << WCCoords[3].second << std::endl;
        //std::cout << "WC4 - x: " << WCCoords[4].first << " y: " << WCCoords[4].second << std::endl;
      }
      
      tree -> Fill();
      ++nEntriesMerged;
    }
  }
  std::cout << std::endl;
  std::cout << ">>> merged " << nEntriesMerged << " events" << std::endl;
  
  tree -> Write();
  outFile -> Close();
}



bool analyze_spill(std::vector<std::map<int,std::pair<int,int> > >* WCSpillDecodedData, std::vector<int>* data, const bool& verbosity)
{
  int counterOld = 0;
  int counterNew = 0;
  
  controllerHeader myControllerHeader;
  std::map<int,TDCSpillHeader> myTDCSpillHeader;
  std::map<int,TDCEventHeader> myTDCEventHeader;
  
  
  counterNew = analyze_controllerHeader(myControllerHeader,data,counterOld,verbosity);
  if( counterNew == counterOld )
  {
    std::cout << ">>> !!! error in controller header <<<" << std::endl;
    return false;
  }
  else
  {
    counterOld = counterNew;
  }
  
  
  counterNew = analyze_TDCSpillHeader(myTDCSpillHeader,data,myControllerHeader,counterOld,verbosity);
  if( counterNew == counterOld )
  {
    std::cout << ">>> !!! error in TDC spill header <<<" << std::endl;
    return false;
  }
  else
  {
    counterOld = counterNew;
  }
  
  
  for(int iTrigger = 1; iTrigger <= myControllerHeader.triggerCount; ++iTrigger)
  {
    counterNew = analyze_TDCEventHeader(myTDCEventHeader,myTDCSpillHeader,data,iTrigger,counterOld,verbosity);
    if( counterNew == counterOld )
    {
      std::cout << ">>> !!! error in TDC event header for trigger " << iTrigger << " <<<" << std::endl;
      
      counterOld = seekNextTrigger(data,iTrigger+1,counterNew,verbosity);
      std::pair<int,int> dummyVal(9999,9999);
      std::map<int,std::pair<int,int> > dummyMap;
      dummyMap[1] = dummyVal;
      dummyMap[2] = dummyVal;
      dummyMap[3] = dummyVal;
      dummyMap[4] = dummyVal;
      WCSpillDecodedData -> push_back(dummyMap);
      
      if( verbosity )
      {
        std::cout << "counter: " << counterNew << std::endl;
        std::cout << "corrupted-bits begin " << std::endl;
        for(int it = counterNew; it < counterOld; ++it)
        {
          std::cout << data->at(it) << " ";
          if( it == counterOld-1 ) std::cout << std::endl;
        }
        std::cout << "corrupted-bits end " << std::endl;
      }
    }
    else
    {
      std::map<int,std::pair<int,int> > dummyMap = writeData(myControllerHeader,myTDCEventHeader);
      WCSpillDecodedData -> push_back(dummyMap);
      counterOld = counterNew;
      
      //std::cout << "WC1 - x: " << dummyMap[1].first << " y: " << dummyMap[1].second << std::endl;
      //std::cout << "WC2 - x: " << dummyMap[2].first << " y: " << dummyMap[2].second << std::endl;
      //std::cout << "WC3 - x: " << dummyMap[3].first << " y: " << dummyMap[3].second << std::endl;
      //std::cout << "WC4 - x: " << dummyMap[4].first << " y: " << dummyMap[4].second << std::endl;
    }
  }
  
  if( verbosity )
  {
    std::cout << ">>> !!! end of WC file <<<" << std::endl;
    
    std::cout << "counter: " << counterNew << std::endl;
    std::cout << "extra-bits begin " << std::endl;
    for(unsigned int it = counterNew; it < data->size(); ++it)
    {
      std::cout << data->at(it) << " ";
      if( it == data->size()-1 ) std::cout << std::endl;
    }
    std::cout << "extra-bits end " << std::endl;
  }
  
  return true;
}

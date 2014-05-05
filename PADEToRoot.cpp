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
  int nCh = 32;
  int nFib = 64;
  int nCryst = 9;
  
  
  
  //----------------
  // read parameters
  
  std::string inFileName(argv[1]);
  
  int triggerCountCheckBaF = 1;
  int triggerCountCheckWC = 1;
  if( argc >= 3 ) triggerCountCheckBaF = atoi(argv[2]);
  if( argc >= 4 ) triggerCountCheckWC  = atoi(argv[3]);
  
  
  
  //------------
  // open infile
  
  std::ifstream inFile(Form("data/rec_capture_%s.txt",inFileName.c_str()),std::ios::in);
  std::ifstream inFileWC(Form("data/WC_capture_%s.txt",inFileName.c_str()),std::ios::in);  
  
  
  
  //-----------------------------
  // define outfile and out trees
  
  TFile* outFile = TFile::Open(Form("ntuples/tot_capture_%s.root",inFileName.c_str()),"RECREATE");
  
  std::map<int,TTree*> trees;
  trees[16]  = new TTree("tree_PADE16", "tree_PADE16");
  trees[103] = new TTree("tree_PADE103","tree_PADE103");
  trees[28]  = new TTree("tree_PADE28", "tree_PADE28");
  
  
  
  //-----------------------------
  // define and set tree branches
  
  std::map<int,int> t_spill;
  std::map<int,int> t_entry;
  
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
  
  trees[16]  -> Branch("spill", &t_spill[16], "spill/I");
  trees[103] -> Branch("spill", &t_spill[103],"spill/I");
  trees[28]  -> Branch("spill", &t_spill[28], "spill/I");
  trees[16]  -> Branch("entry", &t_entry[16], "entry/I");
  trees[103] -> Branch("entry", &t_entry[103],"entry/I");
  trees[28]  -> Branch("entry", &t_entry[28], "entry/I");
  
  trees[16]  -> Branch("total_integral_pedSub",&t_total_integral_pedSub[16], "total_integral_pedSub/F");
  trees[103] -> Branch("total_integral_pedSub",&t_total_integral_pedSub[103],"total_integral_pedSub/F");
  trees[28]  -> Branch("total_integral_pedSub",&t_total_integral_pedSub[28], "total_integral_pedSub/F");
  trees[16]  -> Branch("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[16], "total_posIntegral_pedSub/F");
  trees[103] -> Branch("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[103],"total_posIntegral_pedSub/F");
  trees[28]  -> Branch("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[28], "total_posIntegral_pedSub/F");
  trees[16]  -> Branch("total_maximum_pedSub", &t_total_maximum_pedSub[16], "total_maximum_pedSub/F");
  trees[103] -> Branch("total_maximum_pedSub", &t_total_maximum_pedSub[103],"total_maximum_pedSub/F");
  trees[28]  -> Branch("total_maximum_pedSub", &t_total_maximum_pedSub[28], "total_maximum_pedSub/F");
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    int PADEId = 16;
    if( fibIt >= 32 ) PADEId = 103;
    
    trees[PADEId] -> Branch(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
    trees[PADEId] -> Branch(Form("fib%02d_integral",fibIt),&t_integral[fibIt],Form("fib%02d_integral/F",fibIt));
    trees[PADEId] -> Branch(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt],  Form("fib%02d_maximum/F",fibIt));
    
    trees[PADEId] -> Branch(Form("fib%02d_waveform_pedSub",fibIt),   &t_waveform_pedSub[fibIt]);
    trees[PADEId] -> Branch(Form("fib%02d_integral_pedSub",fibIt),   &t_integral_pedSub[fibIt],      Form("fib%02d_integral_pedSub/F",fibIt));
    trees[PADEId] -> Branch(Form("fib%02d_posIntegral_pedSub",fibIt),&t_posIntegral_pedSub[fibIt],Form("fib%02d_posIntegral_pedSub/F",fibIt));
    trees[PADEId] -> Branch(Form("fib%02d_maximum_pedSub", fibIt),   &t_maximum_pedSub[fibIt],        Form("fib%02d_maximum_pedSub/F",fibIt));
  }
  
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    trees[28] -> Branch(Form("cryst%01d_waveform",crystIt),&t_waveform[nFib+crystIt]);
    trees[28] -> Branch(Form("cryst%01d_integral",crystIt),&t_integral[nFib+crystIt],Form("cryst%01d_integral/F",crystIt));
    trees[28] -> Branch(Form("cryst%01d_maximum", crystIt),&t_maximum[nFib+crystIt],  Form("cryst%01d_maximum/F",crystIt));
  }
  trees[28] -> Branch("outCerAna_waveform",&t_waveform[nFib+nCryst]);
  trees[28] -> Branch("outCerLog_waveform",&t_waveform[nFib+nCryst+1]);
  
  
  
  //-------------------
  // wire-chambers data
  std::cout << ">>> reading file " << Form("data/WC_capture_%s.txt",inFileName.c_str()) << std::endl;
  
  std::string line;  
  std::string str;
  
  int spill = 0;
  
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
  
  
  //----------
  // PADE data
  std::cout << ">>> reading file " << Form("data/rec_capture_%s.txt",inFileName.c_str()) << std::endl;
  
  long int stamp;
  int dummy;
  int PADEId;
  int chId;
  int evtId[2];
  int waveform[1000];
  
  int nChRead;
  
  std::map<int,int> currentEntry;
  bool isFirstSpill = true;
  std::map<int,int> isGoodSpill;
  
  while(1)
  {
    getline(inFile,line,'\n');
    if( !inFile.good() ) break;
    
    
    //----------------
    // read spill line
    
    if( line.at(0) == '*' )
    {
      if( !isFirstSpill )
      {
        int nTriggers_16  = t_entry[16]  - currentEntry[16];
        int nTriggers_103 = t_entry[103] - currentEntry[103];
        int nTriggers_28  = t_entry[28]  - currentEntry[28];
        int nTriggers_64  = t_entry[64]  - currentEntry[64];
        
        if( nTriggers_103 == nTriggers_16 ) isGoodSpill[spill] = 1; else isGoodSpill[spill] = 0;
        if( (nTriggers_28 != nTriggers_16) && (triggerCountCheckBaF == 1) ) isGoodSpill[spill] = 0;
        if( (nTriggers_64 != nTriggers_16) && (triggerCountCheckWC  == 1) ) isGoodSpill[spill] = 0;
        
        std::cout << "spill " << std::fixed << std::setw(3) << spill
                  << "  --  " << std::fixed << std::setw(3) << nTriggers_16  << " triggers (PADE16)"
                  << "  --  " << std::fixed << std::setw(3) << nTriggers_103 << " triggers (PADE103)"
                  << "  --  " << std::fixed << std::setw(3) << nTriggers_28  << " triggers (PADE28)"
                  << "  --  " << std::fixed << std::setw(3) << nTriggers_64  << " triggers (WC)"
                  << "  --  good: " << isGoodSpill[spill]
                  << std::endl;
        
        currentEntry[16]  = t_entry[16];
        currentEntry[103] = t_entry[103];
        currentEntry[28]  = t_entry[28];
        currentEntry[64]  = t_entry[64];
      }
      
      std::stringstream ss;
      ss << line;
      
      ss >> str >> str >> str >> str >> spill;
      //std::cout << "spill " << spill << std::endl;
      
      nChRead = 0;
      t_total_integral_pedSub[16] = 0.;
      t_total_integral_pedSub[103] = 0.;
      t_total_integral_pedSub[28] = 0.;
      t_total_posIntegral_pedSub[16] = 0.;
      t_total_posIntegral_pedSub[103] = 0.;
      t_total_posIntegral_pedSub[28] = 0.;
      t_total_maximum_pedSub[16] = 0.;
      t_total_maximum_pedSub[103] = 0.;
      t_total_maximum_pedSub[28] = 0.;
      
      analyze_spill(WCDecodedData[spill],WCRawData[spill],false);      
      t_entry[64] += WCDecodedData[spill] -> size();
      
      isFirstSpill = false;
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
      
      int n = 0;
      while(ss)
      {
        ss >> std::hex >> waveform[n];
        ++n;
      }
      
      //std::cout << std::dec << stamp    << " "
      //          << std::dec << PADEId   << " "
      //          << std::dec << chId     << " "
      //          << std::dec << evtId[0] << " "
      //          << std::dec << evtId[1] << " ";
      ////for(int it = 0; it < n; ++it) std::cout << std::hex << waveform[it] << " ";
      //std::cout << std::endl;
      
      ++nChRead;
      
      
      int fibId = -1;
      if( (int(PADEId) == 16) || (int(PADEId) == 103) ) fibId = GetFiberId(chId,int(PADEId));
      if( int(PADEId) == 28 )                           fibId = nFib + GetCrystalId(chId,int(PADEId));
      
      if( (int(PADEId) == 28) && (chId == 11) ) fibId = nFib + nCryst;
      if( (int(PADEId) == 28) && (chId ==  4) ) fibId = nFib + nCryst + 1;
      
      if( fibId != -1 )
      {
        t_waveform[fibId] -> clear();
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
        
        t_waveform_pedSub[fibId] -> clear();
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
                
        t_total_integral_pedSub[int(PADEId)] += integral_pedSub;
        t_total_posIntegral_pedSub[int(PADEId)] += posIntegral_pedSub;
        t_total_maximum_pedSub[int(PADEId)] += maximum_pedSub;
      }
    }
    
    //--------------
    // fill the tree
    
    if( nChRead == nCh )
    {
      t_spill[int(PADEId)] = spill;
      ++t_entry[int(PADEId)];
      
      trees[int(PADEId)] -> Fill();
      
      nChRead = 0;
    }
  }
  
  
  int nTriggers_16  = t_entry[16]  - currentEntry[16];
  int nTriggers_103 = t_entry[103] - currentEntry[103];
  int nTriggers_28  = t_entry[28]  - currentEntry[28];
  int nTriggers_64  = t_entry[64]  - currentEntry[64];
  
  if( nTriggers_103 == nTriggers_16 ) isGoodSpill[spill] = 1; else isGoodSpill[spill] = 0;
  if( (nTriggers_28 != nTriggers_16) && (triggerCountCheckBaF == 1) ) isGoodSpill[spill] = 0;
  if( (nTriggers_64 != nTriggers_16) && (triggerCountCheckWC  == 1) ) isGoodSpill[spill] = 0;
  
  std::cout << "spill " << std::fixed << std::setw(3) << spill
            << "  --  " << std::fixed << std::setw(3) << nTriggers_16  << " triggers (PADE16)"
            << "  --  " << std::fixed << std::setw(3) << nTriggers_103 << " triggers (PADE103)"
            << "  --  " << std::fixed << std::setw(3) << nTriggers_28  << " triggers (PADE28)"
            << "  --  " << std::fixed << std::setw(3) << nTriggers_64  << " triggers (WC)"
            << "  --  good: " << isGoodSpill[spill]
            << std::endl;
  
  std::cout << t_entry[16]  << " total triggers (PADE16) -- "
            << t_entry[103] << " total triggers (PADE103) -- "
            << t_entry[28]  << " total triggers (PADE28) -- "
            << t_entry[64]  << " total triggers (WC)"
            << std::endl;
  
  
  
  //--------------------
  // merge the two trees
  std::cout << ">>> merging the three PADEs" << std::endl;
  
  trees[16]  -> SetBranchAddress("spill", &t_spill[16]);
  trees[103] -> SetBranchAddress("spill", &t_spill[103]);
  trees[28]  -> SetBranchAddress("spill", &t_spill[28]);
  trees[16]  -> SetBranchAddress("entry", &t_entry[16]);
  trees[103] -> SetBranchAddress("entry", &t_entry[103]);
  trees[28]  -> SetBranchAddress("entry", &t_entry[28]);
  
  trees[16]  -> SetBranchAddress("total_integral_pedSub",&t_total_integral_pedSub[16]);
  trees[103] -> SetBranchAddress("total_integral_pedSub",&t_total_integral_pedSub[103]);
  trees[28]  -> SetBranchAddress("total_integral_pedSub",&t_total_integral_pedSub[28]);
  trees[16]  -> SetBranchAddress("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[16]);
  trees[103] -> SetBranchAddress("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[103]);
  trees[28]  -> SetBranchAddress("total_posIntegral_pedSub",&t_total_posIntegral_pedSub[28]);
  trees[16]  -> SetBranchAddress("total_maximum_pedSub", &t_total_maximum_pedSub[16]);
  trees[103] -> SetBranchAddress("total_maximum_pedSub", &t_total_maximum_pedSub[103]);
  trees[28]  -> SetBranchAddress("total_maximum_pedSub", &t_total_maximum_pedSub[28]);
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    if( fibIt < 32)
    {
      trees[16] -> SetBranchAddress(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
      trees[16] -> SetBranchAddress(Form("fib%02d_integral",fibIt),&t_integral[fibIt]);
      trees[16] -> SetBranchAddress(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt]);
      
      trees[16] -> SetBranchAddress(Form("fib%02d_waveform_pedSub",fibIt),   &t_waveform_pedSub[fibIt]);
      trees[16] -> SetBranchAddress(Form("fib%02d_integral_pedSub",fibIt),   &t_integral_pedSub[fibIt]);
      trees[16] -> SetBranchAddress(Form("fib%02d_posIntegral_pedSub",fibIt),&t_posIntegral_pedSub[fibIt]);
      trees[16] -> SetBranchAddress(Form("fib%02d_maximum_pedSub", fibIt),   &t_maximum_pedSub[fibIt]);
    }
    else
    {
      trees[103] -> SetBranchAddress(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
      trees[103] -> SetBranchAddress(Form("fib%02d_integral",fibIt),&t_integral[fibIt]);
      trees[103] -> SetBranchAddress(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt]);
      
      trees[103] -> SetBranchAddress(Form("fib%02d_waveform_pedSub",fibIt),   &t_waveform_pedSub[fibIt]);
      trees[103] -> SetBranchAddress(Form("fib%02d_integral_pedSub",fibIt),   &t_integral_pedSub[fibIt]);
      trees[103] -> SetBranchAddress(Form("fib%02d_posIntegral_pedSub",fibIt),&t_posIntegral_pedSub[fibIt]);
      trees[103] -> SetBranchAddress(Form("fib%02d_maximum_pedSub", fibIt),   &t_maximum_pedSub[fibIt]);
    }
  }
  
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    trees[28] -> SetBranchAddress(Form("cryst%01d_waveform",crystIt),&t_waveform[nFib+crystIt]);
    trees[28] -> SetBranchAddress(Form("cryst%01d_integral",crystIt),&t_integral[nFib+crystIt]);
    trees[28] -> SetBranchAddress(Form("cryst%01d_maximum", crystIt),&t_maximum[nFib+crystIt]);
  }
  trees[28] -> SetBranchAddress("outCerAna_waveform",&t_waveform[nFib+nCryst]);
  trees[28] -> SetBranchAddress("outCerLog_waveform",&t_waveform[nFib+nCryst+1]);
  
  
  outFile -> cd();
  TTree* tree = new TTree("tree","tree");
  
  tree  -> Branch("spill", &t_spill[16], "spill/I");
  tree  -> Branch("entry", &t_entry[16], "entry/I");
  
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
  
  tree  -> Branch("total_cryst_integral",&t_total_integral_pedSub[28],"total_cryst_integral/F");
  tree  -> Branch("total_cryst_maximum", &t_total_maximum_pedSub[28],  "total_cryst_maximum/F");
  
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    tree -> Branch(Form("cryst%01d_waveform",crystIt),&t_waveform[nFib+crystIt]);
    tree -> Branch(Form("cryst%01d_integral",crystIt),&t_integral[nFib+crystIt],Form("cryst%01d_integral/F",crystIt));
    tree -> Branch(Form("cryst%01d_maximum", crystIt),&t_maximum[nFib+crystIt],  Form("cryst%01d_maximum/F",crystIt));
  }
  tree -> Branch("outCerAna_waveform",&t_waveform[nFib+nCryst]);
  tree -> Branch("outCerLog_waveform",&t_waveform[nFib+nCryst+1]);
  
  int WC1_x,WC1_y;
  int WC2_x,WC2_y;
  int WC3_x,WC3_y;
  int WC4_x,WC4_y;
  tree -> Branch("WC1_x",&WC1_x,"WC1_x/I");
  tree -> Branch("WC1_y",&WC1_y,"WC1_y/I");
  tree -> Branch("WC2_x",&WC2_x,"WC2_x/I");
  tree -> Branch("WC2_y",&WC2_y,"WC2_x/I");
  tree -> Branch("WC3_x",&WC3_x,"WC3_x/I");
  tree -> Branch("WC3_y",&WC3_y,"WC3_y/I");
  tree -> Branch("WC4_x",&WC4_x,"WC4_x/I");
  tree -> Branch("WC4_y",&WC4_y,"WC4_y/I");  
  
  
  
  int nEntriesMerged = 0;
  int nEntriesMergedWC = 0;
  spill = -1;
  for(int entry = 0; entry < trees[16]->GetEntries(); ++entry)
  {
    std::cout << "merging entry " << entry << " / " << trees[16]->GetEntries() << "\r" << std::flush;
    
    trees[16]  -> GetEntry(entry);
    trees[103] -> GetEntry(entry);
    trees[28]  -> GetEntry(entry);
    
    if( t_spill[16] != spill )
    {
      spill = t_spill[16];
      nEntriesMergedWC = 0;
    }
    
    if( isGoodSpill[spill] == false ) continue;
    
    t_total_integral_pedSub[16]    += t_total_integral_pedSub[103];
    t_total_posIntegral_pedSub[16] += t_total_posIntegral_pedSub[103];
    t_total_maximum_pedSub[16]     += t_total_maximum_pedSub[103];
    
    std::map<int,std::pair<int,int> > WCCoords = WCDecodedData[spill]->at(nEntriesMergedWC);
    WC1_x = WCCoords[1].first; WC1_y = WCCoords[1].second;
    WC2_x = WCCoords[2].first; WC2_y = WCCoords[2].second;
    WC3_x = WCCoords[3].first; WC3_y = WCCoords[3].second;
    WC4_x = WCCoords[4].first; WC4_y = WCCoords[4].second;
    //std::cout << "WC1 - x: " << WCCoords[1].first << " y: " << WCCoords[1].second << std::endl;
    //std::cout << "WC2 - x: " << WCCoords[2].first << " y: " << WCCoords[2].second << std::endl;
    //std::cout << "WC3 - x: " << WCCoords[3].first << " y: " << WCCoords[3].second << std::endl;
    //std::cout << "WC4 - x: " << WCCoords[4].first << " y: " << WCCoords[4].second << std::endl;
    
    tree -> Fill();
    ++nEntriesMerged;
    ++nEntriesMergedWC;
  }
  std::cout << std::endl;
  std::cout << "merged " << nEntriesMerged << " events" << std::endl;
  
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
    counterNew = analyze_TDCEventHeader(myTDCEventHeader,data,iTrigger,counterOld,verbosity);
    if( counterNew == counterOld )
    {
      std::cout << ">>> !!! error in TDC event header <<<" << std::endl;
      
      return false;
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
    std::cout << "counter: " << counterNew << std::endl;
    std::cout << "extra-bits begin " << std::endl;
    for(int it = counterNew; it < data->size(); ++it)
    std::cout << data->at(it) << " ";
    std::cout << std::endl;
    std::cout << "extra-bits end " << std::endl;
  }
  
  return true;
}

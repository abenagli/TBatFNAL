// g++ -Wall -o PADEToRoot `root-config --cflags --glibs` functions.cc PADEToRoot.cpp

#include "functions.h"

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



int main(int argc, char** argv)
{
  int nCh = 32;
  int nFib = 64;
  int nCryst = 9;
  
  
  
  //----------------
  // read parameters
  
  std::string inFileName(argv[1]);
  
  int triggerCountCheck = 1;
  if( argc >= 3 ) triggerCountCheck = atoi(argv[2]);
  
  
  
  //------------
  // open infile
  
  std::ifstream inFile(Form("data/rec_capture_%s.txt",inFileName.c_str()),std::ios::in);
  
  
  
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
  
  std::map<int,float> t_total_integral;
  std::map<int,float> t_total_maximum;
  
  std::map<int,float> t_integral;
  std::map<int,float> t_maximum;

  std::map<int,std::vector<int>* > t_waveform;
  for(int fibIt = 0; fibIt < nFib+nCryst+2; ++fibIt)
  {
    t_waveform[fibIt] = new std::vector<int>;
  }
  
  trees[16]  -> Branch("spill", &t_spill[16], "spill/I");
  trees[103] -> Branch("spill", &t_spill[103],"spill/I");
  trees[28]  -> Branch("spill", &t_spill[28], "spill/I");
  trees[16]  -> Branch("entry", &t_entry[16], "entry/I");
  trees[103] -> Branch("entry", &t_entry[103],"entry/I");
  trees[28]  -> Branch("entry", &t_entry[28], "entry/I");
  
  trees[16]  -> Branch("total_integral",&t_total_integral[16], "total_integral/F");
  trees[103] -> Branch("total_integral",&t_total_integral[103],"total_integral/F");
  trees[28]  -> Branch("total_integral",&t_total_integral[28], "total_integral/F");
  trees[16]  -> Branch("total_maximum", &t_total_maximum[16],   "total_maximum/F");
  trees[103] -> Branch("total_maximum", &t_total_maximum[103],  "total_maximum/F");
  trees[28]  -> Branch("total_maximum", &t_total_maximum[28],   "total_maximum/F");
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    int PADEId = 16;
    if( fibIt >= 32 ) PADEId = 103;
    
    trees[PADEId] -> Branch(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
    trees[PADEId] -> Branch(Form("fib%02d_integral",fibIt),&t_integral[fibIt],Form("fib%02d_integral/F",fibIt));
    trees[PADEId] -> Branch(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt],  Form("fib%02d_maximum/F",fibIt));
  }
  
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    trees[28] -> Branch(Form("cryst%01d_waveform",crystIt),&t_waveform[nFib+crystIt]);
    trees[28] -> Branch(Form("cryst%01d_integral",crystIt),&t_integral[nFib+crystIt],Form("cryst%01d_integral/F",crystIt));
    trees[28] -> Branch(Form("cryst%01d_maximum", crystIt),&t_maximum[nFib+crystIt],  Form("cryst%01d_maximum/F",crystIt));
  }
  trees[28] -> Branch("outCerAna_waveform",&t_waveform[nFib+nCryst]);
  trees[28] -> Branch("outCerLog_waveform",&t_waveform[nFib+nCryst+1]);
  
  
  
  //--------------
  // read the file
  std::cout << ">>> reading file " << Form("data/rec_capture_%s.txt",inFileName.c_str()) << std::endl;

  std::string line;  
  std::string str;
  
  int spill;
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
        
        if( (nTriggers_16 == nTriggers_103) && (nTriggers_16 == nTriggers_28) ) isGoodSpill[spill] = 1;
        else                                                                    isGoodSpill[spill] = 0;
        
        std::cout << "spill " << std::fixed << std::setw(3) << spill
                  << "  --  " << std::fixed << std::setw(4) << nTriggers_16  << " triggers (PADE16)"
                  << "  --  " << std::fixed << std::setw(4) << nTriggers_103 << " triggers (PADE103)"
                  << "  --  " << std::fixed << std::setw(4) << nTriggers_28  << " triggers (PADE28)"
                  << "  --  good: " << isGoodSpill[spill]
                  << std::endl;
        
        currentEntry[16]  = t_entry[16];
        currentEntry[103] = t_entry[103];
        currentEntry[28]  = t_entry[28];
      }
      
      std::stringstream ss;
      ss << line;
      
      ss >> str >> str >> str >> str >> spill;
      //std::cout << "spill " << spill << std::endl;
      
      nChRead = 0;
      t_total_integral[16] = 0.;
      t_total_integral[103] = 0.;
      t_total_integral[28] = 0.;
      t_total_maximum[16] = 0.;
      t_total_maximum[103] = 0.;
      t_total_maximum[28] = 0.;
      
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
        for(int it = 0; it < n-4; ++it) t_waveform[fibId]->push_back(int(waveform[it]));
        
        float ped = CalculatePedestal(t_waveform[fibId]);
        float integral = 0.;
        float maximum = -999999.;
        for(unsigned int it = 0; it < t_waveform[fibId]->size(); ++it)
        {
          float val = t_waveform[fibId]->at(it) - ped;;
          integral += val;
          if( val > maximum ) maximum = val;
        }
        
        t_integral[fibId] = integral;
        t_maximum[fibId] = maximum;
        
        t_total_integral[int(PADEId)] += integral;
        t_total_maximum[int(PADEId)] += maximum;      
        
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
  
  if( (nTriggers_16 == nTriggers_103) && (nTriggers_16 == nTriggers_28) ) isGoodSpill[spill] = 1;
  else                                                                    isGoodSpill[spill] = 0;  
  
  std::cout << "spill " << std::fixed << std::setw(3) << spill
            << "  --  " << std::fixed << std::setw(4) << nTriggers_16  << " triggers (PADE16)"
            << "  --  " << std::fixed << std::setw(4) << nTriggers_103 << " triggers (PADE103)"
            << "  --  " << std::fixed << std::setw(4) << nTriggers_28  << " triggers (PADE28)"
            << "  --  good: " << isGoodSpill[spill]
            << std::endl;
  
  std::cout << t_entry[16]  << " total triggers (PADE16) -- "
            << t_entry[103] << " total triggers (PADE103) -- "
            << t_entry[28]  << " total triggers (PADE28)"
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
  
  trees[16]  -> SetBranchAddress("total_integral",&t_total_integral[16]);
  trees[103] -> SetBranchAddress("total_integral",&t_total_integral[103]);
  trees[28]  -> SetBranchAddress("total_integral",&t_total_integral[28]);
  trees[16]  -> SetBranchAddress("total_maximum", &t_total_maximum[16]);
  trees[103] -> SetBranchAddress("total_maximum", &t_total_maximum[103]);
  trees[28]  -> SetBranchAddress("total_maximum", &t_total_maximum[28]);
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    if( fibIt < 32)
    {
      trees[16] -> SetBranchAddress(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
      trees[16] -> SetBranchAddress(Form("fib%02d_integral",fibIt),&t_integral[fibIt]);
      trees[16] -> SetBranchAddress(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt]);
    }
    else
    {
      trees[103] -> Branch(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
      trees[103] -> Branch(Form("fib%02d_integral",fibIt),&t_integral[fibIt]);
      trees[103] -> Branch(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt]);
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
  
  tree  -> Branch("total_integral",&t_total_integral[16],"total_integral/F");
  tree  -> Branch("total_maximum", &t_total_maximum[16],  "total_maximum/F");
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    tree -> Branch(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
    tree -> Branch(Form("fib%02d_integral",fibIt),&t_integral[fibIt],Form("fib%02d_integral/F",fibIt));
    tree -> Branch(Form("fib%02d_maximum", fibIt),&t_maximum[fibIt],  Form("fib%02d_maximum/F",fibIt));
  }
  
  tree  -> Branch("total_cryst_integral",&t_total_integral[28],"total_cryst_integral/F");
  tree  -> Branch("total_cryst_maximum", &t_total_maximum[28],  "total_cryst_maximum/F");
  
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    tree -> Branch(Form("cryst%01d_waveform",crystIt),&t_waveform[nFib+crystIt]);
    tree -> Branch(Form("cryst%01d_integral",crystIt),&t_integral[nFib+crystIt],Form("cryst%01d_integral/F",crystIt));
    tree -> Branch(Form("cryst%01d_maximum", crystIt),&t_maximum[nFib+crystIt],  Form("cryst%01d_maximum/F",crystIt));
  }
  tree -> Branch("outCerAna_waveform",&t_waveform[nFib+nCryst]);
  tree -> Branch("outCerLog_waveform",&t_waveform[nFib+nCryst+1]);
  
  
  int nEntriesMerged = 0;
  for(int entry = 0; entry < trees[16]->GetEntries(); ++entry)
  {
    std::cout << "merging entry " << entry << " / " << trees[16]->GetEntries() << "\r" << std::flush;
    
    trees[16]  -> GetEntry(entry);
    trees[103] -> GetEntry(entry);
    trees[28]  -> GetEntry(entry);
    
    t_total_integral[16] += t_total_integral[103];
    t_total_maximum[16] += t_total_maximum[103];
    
    if( (isGoodSpill[t_spill[16]] == false) && (triggerCountCheck == 1) ) continue;
    
    tree -> Fill();
    ++nEntriesMerged;
  }
  std::cout << std::endl;
  std::cout << "merged " << nEntriesMerged << " events" << std::endl;
  
  tree -> Write();
  outFile -> Close();
}

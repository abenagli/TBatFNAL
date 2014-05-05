// g++ -Wall -o WCDecoder.exe `root-config --cflags --glibs` WCFunctions.cc WCDecoder.cpp

#include "WCFunctions.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "TFile.h"
#include "TTree.h"



bool analyze_spill(std::ofstream& outFile, std::vector<int>* data, const bool& verbosity);




int main(int argc, char** argv)
{
  //------------
  // read infile
  
  std::string inFileName(argv[1]);  
  std::ifstream inFile(Form("data/WC_capture_%s.txt",inFileName.c_str()),std::ios::binary);
  std::ofstream outFile(Form("data/WC_capture_%s_decoded.txt",inFileName.c_str()),std::ios::out);
  
  
  //--------------
  // read the file
  std::cout << ">>> reading file " << Form("data/WC_capture_%s.txt",inFileName.c_str()) << std::endl;

  std::string line;  
  std::string str;
  
  int spill = 0;
  
  
  std::map<int,std::vector<int>* > spills;
  
  while(1)
  {
    getline(inFile,line,'\n');
    if( !inFile.good() ) break;
    
    if( line.find("spill") != std::string::npos )
    {
      std::cout << line << std::endl;
      ++spill;
      spills[spill] = new std::vector<int>;
      continue;
    }
    
    //if( spill != 1 ) break;
    
    
    //----------------
    // read spill line
    
    std::stringstream ss(line);
    
    int token;
    while( ss >> std::hex >> token )
    {
      int val = int(token);
      spills[spill] -> push_back(val);
    }
  }
  
  
  
  std::cout << std::endl;
  bool verbosity = false;
  
  for(std::map<int,std::vector<int>*>::const_iterator mapIt = spills.begin(); mapIt != spills.end(); ++mapIt)
  {
    int spill = mapIt -> first;
    std::vector<int>* data = mapIt -> second;
    
    if( verbosity )
    {
      std::cout << "\nspill: " << spill << "    size: " << data->size() << std::endl;
    }
    if( data->size() > 20+12*16 ) analyze_spill(outFile,data,verbosity);
  }
}



bool analyze_spill(std::ofstream& outFile, std::vector<int>* data, const bool& verbosity)
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
      writeData(outFile,myControllerHeader,myTDCEventHeader);
      counterOld = counterNew;
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

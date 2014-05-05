#ifndef WCFunctions_h 
#define WCFunctions_h

#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <fstream>



struct controllerHeader
{
  int wordCount;
  int spillCount;
  int year;
  int month;
  int day;
  int hour;
  int minutes;
  int seconds;
  int triggerCount;
  int status;
  int linkStatus;
};

struct TDCSpillHeader
{
  int wordCount;
  int TDCNum;
  int triggerCount;
  int status;
};

struct TDCEventHeader
{
  int wordCount;
  int TDCNum;
  int status;
  int triggerCount;
  int triggerType;
  int controllerTimeStamp;
  int TDCTimeStamp;
  
  std::vector<std::pair<int,int> > eventData;
};


int analyze_controllerHeader(controllerHeader& dummy,
                              std::vector<int>* data, const int& iStart, const bool& verbosity);
int analyze_TDCSpillHeader(std::map<int,TDCSpillHeader>& dummyMap,
                            std::vector<int>* data, controllerHeader& myControllerHeader, const int& iStart, const bool& verbosity);
int analyze_TDCEventHeader(std::map<int,TDCEventHeader>& dummyMap, std::vector<int>* data, const int& iTrigger, const int& iStart, const bool& verbosity);

int fillControllerHeader(controllerHeader& dummy, std::vector<int>* data, const int& iStart);
int fillTDCSpillHeader(TDCSpillHeader& dummy, std::vector<int>* data, const int& iStart);
int fillTDCEventHeader(TDCEventHeader& dummy, std::vector<int>* data, const int& iStart);

bool tryTDCEventHeaderRemoval(std::vector<int>* data, const int& iStart);

void writeRawData(std::ofstream& outFile, controllerHeader& myControllerHeader, std::map<int,TDCEventHeader>& myTDCEventHeader);
std::map<int,std::pair<int,int> > writeData(controllerHeader& myControllerHeader, std::map<int,TDCEventHeader>& myTDCEventHeader);

#endif

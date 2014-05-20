#include "WCFunctions.h"






int analyze_controllerHeader(controllerHeader& dummy,
                              std::vector<int>* data, const int& iStart, const bool& verbosity)
{
  bool isHeaderGood = false;
  
  while( !isHeaderGood )
  {
    int counter = fillControllerHeader(dummy,data,iStart);
    if( counter == iStart ) return iStart;
    
    if( (dummy.year == 14) &&
        (dummy.month == 3) )
    {
      isHeaderGood = true;
      
      if( verbosity )
      {
        std::cout << ">>> controllerHeader <<<" << std::endl;
        for(int it = iStart; it < counter; ++it)
          std::cout << std::setw(3) << data->at(it) << " - ";
        std::cout << std::endl;
      }
      
      return counter;
    }
    else
    {
      data->erase(data->begin());
    }
  }
  
  return iStart;
}



int analyze_TDCSpillHeader(std::map<int,TDCSpillHeader>& dummyMap,
                            std::vector<int>* data, controllerHeader& myControllerHeader, const int& iStart, const bool& verbosity)
{
  if( verbosity )
    std::cout << ">>> TDCSpillHeader <<<" << std::endl;
  
  int counter = iStart;
  int iStart2 = iStart;
  for(int TDCIt = 0; TDCIt < 16; ++TDCIt)
  {
    TDCSpillHeader dummy;
    counter = fillTDCSpillHeader(dummy,data,iStart2);
    if( counter == iStart2 ) return iStart;
    
    if( verbosity )
    {
      for(int it = iStart2; it < counter; ++it)
        std::cout << std::setw(3) << data->at(it) << " - ";
       std::cout << std::endl;
    }
    
    dummyMap[TDCIt] = dummy;
    iStart2 = counter;
  }
  
  bool isHeaderGood = true;
  for(int TDCIt = 0; TDCIt < 16; ++TDCIt)
  {
    if( dummyMap[TDCIt].TDCNum != TDCIt+1 ) isHeaderGood *= false;
    if( dummyMap[TDCIt].triggerCount != myControllerHeader.triggerCount ) isHeaderGood *= false;
  }
  
  if( isHeaderGood ) return counter;
  else               return iStart;
}



int analyze_TDCEventHeader(std::map<int,TDCEventHeader>& dummyMap,
                           std::map<int,TDCSpillHeader>& dummyMap2, std::vector<int>* data, const int& iTrigger, const int& iStart, const bool& verbosity)
{
  if( verbosity )
    std::cout << ">>> TDCEventHeader: iTrigger: " << iTrigger << " <<<" << std::endl;
  
  dummyMap.clear();
  
  bool isHeaderGood = false;
  int counter = iStart;
  int iStart2 = iStart;
  int currentTDCNum = 1;
  while( !isHeaderGood )
  {
    TDCEventHeader dummy;
    counter = fillTDCEventHeader(dummy,data,iStart2);
    if( counter == iStart2 )
    {
      if( verbosity )
      {
        std::cout << ">>>>>> error: not enough bits to fill TDC event header" << std::endl;
      }
      return iStart;
    }
    
    if( dummy.triggerCount != iTrigger )
    {
      tryTDCEventHeaderRemoval(data,iStart2);
      counter = fillTDCEventHeader(dummy,data,iStart2);
      if( dummy.triggerCount != iTrigger )
      {
        bool tryTDC4Skip = false;
        
        for(int jj = 0; jj < 100; ++jj)
        {
          TDCEventHeader dummyNext;
          int counterNew = fillTDCEventHeader(dummyNext,data,iStart2+jj);
          if( counterNew == (iStart2+jj) )
          { 
            std::cout << ">>>>>> error: trigger count mismatch (iTrigger " << iTrigger << " vs triggerCount " << dummy.triggerCount
                      << ") for TDC " << dummy.TDCNum << std::endl;
            return iStart;
          }
          if( dummyNext.TDCNum == currentTDCNum+1 && dummyNext.triggerCount == iTrigger )
          {
            tryTDC4Skip = true;
            iStart2 = iStart2+jj;
            counter = fillTDCEventHeader(dummy,data,iStart2);
            break;
          }
        }
        
        if( tryTDC4Skip == false )
        {
          if( verbosity )
          {
            std::cout << ">>>>>> error: trigger count mismatch (iTrigger " << iTrigger << " vs triggerCount " << dummy.triggerCount
                      << ") for TDC " << dummy.TDCNum << std::endl;
          }
          return iStart;
        }
      }
    }
    
    if( verbosity )
    {
      for(int it = iStart2; it < counter; ++it)
        std::cout << std::setw(3) << data->at(it) << " - ";
       std::cout << std::endl;
    }
    
    
    iStart2 = counter;
    
    for(int dataIt = 0; dataIt < dummy.wordCount-9; ++dataIt)
    {
      if( iStart2+1 >= int(data->size()) ) return counter;
      
      counter += 2;
      
      std::pair<int,int> dummyData(data->at(iStart2),data->at(iStart2+1));
      dummy.eventData.push_back(dummyData);
      
      if( verbosity )
      {
        std::cout << ">>>>>> ";
        for(int it = iStart2; it < counter; ++it)
          std::cout << std::setw(3) << data->at(it) << " - ";
         std::cout << std::endl;
      }
      
      iStart2 += 2;
    }
    
    
    dummyMap[dummy.TDCNum] = dummy;
    
    TDCEventHeader dummyNext;
    int counterNext = fillTDCEventHeader(dummyNext,data,counter);
    if( (dummy.TDCNum == 16) ||
        (dummyNext.TDCNum >= 1 && dummyNext.TDCNum <= 16 && dummyNext.triggerCount == iTrigger+1) ) isHeaderGood = true;
    
    //if( (iTrigger == (dummyMap2[dummy.TDCNum]).triggerCount) &&
    //    (dummyNext.TDCNum >= 1 && dummyNext.TDCNum <= 16 && dummyNext.triggerCount != iTrigger+1) ) isHeaderGood = true;
    
    if( counterNext == counter )
      isHeaderGood = true;
    
    ++currentTDCNum;
  }
  
  return counter;
}


int seekNextTrigger(std::vector<int>* data, const int& iTrigger, const int& iStart, const bool& verbosity)
{
  int iStart2 = iStart;
  
  while( iStart2 < int(data->size())-18 )
  {
    TDCEventHeader dummyNext;
    fillTDCEventHeader(dummyNext,data,iStart2);
    if( dummyNext.TDCNum == 1 && dummyNext.triggerCount == iTrigger )
    {
      break;
    }
    else
    {
      ++iStart2;
    }
  }
  
  return iStart2;
}

void writeRawData(std::ofstream& outFile, controllerHeader& myControllerHeader, std::map<int,TDCEventHeader>& myTDCEventHeader)
{
  for(std::map<int,TDCEventHeader>::const_iterator mapIt = myTDCEventHeader.begin(); mapIt != myTDCEventHeader.end(); ++mapIt)
  {
    for(unsigned int eventIt = 0; eventIt < (mapIt->second).eventData.size(); ++eventIt)
    {
      outFile << (mapIt->second).TDCTimeStamp << " "
              << myControllerHeader.spillCount << " "
              << (mapIt->second).triggerCount << " "
              << (mapIt->second).TDCNum << " "
              << (mapIt->second).eventData.at(eventIt).first/4 << " "
              << (mapIt->second).eventData.at(eventIt).second
              << std::endl;
    }
  }
}

std::map<int,std::pair<int,int> > writeData(controllerHeader& myControllerHeader, std::map<int,TDCEventHeader>& myTDCEventHeader)
{
  std::map<int,std::pair<int,int> > dummyMapCh;
  dummyMapCh[1] = std::pair<int,int>(9999,9999);
  dummyMapCh[2] = std::pair<int,int>(9999,9999);
  dummyMapCh[3] = std::pair<int,int>(9999,9999);
  dummyMapCh[4] = std::pair<int,int>(9999,9999);
  
  std::map<int,std::pair<int,int> > dummyMapTime;
  dummyMapTime[1] = std::pair<int,int>(999999,999999);
  dummyMapTime[2] = std::pair<int,int>(999999,999999);
  dummyMapTime[3] = std::pair<int,int>(999999,999999);
  dummyMapTime[4] = std::pair<int,int>(999999,999999);
    
  for(std::map<int,TDCEventHeader>::const_iterator mapIt = myTDCEventHeader.begin(); mapIt != myTDCEventHeader.end(); ++mapIt)
  {
    int x = -1;
    int y = -1;
    int TDCNum = (mapIt->second).TDCNum;
    int WCNum = (TDCNum-1)/4 + 1;
    
    for(unsigned int eventIt = 0; eventIt < (mapIt->second).eventData.size(); ++eventIt)
    {
      int TDCCh = (mapIt->second).eventData.at(eventIt).first/4;
      int TDCTime = (mapIt->second).eventData.at(eventIt).second;
      bool isX = false;
      bool isY = false;
      
      if( TDCNum ==  1 || TDCNum ==  2) { x = (TDCNum- 1)*64 + TDCCh; isX = true; } ;
      if( TDCNum ==  3 || TDCNum ==  4) { y = (TDCNum- 3)*64 + TDCCh; isY = true; } ;
      if( TDCNum ==  5 || TDCNum ==  6) { x = (TDCNum- 5)*64 + TDCCh; isX = true; } ;
      if( TDCNum ==  7 || TDCNum ==  8) { y = (TDCNum- 7)*64 + TDCCh; isY = true; } ;
      if( TDCNum ==  9 || TDCNum == 10) { x = (TDCNum- 9)*64 + TDCCh; isX = true; } ;
      if( TDCNum == 11 || TDCNum == 12) { y = (TDCNum-11)*64 + TDCCh; isY = true; } ;
      if( TDCNum == 13 || TDCNum == 14) { x = (TDCNum-13)*64 + TDCCh; isX = true; } ;
      if( TDCNum == 15 || TDCNum == 16) { y = (TDCNum-15)*64 + TDCCh; isY = true; } ;
      
      if( (isX) && (TDCTime < dummyMapTime[WCNum].first) )
      {
        dummyMapCh[WCNum].first = x;
        dummyMapTime[WCNum].first = TDCTime;
        //std::cout << "x - WCNum: " << WCNum << "   TDCNum: " << TDCNum << "   TDCTime: " << TDCTime << "   x: " << x << std::endl;
      }
      if( (isY) && (TDCTime < dummyMapTime[WCNum].second) )
      {
        dummyMapCh[WCNum].second = y;
        dummyMapTime[WCNum].second = TDCTime;
        //std::cout << "y - WCNum: " << WCNum << "   TDCNum: " << TDCNum << "   TDCTime: " << TDCTime << "   y: " << y << std::endl;
      }
    }
  }
  
  //std::cout << "WC1 - x: " << dummyMapCh[1].first << " y: " << dummyMapCh[1].second << std::endl;
  //std::cout << "WC2 - x: " << dummyMapCh[2].first << " y: " << dummyMapCh[2].second << std::endl;
  //std::cout << "WC3 - x: " << dummyMapCh[3].first << " y: " << dummyMapCh[3].second << std::endl;
  //std::cout << "WC4 - x: " << dummyMapCh[4].first << " y: " << dummyMapCh[4].second << std::endl;
  
  return dummyMapCh;
}



int fillControllerHeader(controllerHeader& dummy, std::vector<int>* data, const int& iStart)
{
  if( data->size()-iStart < 20 ) return iStart;
  
  dummy.wordCount = 256*256*256*data->at(iStart+0) +
                        256*256*data->at(iStart+1) +
                            256*data->at(iStart+2) +
                                data->at(iStart+3);
  
  dummy.spillCount = 256*data->at(iStart+4) +
                         data->at(iStart+5);
  
  dummy.year    = data->at(iStart+6);
  dummy.month   = data->at(iStart+7);
  dummy.day     = data->at(iStart+8);
  dummy.hour    = data->at(iStart+9);
  dummy.minutes = data->at(iStart+10);
  dummy.seconds = data->at(iStart+11);
  
  dummy.triggerCount = 256*256*256*data->at(iStart+12) +
                           256*256*data->at(iStart+13) +
                               256*data->at(iStart+14) +
                                   data->at(iStart+15);
  
  dummy.status = 256*data->at(iStart+16) +
                     data->at(iStart+17);
  
  dummy.linkStatus = 256*data->at(iStart+18) +
                         data->at(iStart+19);
  
  return iStart + 20;
}



int fillTDCSpillHeader(TDCSpillHeader& dummy, std::vector<int>* data, const int& iStart)
{
  if( data->size()-iStart < 12 ) return iStart;
  
  dummy.wordCount = 256*256*256*data->at(iStart+0) +
                        256*256*data->at(iStart+1) +
                            256*data->at(iStart+2) +
                                data->at(iStart+3);
  
  dummy.TDCNum = data->at(iStart+5);
  
  dummy.triggerCount = 256*256*256*data->at(iStart+6) +
                           256*256*data->at(iStart+7) +
                               256*data->at(iStart+8) +
                                   data->at(iStart+9);
  
  dummy.status = data->at(iStart+11);
  
  return iStart + 12;
}



int fillTDCEventHeader(TDCEventHeader& dummy, std::vector<int>* data, const int& iStart)
{
  if( data->size()-iStart < 18 ) return iStart;
  
  dummy.wordCount = data->at(iStart+1);
  
  dummy.TDCNum = data->at(iStart+3);
  
  dummy.status = data->at(iStart+5);
  
  dummy.triggerCount = 256*256*256*data->at(iStart+6) +
                           256*256*data->at(iStart+7) +
                               256*data->at(iStart+8) +
                                   data->at(iStart+9);
  
  dummy.triggerType = data->at(iStart+11);
  
  dummy.controllerTimeStamp = data->at(iStart+13);
  
  dummy.TDCTimeStamp = 256*256*256*data->at(iStart+14) +
                           256*256*data->at(iStart+15) +
                               256*data->at(iStart+16) +
                                   data->at(iStart+17);
  
  dummy.eventData.clear();
  
  return iStart + 18;
}



bool tryTDCEventHeaderRemoval(std::vector<int>* data, const int& iStart)
{
  for(std::vector<int>::iterator it = data->begin(); it != data->end()-1; ++it)
  {
    if( (*it) == 13 && (*(it+1)) == 10 )
    {
      data->erase(it);
      data->erase(it);
      return true;
    }
  }
  return false;
}

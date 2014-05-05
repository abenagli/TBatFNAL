#include "functions.h"

float fSample = 0.075; // GHz
int nCh = 32;
int nFib = 64;
int nCryst = 9;

float min = -50.;
float max = 200.;



void AddWaveform(TGraph* g, std::vector<float>* v, bool pedSub)
{
  float ped = CalculatePedestal(v);
  if( !pedSub ) ped = 0;
  
  if( g->GetN() == 0 )
  {
    for(unsigned int it = 0; it < v->size(); ++it)
    {
      g -> SetPoint(it,it/fSample,v->at(it)-ped);
    }
  }
  
  else
  {
    double x,y;
    for(unsigned int it = 0; it < v->size(); ++it)
    {
      g -> GetPoint(it,x,y);
      g -> SetPoint(it,x,y+v->at(it)-ped);
    }
  }
}



float CalculatePedestal(std::vector<float>* v)
{
  int nPoints = 5;
  float val = 0.;
  for(int it = 0; it < nPoints; ++it)
  {
    val += v->at(it);
  }
  
  return val/nPoints;
}



void CalculateAmplitude(std::vector<float>* v, float& ped, float& integral, float& maximum)
{
  ped = CalculatePedestal(v);
  
  integral = 0.;
  maximum = -999999.;
  for(unsigned int it = 0; it < v->size(); ++it)
  {
    float val = v->at(it) - ped;
    integral += val;
    if( val > maximum )
    {
      maximum = val;
    }
  }
}



void NormalizeGraph(TGraph* g, const float& norm)
{
  double x,y;
  for(int it = 0; it < g->GetN(); ++it)
  {
    g -> GetPoint(it,x,y);
    g -> SetPoint(it,x,y/norm);
  }
}



float GetMinimum(TGraph* g)
{
  int min = 999999;
  double x,y;
  for(int point = 0; point < g->GetN(); ++point)
  {
    g -> GetPoint(point,x,y);
    if( y < min ) min = y;
  }
  return min;
}



float GetMaximum(TGraph* g)
{
  int max = -999999;
  double x,y;
  for(int point = 0; point < g->GetN(); ++point)
  {
    g -> GetPoint(point,x,y);
    if( y > max ) max = y;
  }
  return max;
}



int GetFiberId(const int& chId, const int& PADEId)
{
  if( chId == 15 && PADEId == 16 ) return 0;
  if( chId == 13 && PADEId == 16 ) return 1;
  if( chId == 10 && PADEId == 16 ) return 2;
  if( chId ==  8 && PADEId == 16 ) return 3;
  if( chId ==  7 && PADEId == 16 ) return 4;
  if( chId ==  5 && PADEId == 16 ) return 5;
  if( chId ==  2 && PADEId == 16 ) return 6;
  if( chId ==  0 && PADEId == 16 ) return 7;
  if( chId ==  1 && PADEId == 16 ) return 8;
  if( chId ==  3 && PADEId == 16 ) return 9;
  if( chId ==  4 && PADEId == 16 ) return 10;
  if( chId ==  6 && PADEId == 16 ) return 11;
  if( chId ==  9 && PADEId == 16 ) return 12;
  if( chId == 11 && PADEId == 16 ) return 13;
  if( chId == 12 && PADEId == 16 ) return 14;
  if( chId == 14 && PADEId == 16 ) return 15;
  if( chId == 17 && PADEId == 16 ) return 16;
  if( chId == 19 && PADEId == 16 ) return 17;
  if( chId == 20 && PADEId == 16 ) return 18;
  if( chId == 22 && PADEId == 16 ) return 19;
  if( chId == 25 && PADEId == 16 ) return 20;
  if( chId == 27 && PADEId == 16 ) return 21;
  if( chId == 28 && PADEId == 16 ) return 22;
  if( chId == 30 && PADEId == 16 ) return 23;
  if( chId == 31 && PADEId == 16 ) return 24;
  if( chId == 29 && PADEId == 16 ) return 25;
  if( chId == 26 && PADEId == 16 ) return 26;
  if( chId == 24 && PADEId == 16 ) return 27;
  if( chId == 23 && PADEId == 16 ) return 28;
  if( chId == 21 && PADEId == 16 ) return 29;
  if( chId == 18 && PADEId == 16 ) return 30;
  if( chId == 16 && PADEId == 16 ) return 31;
  
  if( chId ==  0 && PADEId == 103 ) return 32;
  if( chId ==  2 && PADEId == 103 ) return 33;
  if( chId ==  5 && PADEId == 103 ) return 34;
  if( chId ==  7 && PADEId == 103 ) return 35;
  if( chId ==  8 && PADEId == 103 ) return 36;
  if( chId == 10 && PADEId == 103 ) return 37;
  if( chId == 13 && PADEId == 103 ) return 38;
  if( chId == 15 && PADEId == 103 ) return 39;
  if( chId == 14 && PADEId == 103 ) return 40;
  if( chId == 12 && PADEId == 103 ) return 41;
  if( chId == 11 && PADEId == 103 ) return 42;
  if( chId ==  9 && PADEId == 103 ) return 43;
  if( chId ==  6 && PADEId == 103 ) return 44;
  if( chId ==  4 && PADEId == 103 ) return 45;
  if( chId ==  3 && PADEId == 103 ) return 46;
  if( chId ==  1 && PADEId == 103 ) return 47;
  if( chId == 30 && PADEId == 103 ) return 48;
  if( chId == 28 && PADEId == 103 ) return 49;
  if( chId == 27 && PADEId == 103 ) return 50;
  if( chId == 25 && PADEId == 103 ) return 51;
  if( chId == 22 && PADEId == 103 ) return 52;
  if( chId == 20 && PADEId == 103 ) return 53;
  if( chId == 19 && PADEId == 103 ) return 54;
  if( chId == 17 && PADEId == 103 ) return 55;
  if( chId == 16 && PADEId == 103 ) return 56;
  if( chId == 18 && PADEId == 103 ) return 57;
  if( chId == 21 && PADEId == 103 ) return 58;
  if( chId == 23 && PADEId == 103 ) return 59;
  if( chId == 24 && PADEId == 103 ) return 60;
  if( chId == 26 && PADEId == 103 ) return 61;
  if( chId == 29 && PADEId == 103 ) return 62;
  if( chId == 31 && PADEId == 103 ) return 63;
  
  return -1;
}



int GetCrystalId(const int& chId, const int& PADEId)
{
  if( chId ==  7 && PADEId == 28 ) return 0;
  if( chId ==  0 && PADEId == 28 ) return 1;
  if( chId ==  3 && PADEId == 28 ) return 2;
  if( chId ==  9 && PADEId == 28 ) return 3;
  if( chId ==  2 && PADEId == 28 ) return 4;
  if( chId == 14 && PADEId == 28 ) return 5;
  if( chId == 12 && PADEId == 28 ) return 6;
  if( chId ==  6 && PADEId == 28 ) return 7;
  if( chId ==  1 && PADEId == 28 ) return 8;
  
  return -1;
}



int GetChId(const int& id, const int& PADEId)
{
  if( id ==  0 && PADEId == 16 ) return 15;
  if( id ==  1 && PADEId == 16 ) return 13;
  if( id ==  2 && PADEId == 16 ) return 10;
  if( id ==  3 && PADEId == 16 ) return  8;
  if( id ==  4 && PADEId == 16 ) return  7;
  if( id ==  5 && PADEId == 16 ) return  5;
  if( id ==  6 && PADEId == 16 ) return  2;
  if( id ==  7 && PADEId == 16 ) return  0;
  if( id ==  8 && PADEId == 16 ) return  1;
  if( id ==  9 && PADEId == 16 ) return  3;
  if( id == 10 && PADEId == 16 ) return  4;
  if( id == 11 && PADEId == 16 ) return  6;
  if( id == 12 && PADEId == 16 ) return  9;
  if( id == 13 && PADEId == 16 ) return 11;
  if( id == 14 && PADEId == 16 ) return 12;
  if( id == 15 && PADEId == 16 ) return 14;
  if( id == 16 && PADEId == 16 ) return 17;
  if( id == 17 && PADEId == 16 ) return 19;
  if( id == 18 && PADEId == 16 ) return 20;
  if( id == 19 && PADEId == 16 ) return 22;
  if( id == 20 && PADEId == 16 ) return 25;
  if( id == 21 && PADEId == 16 ) return 27;
  if( id == 22 && PADEId == 16 ) return 28;
  if( id == 23 && PADEId == 16 ) return 30;
  if( id == 24 && PADEId == 16 ) return 31;
  if( id == 25 && PADEId == 16 ) return 29;
  if( id == 26 && PADEId == 16 ) return 26;
  if( id == 27 && PADEId == 16 ) return 24;
  if( id == 28 && PADEId == 16 ) return 23;
  if( id == 29 && PADEId == 16 ) return 21;
  if( id == 30 && PADEId == 16 ) return 18;
  if( id == 31 && PADEId == 16 ) return 16;
  
  if( id == 32 && PADEId == 103 ) return  0;
  if( id == 33 && PADEId == 103 ) return  2;
  if( id == 34 && PADEId == 103 ) return  5;
  if( id == 35 && PADEId == 103 ) return  7;
  if( id == 36 && PADEId == 103 ) return  8;
  if( id == 37 && PADEId == 103 ) return 10;
  if( id == 38 && PADEId == 103 ) return 13;
  if( id == 39 && PADEId == 103 ) return 15;
  if( id == 40 && PADEId == 103 ) return 14;
  if( id == 41 && PADEId == 103 ) return 12;
  if( id == 42 && PADEId == 103 ) return 11;
  if( id == 43 && PADEId == 103 ) return  9;
  if( id == 44 && PADEId == 103 ) return  6;
  if( id == 45 && PADEId == 103 ) return  4;
  if( id == 46 && PADEId == 103 ) return  3;
  if( id == 47 && PADEId == 103 ) return  1;
  if( id == 48 && PADEId == 103 ) return 30;
  if( id == 49 && PADEId == 103 ) return 28;
  if( id == 50 && PADEId == 103 ) return 27;
  if( id == 51 && PADEId == 103 ) return 25;
  if( id == 52 && PADEId == 103 ) return 22;
  if( id == 53 && PADEId == 103 ) return 20;
  if( id == 54 && PADEId == 103 ) return 19;
  if( id == 55 && PADEId == 103 ) return 17;
  if( id == 56 && PADEId == 103 ) return 16;
  if( id == 57 && PADEId == 103 ) return 18;
  if( id == 58 && PADEId == 103 ) return 21;
  if( id == 59 && PADEId == 103 ) return 23;
  if( id == 60 && PADEId == 103 ) return 24;
  if( id == 61 && PADEId == 103 ) return 26;
  if( id == 62 && PADEId == 103 ) return 29;
  if( id == 63 && PADEId == 103 ) return 31;
  
  if( id == 0 && PADEId == 28 ) return 7;
  if( id == 1 && PADEId == 28 ) return 0;
  if( id == 2 && PADEId == 28 ) return 3;
  if( id == 3 && PADEId == 28 ) return 9;
  if( id == 4 && PADEId == 28 ) return 2;
  if( id == 5 && PADEId == 28 ) return 14;
  if( id == 6 && PADEId == 28 ) return 12;
  if( id == 7 && PADEId == 28 ) return 6;
  if( id == 8 && PADEId == 28 ) return 1;
  
  return -1;
}

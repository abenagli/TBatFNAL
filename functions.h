#ifndef functions_h
#define functions_h

#include <iostream>
#include <vector>

#include "TGraph.h"

void AddWaveform(TGraph* g, std::vector<int>* v, bool pedSub = true);
float CalculatePedestal(std::vector<int>* v);
void CalculateAmplitude(std::vector<int>* v, float& ped, float& integral, float& maximum);

void NormalizeGraph(TGraph* g, const float& norm);
float GetMinimum(TGraph* g);
float GetMaximum(TGraph* g);

int GetFiberId(const int& chId, const int& PADEId);
int GetCrystalId(const int& chId, const int& PADEId);
int GetChId(const int& id, const int& PADEId);

#endif

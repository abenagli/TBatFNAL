#ifndef functions_h
#define functions_h

#include <iostream>
#include <vector>

#include "TGraph.h"

void AddWaveform(TGraph* g, std::vector<float>* v, bool pedSub = true);
float CalculatePedestal(std::vector<float>* v);
void CalculateAmplitude(std::vector<float>* v, float& ped, float& integral, float& maximum);

void NormalizeGraph(TGraph* g, const float& norm);
float GetMinimum(TGraph* g);
float GetMaximum(TGraph* g);

int GetFiberId(const int& chId, const int& PADEId);
int GetCrystalId(const int& chId, const int& PADEId);
int GetChId(const int& id, const int& PADEId);

#endif

// g++ -Wall -o DQMPlots.exe `root-config --cflags --glibs` functions.cc setTDRStyle.cc DQMPlots.cpp

#include "setTDRStyle.h"
#include "functions.h"

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <cmath>

#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH1F.h"
#include "TProfile2D.h"
#include "TF1.h"
#include "TLatex.h"



int main(int argc, char** argv)
{
  setTDRStyle();
  gStyle -> SetOptFit(0000);
  
  int nFib = 64;
  int nCryst = 9;
  
  std::string inFileName(argv[1]);
  
  
  //----------
  // open file
  
  TFile* inFile = TFile::Open(Form("ntuples/tot_capture_%s.root",inFileName.c_str()));
  TTree* tree = (TTree*)( inFile->Get("tree") );
  
  
  //---------------------
  // set branch addresses
  
  std::map<int,std::vector<int>*> t_waveform;
  std::map<int,std::vector<int>*> t_crystWaveform;
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    t_waveform[fibIt] = new std::vector<int>;
    tree -> SetBranchAddress(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
  }
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    t_crystWaveform[crystIt] = new std::vector<int>;
    tree -> SetBranchAddress(Form("cryst%01d_waveform",crystIt),&t_crystWaveform[crystIt]);
  }
  
  
  
  //-------------
  // define plots
  
  std::map<int,int> n_waveform_fib;
  TGraph** g_waveform_fib = new TGraph*[nFib];
  
  std::map<int,int> n_waveform_cut_fib;
  TGraph** g_waveform_cut_fib = new TGraph*[nFib];
  
  TH1F** h_ped_fib = new TH1F*[nFib];
  TH1F* h_ped_fib_all = new TH1F("h_ped_fib_all","",100,80.,120.);
  
  TH1F** h_maximum_fib = new TH1F*[nFib];
  TH1F* h_maximum_fib_all = new TH1F("h_maximum_fib_all","",1000,0.,2500.);
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    g_waveform_fib[fibIt] = new TGraph();
    g_waveform_cut_fib[fibIt] = new TGraph();
    
    h_ped_fib[fibIt] = new TH1F(Form("h_ped_fib%02d",fibIt),"",100,80.,120.);
    h_maximum_fib[fibIt] = new TH1F(Form("h_maximum_fib%02d",fibIt),"",1000,0.,2500.);
  }
  
  TProfile2D* p_fibAveInt = new TProfile2D("p_fibAveInt","",17,-0.5,16.5,18,-0.5,17.5);
  TProfile2D* p_fibAveMax = new TProfile2D("p_fibAveMax","",17,-0.5,16.5,18,-0.5,17.5);
  TProfile2D* p_crystAveMax = new TProfile2D("p_crystAveMax","",3,-0.5,2.5,3,-0.5,2.5);
  
  TH1F* h_tot_integral = new TH1F("h_tot_integral","",1000,0.,100000.);
  TH1F* h_tot_maximum  = new TH1F("h_tot_maximum", "",1000,0.,1000.);
  
  
  //------------------
  // loop over entries
  
  for(int entry = 0; entry < tree->GetEntries(); ++entry)
  {
    std::cout << ">>> reading entry " << entry << " / " << tree->GetEntries() << "\r" << std::flush;
    tree -> GetEntry(entry);
    
    float tot_integral = 0.;
    float tot_maximum = 0.;
    
    for(int fibIt = 0; fibIt < nFib; ++fibIt)
    {
      ++n_waveform_fib[fibIt];
      AddWaveform(g_waveform_fib[fibIt],t_waveform[fibIt]);
      
      float ped, integral, maximum;
      CalculateAmplitude(t_waveform[fibIt],ped,integral,maximum);
      
      h_ped_fib[fibIt] -> Fill(ped);
      h_ped_fib_all -> Fill(ped);
      h_maximum_fib[fibIt] -> Fill(maximum);
      h_maximum_fib_all -> Fill(maximum);
      
      int x = 16-2*int(fibIt/8);
      int y = (x/2)%2 == 0 ? 16-2*(fibIt%8) : 16-2*(fibIt%8)-1;
      p_fibAveInt -> Fill(x,y,integral);
      p_fibAveMax -> Fill(x,y,maximum);
      
      if( maximum+ped > 120. )
      {
        tot_integral += integral;
        tot_maximum += maximum;
        
        ++n_waveform_cut_fib[fibIt];
        AddWaveform(g_waveform_cut_fib[fibIt],t_waveform[fibIt]);
      }
    }
    
    for(int crystIt = 0; crystIt < nCryst; ++crystIt)
    {
      float ped, integral, maximum;
      CalculateAmplitude(t_waveform[crystIt],ped,integral,maximum);
      p_crystAveMax -> Fill(crystIt%3,2-crystIt/3,maximum);
    }
    
    h_tot_integral -> Fill(tot_integral);
    h_tot_maximum -> Fill(tot_maximum);
  }
  
  
  
  TCanvas* c_waveform_fib_all = new TCanvas();
  
  int plotIt = 0;
  float min = +999999.;
  float max = -999999.;
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    TGraph* g = g_waveform_fib[fibIt];
    if( g->GetN() == 0 ) continue;
    
    NormalizeGraph(g,n_waveform_fib[fibIt]);
    
    if( GetMinimum(g) < min ) min = GetMinimum(g);
    if( GetMaximum(g) > max ) max = GetMaximum(g);
  }
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    TGraph* g = g_waveform_fib[fibIt];
    if( g->GetN() == 0 ) continue;
    
    TCanvas* c_waveform_fib = new TCanvas();
    
    g -> SetMinimum(min-0.05*fabs(max-min));
    g -> SetMaximum(max+0.05*fabs(max-min));
    g -> SetLineWidth(2);
    g -> SetLineColor(fibIt+1);
    g -> SetMarkerSize(0.2);
    g -> GetXaxis() -> SetTitle("sample time (ns)");
    g -> Draw("APL");
    
    c_waveform_fib -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/plotsPerFib/waveform_fib%02d.png",inFileName.c_str(),fibIt),"png");
    
    c_waveform_fib_all -> cd();
    
    if( plotIt == 0 ) g -> Draw("APL");
    else              g -> Draw("PL,same");
    
    ++plotIt;
  }
  
  c_waveform_fib_all -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/waveform_fib_all.png",inFileName.c_str()),"png");
  
  
  
  TCanvas* c_waveform_cut_fib_all = new TCanvas();
  
  plotIt = 0;
  min = +999999.;
  max = -999999.;
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    TGraph* g = g_waveform_cut_fib[fibIt];
    if( g->GetN() == 0 ) continue;
    
    NormalizeGraph(g,n_waveform_cut_fib[fibIt]);
    
    if( GetMinimum(g) < min ) min = GetMinimum(g);
    if( GetMaximum(g) > max ) max = GetMaximum(g);
  }
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    TGraph* g = g_waveform_cut_fib[fibIt];
    if( g->GetN() == 0 ) continue;
    
    //g -> SetMinimum(min-0.05*fabs(max-min));
    //g -> SetMaximum(max+0.05*fabs(max-min));
    g -> SetLineWidth(2);
    g -> SetLineColor(fibIt+1);
    g -> SetMarkerSize(0.2);
    g -> GetXaxis() -> SetTitle("sample time (ns)");
    g -> Draw("APL");
    
    c_waveform_cut_fib_all -> cd();
    
    if( plotIt == 0 ) g -> Draw("APL");
    else              g -> Draw("PL,same");
    
    ++plotIt;
  }
  
  c_waveform_cut_fib_all -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/waveform_cut_fib_all.png",inFileName.c_str()),"png");
  
  
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    TH1F* h = h_ped_fib[fibIt];
    
    TCanvas* c_ped_fib = new TCanvas();
    c_ped_fib -> SetLogy();
    
    h -> SetLineWidth(2);
    h -> GetXaxis() -> SetTitle("max sample");
    h -> Draw();
    h -> Fit("gaus","Q");
    
    TLatex* latex1 = new TLatex(0.60,0.90,Form("RMS = %.1f",h->GetRMS()));
    latex1 -> SetNDC();
    latex1 -> SetTextFont(42);
    latex1 -> SetTextSize(0.04);
    latex1 -> Draw("same");
    
    TLatex* latex2 = new TLatex(0.60,0.85,Form("#sigma = %.1f",h->GetFunction("gaus")->GetParameter(2)));
    latex2 -> SetNDC();
    latex2 -> SetTextFont(42);
    latex2 -> SetTextSize(0.04);
    latex2 -> Draw("same");
    
    c_ped_fib -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/plotsPerFib/ped_fib%02d.png",inFileName.c_str(),fibIt),"png");
    
    h = h_maximum_fib[fibIt];
    
    TCanvas* c_maximum_fib = new TCanvas();
    c_maximum_fib -> SetLogy();
    
    h -> SetLineWidth(2);
    h -> GetXaxis() -> SetTitle("max sample");
    h -> Draw();
    
    c_maximum_fib -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/plotsPerFib/maximum_fib%02d.png",inFileName.c_str(),fibIt),"png");
  }
  
  TCanvas* c_ped_fib_all = new TCanvas();
  c_ped_fib_all -> SetLogy();
  
  h_ped_fib_all -> SetLineWidth(2);
  h_ped_fib_all -> GetXaxis() -> SetTitle("pedestal");
  h_ped_fib_all -> Draw();
  h_ped_fib_all -> Fit("gaus","Q");
  
  TLatex* latex1 = new TLatex(0.60,0.90,Form("RMS = %.1f",h_ped_fib_all->GetRMS()));
  latex1 -> SetNDC();
  latex1 -> SetTextFont(42);
  latex1 -> SetTextSize(0.04);
  latex1 -> Draw("same");
  
  TLatex* latex2 = new TLatex(0.60,0.85,Form("#sigma = %.1f",h_ped_fib_all->GetFunction("gaus")->GetParameter(2)));
  latex2 -> SetNDC();
  latex2 -> SetTextFont(42);
  latex2 -> SetTextSize(0.04);
  latex2 -> Draw("same");
  c_ped_fib_all -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/ped_fib_all.png",inFileName.c_str()),"png");
  
  TCanvas* c_maximum_fib_all = new TCanvas();
  c_maximum_fib_all -> SetLogy();
  
  h_maximum_fib_all -> SetLineWidth(2);
  h_maximum_fib_all -> GetXaxis() -> SetTitle("max sample");
  h_maximum_fib_all -> Draw();
  
  c_maximum_fib_all -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/maximum_fib_all.png",inFileName.c_str()),"png");
  
  
  
  TCanvas* c_fibAveInt = new TCanvas();
  
  p_fibAveInt -> Draw("COLZ");
  c_fibAveInt -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/fibAveInt.png",inFileName.c_str()),"png");
  
  TCanvas* c_fibAveMax = new TCanvas();
  
  p_fibAveMax -> Draw("COLZ");
  c_fibAveMax -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/fibAveMax.png",inFileName.c_str()),"png");
  
  
  TCanvas* c_crystAveMax = new TCanvas();
  
  p_crystAveMax -> Draw("COLZ");
  c_crystAveMax -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/crystAveMax.png",inFileName.c_str()),"png");
  
  
  
  TCanvas* c_tot_integral = new TCanvas();
  c_tot_integral -> SetLogy();
  
  h_tot_integral -> Draw();
  c_tot_integral -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/tot_integral.png",inFileName.c_str()),"png");
  
  TCanvas* c_tot_maximum = new TCanvas();
  c_tot_maximum -> SetLogy();
  
  h_tot_maximum -> Draw();
  c_tot_maximum -> Print(Form("/afs/cern.ch/user/a/abenagli/www/TBatFNAL/%s/tot_maximum.png",inFileName.c_str()),"png");
}

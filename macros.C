float fSample = 0.075; // GHz
int nCh = 32;
int nFib = 64;
int nCryst = 9;

float min = -50.;
float max = 200.;



void drawWaveform(const std::string& inFileName, const int& entry, const int& fibId)
{
  TFile* inFile = TFile::Open(Form("ntuples/tot_capture_%s.root",inFileName.c_str()));
  TTree* tree = (TTree*)( inFile->Get("tree") );
  
  
  std::vector<int>** t_waveform = new std::vector<int>*[nFib];
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    t_waveform[fibIt] = new std::vector<int>;
    tree -> SetBranchAddress(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
  }
  
  tree -> GetEntry(entry);
  
  int plotIt = 0;
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    if( (fibId != -1) && (fibIt != fibId) ) continue;
    
    TGraph* g = new TGraph();
    for(unsigned int it = 0; it < t_waveform[fibIt]->size(); ++it)
    {
      g -> SetPoint(it,it,t_waveform[fibIt]->at(it));
    }
    
    g -> SetMinimum(min);
    g -> SetMaximum(+max);
    g -> SetLineColor(fibIt+1);
    g -> SetMarkerColor(fibIt+1);
    
    if( plotIt == 0 ) g -> Draw("APL");
    else              g -> Draw("PL,same");
    
    ++plotIt;
  }
}



void drawCrystalWaveform(const std::string& inFileName, const int& entry, const int& crystId)
{
  TFile* inFile = TFile::Open(Form("ntuples/tot_capture_%s.root",inFileName.c_str()));
  TTree* tree = (TTree*)( inFile->Get("tree") );
  
  
  std::vector<int>** t_waveform = new std::vector<int>*[nCryst];
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    t_waveform[crystIt] = new std::vector<int>;
    tree -> SetBranchAddress(Form("cryst%01d_waveform",crystIt),&t_waveform[crystIt]);
  }
  
  tree -> GetEntry(entry);
  
  int plotIt = 0;
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    if( (crystId != -1) && (crystIt != crystId) ) continue;
    
    TGraph* g = new TGraph();
    std::cout << "entry: " << entry << std::endl;
    for(unsigned int it = 0; it < t_waveform[crystIt]->size(); ++it)
    {
      std::cout << t_waveform[crystIt]->at(it) << " ";
      g -> SetPoint(it,it,t_waveform[crystIt]->at(it));
    }
    std::cout << std::endl;
    
    g -> SetMinimum(0.);
    g -> SetMaximum(2000);
    g -> GetXaxis() -> SetRangeUser(0.,300.);
    g -> SetLineColor(crystIt+1);
    g -> SetMarkerColor(crystIt+1);
    
    if( plotIt == 0 ) g -> Draw("APL");
    else              g -> Draw("PL,same");
    
    ++plotIt;
  }
}



void preselection(const std::string& inFileName)
{
  TFile* inFile = TFile::Open(Form("ntuples/tot_capture_%s.root",inFileName.c_str()));
  TTree* tree = (TTree*)( inFile->Get("tree") );
  
  float fib_integral[64];
  float fib_maximum[64];
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {  
    tree -> SetBranchAddress(Form("fib%02d_integral",fibIt),&fib_integral[fibIt]);
    tree -> SetBranchAddress(Form("fib%02d_maximum",fibIt), &fib_maximum[fibIt]);
  }
  

  TH1F* h_fib_integral_all = new TH1F("h_fin_integral_all","",1000,0.,25000.);  
  TH1F* h_fib_maximum_all  = new TH1F("h_fin_maximum_all", "",1000,0.,2500.);

  TH1F* h_fib_integral_all_cut = new TH1F("h_fin_integral_all_cut","",1000,0.,25000.);  
  TH1F* h_fib_maximum_all_cut  = new TH1F("h_fin_maximum_all_cut", "",1000,0.,2500.);  
  
  
  int nAboveNoise = 0;
  for(int entry = 0; entry < tree->GetEntries(); ++entry)
  {
    std::cout << ">>> reading entry " << entry << " / " << tree->GetEntries() << "\r" << std::flush;
    
    tree -> GetEntry(entry);
    
    int nFibAboveNoise = 0;
    for(int fibIt = 0; fibIt < nFib; ++fibIt)
    {
      h_fib_integral_all -> Fill(fib_integral[fibIt]);
      h_fib_maximum_all  -> Fill(fib_maximum[fibIt]);
      
      if( fib_maximum[fibIt] > 160. )
      {
        h_fib_integral_all_cut -> Fill(fib_integral[fibIt]);
        h_fib_maximum_all_cut  -> Fill(fib_maximum[fibIt]);      
        ++nFibAboveNoise;
      }
    }
    
    if( nFibAboveNoise >= 16 ) ++nAboveNoise;
  }
  std::cout << std::endl;
  
  
  
  std::cout << "--> total events: " << tree->GetEntries() << std::endl;
  std::cout << "--> events with >= 16 fib. above nose: " << nAboveNoise << std::endl;  
  
    
  TCanvas* c1_integral = new TCanvas("c1_integral","integral");
  c1_integral -> cd();
  h_fib_integral_all -> Draw();
  
  TCanvas* c1_maximum = new TCanvas("c1_maximum","maximum");
  c1_maximum -> cd();
  h_fib_maximum_all -> Draw();
  
  
  TCanvas* c1_integral_cut = new TCanvas("c1_integral_cut","integral cut");
  c1_integral_cut -> cd();
  h_fib_integral_all_cut -> Draw();
  
  TCanvas* c1_maximum_cut = new TCanvas("c1_maximum_cut","maximum cut");
  c1_maximum_cut -> cd();
  h_fib_maximum_all_cut -> Draw();  
}



void drawWaveformSum(const std::string& inFileName, const int& fibId1, const int& fibId2)
{
  TFile* inFile = TFile::Open(Form("ntuples/tot_capture_%s.root",inFileName.c_str()));
  TTree* tree = (TTree*)( inFile->Get("tree") );
  
  std::vector<int>** t_waveform = new std::vector<int>*[nFib];
  
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    t_waveform[fibIt] = new std::vector<int>;
    tree -> SetBranchAddress(Form("fib%02d_waveform",fibIt),&t_waveform[fibIt]);
  }
  
  
  
  TGraph** g = new TGraph*[nFib];
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
    g[fibIt] = new TGraph();
  }
  
  
  for(int entry = 0; entry < tree->GetEntries(); ++entry)
  {
    std::cout << ">>> reading entry " << entry << " / " << tree->GetEntries() << "\r" << std::flush;
    tree -> GetEntry(entry);
    
    for(int fibIt = 0; fibIt < nFib; ++fibIt)
    {
      if( (fibIt < fibId1) || (fibIt > fibId2) ) continue;
      AddWaveform(g[fibIt],t_waveform[fibIt],entry);
    }
  }
  
  
  
  int plotIt = 0;
  for(int fibIt = 0; fibIt < nFib; ++fibIt)
  {
     TGraph* graph = g[fibIt];
     if( graph->GetN() == 0 ) continue;
     
     graph -> SetName(Form("g_fib%02d",fibIt));
     graph -> SetMinimum(min*tree->GetEntries());
     graph -> SetMaximum(+max*tree->GetEntries());
     graph -> SetLineColor(fibIt+1);
     graph -> SetMarkerColor(fibIt+1);
     
     if( plotIt == 0 ) graph -> Draw("APL");
     else              graph -> Draw("PL,same");
     
     ++plotIt;
  }
}



void drawCrystalWaveformSum(const std::string& inFileName, const int& crystId1, const int& crystId2)
{
  TFile* inFile = TFile::Open(Form("data/tot_capture_%s.root",inFileName.c_str()));
  TTree* tree = (TTree*)( inFile->Get("tree") );
  
  std::vector<int>** t_waveform = new std::vector<int>*[nCryst];
  
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    t_waveform[crystIt] = new std::vector<int>;
    tree -> SetBranchAddress(Form("cryst%01d_waveform",crystIt),&t_waveform[crystIt]);
  }
  
  
  
  TGraph** g = new TGraph*[nCryst];
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    g[crystIt] = new TGraph();
  }
  
  
  for(int entry = 0; entry < tree->GetEntries(); ++entry)
  {
    std::cout << ">>> reading entry " << entry << " / " << tree->GetEntries() << "\r" << std::flush;
    tree -> GetEntry(entry);
    
    for(int crystIt = 0; crystIt < nCryst; ++crystIt)
    {
      if( (crystIt < crystId1) || (crystIt > crystId2) ) continue;
      AddWaveform(g[crystIt],t_waveform[crystIt],entry,false);
    }
  }
  
  
  
  int plotIt = 0;
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
     TGraph* graph = g[crystIt];
     if( graph->GetN() == 0 ) continue;
     
     graph -> SetName(Form("g_cryst%01d",crystIt));
     graph -> SetMinimum(min*tree->GetEntries());
     graph -> SetMaximum(+max*tree->GetEntries());
     graph -> SetLineColor(crystIt+1);
     graph -> SetMarkerColor(crystIt+1);
     
     if( plotIt == 0 ) graph -> Draw("APL");
     else              graph -> Draw("PL,same");
     
     ++plotIt;
  }
}



void drawCrystalAverageMap(const std::string& inFileName, const int& crystId1, const int& crystId2)
{
  TFile* inFile = TFile::Open(Form("data/tot_capture_%s.root",inFileName.c_str()));
  TTree* tree = (TTree*)( inFile->Get("tree") );
  
  std::vector<int>** t_waveform = new std::vector<int>*[nCryst];
  
  for(int crystIt = 0; crystIt < nCryst; ++crystIt)
  {
    t_waveform[crystIt] = new std::vector<int>;
    tree -> SetBranchAddress(Form("cryst%01d_waveform",crystIt),&t_waveform[crystIt]);
  }
  
  
  
  TProfile2D* p_averageMap = new TProfile2D("p_averageMap","",3,-0.5,2.5,3,-0.5,2.5);
  
  for(int entry = 0; entry < tree->GetEntries(); ++entry)
  {
    std::cout << ">>> reading entry " << entry << " / " << tree->GetEntries() << "\r" << std::flush;
    tree -> GetEntry(entry);
    
    for(int crystIt = 0; crystIt < nCryst; ++crystIt)
    {
      if( (crystIt < crystId1) || (crystIt > crystId2) ) continue;
      
      int maximum = -999999;
      float ped = CalculatePedestal(t_waveform[crystIt]);
      for(unsigned int it = 0; it < t_waveform[crystIt]->size(); ++it)
      {
        if( t_waveform[crystIt]->at(it) > maximum)
          maximum = t_waveform[crystIt]->at(it);
      }
      
      p_averageMap -> Fill(crystIt%3,2-crystIt/3,maximum-ped);
    }
  }
  
  
  TCanvas* c1 = new TCanvas(Form("c_averageCrystalMap_%s",inFileName.c_str()),Form("c_averageCrystalMap_%s",inFileName.c_str()));
  p_averageMap -> Draw("COLZ");
  c1 -> Print(Form("crystalAverageMap_%s.pdf",inFileName.c_str()),"pdf");
}



void drawCerenkovWaveformSum(const std::string& inFileName)
{
  TFile* inFile = TFile::Open(Form("data/tot_capture_%s.root",inFileName.c_str()));
  TTree* tree = (TTree*)( inFile->Get("tree") );
  
  std::vector<int>** t_waveform = new std::vector<int>*[2];
  t_waveform[0] = new std::vector<int>;
  t_waveform[1] = new std::vector<int>;
  
  tree -> SetBranchAddress("outCerAna_waveform",&t_waveform[0]);
  tree -> SetBranchAddress("outCerLog_waveform",&t_waveform[1]);
  
  
  TGraph** g = new TGraph*[2];
  for(int it = 0; it < 2; ++it)
  {
    g[it] = new TGraph();
  }
  
  
  for(int entry = 0; entry < tree->GetEntries(); ++entry)
  {
    std::cout << ">>> reading entry " << entry << " / " << tree->GetEntries() << "\r" << std::flush;
    tree -> GetEntry(entry);
    
    for(int it = 0; it < 2; ++it)
    {
      AddWaveform(g[it],t_waveform[it],entry,false);
    }
  }
  
  
  
  int plotIt = 0;
  for(int it = 0; it < 2; ++it)
  {
     TGraph* graph = g[it];
     if( graph->GetN() == 0 ) continue;
     
     graph -> SetName(Form("g_cerenkov%01d",it));
     graph -> SetMinimum(min*tree->GetEntries());
     graph -> SetMaximum(+max*tree->GetEntries());
     graph -> SetLineColor(it+1);
     graph -> SetMarkerColor(it+1);
     
     if( plotIt == 0 ) graph -> Draw("APL");
     else              graph -> Draw("PL,same");
     
     ++plotIt;
  }
}



void AddWaveform(TGraph* g, std::vector<int>* v, const int& entry, bool pedSub = true)
{
  float ped = CalculatePedestal(v);
  if( !pedSub ) ped = 0;
  
  if( entry == 0 )
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



float CalculatePedestal(std::vector<int>* v)
{
  int nPoints = 5;
  float val = 0.;
  for(int it = 0; it < nPoints; ++it)
  {
    val += v->at(it);
  }
  
  return val/nPoints;
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

int GetChannelId(const int& fibId, const int& PADEId)
{
  if( fibId ==  0 && PADEId == 16 ) return 15;
  if( fibId ==  1 && PADEId == 16 ) return 13;
  if( fibId ==  2 && PADEId == 16 ) return 10;
  if( fibId ==  3 && PADEId == 16 ) return  8;
  if( fibId ==  4 && PADEId == 16 ) return  7;
  if( fibId ==  5 && PADEId == 16 ) return  5;
  if( fibId ==  6 && PADEId == 16 ) return  2;
  if( fibId ==  7 && PADEId == 16 ) return  0;
  if( fibId ==  8 && PADEId == 16 ) return  1;
  if( fibId ==  9 && PADEId == 16 ) return  3;
  if( fibId == 10 && PADEId == 16 ) return  4;
  if( fibId == 11 && PADEId == 16 ) return  6;
  if( fibId == 12 && PADEId == 16 ) return  9;
  if( fibId == 13 && PADEId == 16 ) return 11;
  if( fibId == 14 && PADEId == 16 ) return 12;
  if( fibId == 15 && PADEId == 16 ) return 14;
  if( fibId == 16 && PADEId == 16 ) return 17;
  if( fibId == 17 && PADEId == 16 ) return 19;
  if( fibId == 18 && PADEId == 16 ) return 20;
  if( fibId == 19 && PADEId == 16 ) return 22;
  if( fibId == 20 && PADEId == 16 ) return 25;
  if( fibId == 21 && PADEId == 16 ) return 27;
  if( fibId == 22 && PADEId == 16 ) return 28;
  if( fibId == 23 && PADEId == 16 ) return 30;
  if( fibId == 24 && PADEId == 16 ) return 31;
  if( fibId == 25 && PADEId == 16 ) return 29;
  if( fibId == 26 && PADEId == 16 ) return 26;
  if( fibId == 27 && PADEId == 16 ) return 24;
  if( fibId == 28 && PADEId == 16 ) return 23;
  if( fibId == 29 && PADEId == 16 ) return 21;
  if( fibId == 30 && PADEId == 16 ) return 18;
  if( fibId == 31 && PADEId == 16 ) return 16;
  
  if( fibId == 32 && PADEId == 103 ) return  0;
  if( fibId == 33 && PADEId == 103 ) return  2;
  if( fibId == 34 && PADEId == 103 ) return  5;
  if( fibId == 35 && PADEId == 103 ) return  7;
  if( fibId == 36 && PADEId == 103 ) return  8;
  if( fibId == 37 && PADEId == 103 ) return 10;
  if( fibId == 38 && PADEId == 103 ) return 13;
  if( fibId == 39 && PADEId == 103 ) return 15;
  if( fibId == 40 && PADEId == 103 ) return 14;
  if( fibId == 41 && PADEId == 103 ) return 12;
  if( fibId == 42 && PADEId == 103 ) return 11;
  if( fibId == 43 && PADEId == 103 ) return  9;
  if( fibId == 44 && PADEId == 103 ) return  6;
  if( fibId == 45 && PADEId == 103 ) return  4;
  if( fibId == 46 && PADEId == 103 ) return  3;
  if( fibId == 47 && PADEId == 103 ) return  1;
  if( fibId == 48 && PADEId == 103 ) return 30;
  if( fibId == 49 && PADEId == 103 ) return 28;
  if( fibId == 50 && PADEId == 103 ) return 27;
  if( fibId == 51 && PADEId == 103 ) return 25;
  if( fibId == 52 && PADEId == 103 ) return 22;
  if( fibId == 53 && PADEId == 103 ) return 20;
  if( fibId == 54 && PADEId == 103 ) return 19;
  if( fibId == 55 && PADEId == 103 ) return 17;
  if( fibId == 56 && PADEId == 103 ) return 16;
  if( fibId == 57 && PADEId == 103 ) return 18;
  if( fibId == 58 && PADEId == 103 ) return 21;
  if( fibId == 59 && PADEId == 103 ) return 23;
  if( fibId == 60 && PADEId == 103 ) return 24;
  if( fibId == 61 && PADEId == 103 ) return 26;
  if( fibId == 62 && PADEId == 103 ) return 29;
  if( fibId == 63 && PADEId == 103 ) return 31;
  
  return -1;
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


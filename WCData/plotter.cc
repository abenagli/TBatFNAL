#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <stdlib.h>
#include <fstream>
#include "TRandom3.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLorentzVector.h"
#include "TPaveStats.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TStyle.h"
#include <algorithm>
#include <math.h>
#include <complex>

struct WCData
{
  int x;
  int y;
  int t;
  int wc;
};

using namespace std;



int main(int argc, char *argv[])
{
  string version="RPCSim5";
  
  char infilename[500];char outfilename[500];
  char hname[200];
  string fno=argv[1];
  sprintf(infilename,"%s",fno.c_str());
  sprintf(outfilename,"WCout_%s.root",fno.c_str());
  cout<<infilename<<" "<<outfilename<<endl;
  
  ifstream infile(infilename);
  int a[3]={0};double b[11]={0.};double e[2]={0.};
  
  TFile* outfile=new TFile(outfilename,"recreate");
  TH2F* WCXY[4];TH1F* TDC_WC[4];TH1F* XProf[4];TH1F* YProf[4];
  TH2F* TDC_vs_X[4];TH2F* TDC_vs_Y[4];
  for(int i1=0;i1<4;i1++)
  {
    sprintf(hname,"WC%d",i1+1);
    WCXY[i1]=new TH2F(hname,hname,128,-0.5,127.5,128,-0.5,127.5);
    sprintf(hname,"TDC_WC%d",i1+1);
    TDC_WC[i1]=new TH1F(hname,hname,300,-0.5,299.5);
    sprintf(hname,"XProf_WC%d",i1+1);
    XProf[i1]=new TH1F(hname,hname,128,-0.5,127.5);
    sprintf(hname,"YProf_WC%d",i1+1);
    YProf[i1]=new TH1F(hname,hname,128,-0.5,127.5);
    sprintf(hname,"TDC_vs_X_WC%d",i1+1);
    TDC_vs_X[i1]=new TH2F(hname,hname,128,-0.5,127.5,300,-0.5,299.5);
    sprintf(hname,"TDC_vs_Y_WC%d",i1+1);
    TDC_vs_Y[i1]=new TH2F(hname,hname,128,-0.5,127.5,300,-0.5,299.5);
  }
  
  vector <int> data[6];int dd[6]={0};vector <int> datae[6];
  int wcmap[17]={0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4};
  
  int nline=0;int nevt=0;int nhits=0;int NS=1;
  while(!infile.eof())
  {
    infile>>dd[0]>>dd[1]>>dd[2]>>dd[3]>>dd[4]>>dd[5];
    if(nline==0) {for(int i1=0;i1<6;i1++){data[i1].push_back(dd[i1]);}}
    else
    {
      if(dd[0]==data[0].at(data[0].size()-1))
      {
        if(dd[1]!=data[1].at(data[1].size()-1))
        {
          vector <WCData> WCdata;
          int x1=0;int y1=0;
          for(int i1=0;i1<datae[0].size();i1++)
          {
            int t=datae[5].at(i1);
            int tdc=datae[3].at(i1);
            int m=datae[4].at(i1);
            int wc=wcmap[tdc];
            TDC_WC[wc-1]->Fill(t);
            int x=-1;int y=-1;
            if(tdc==1 || tdc==2) x=(tdc-1)*64+m;
            if(tdc==3 || tdc==4) y=(tdc-3)*64+m;
            if(tdc==5 || tdc==6) x=(tdc-5)*64+m;
            if(tdc==7 || tdc==8) y=(tdc-7)*64+m;
            if(tdc==9 || tdc==10) x=(tdc-9)*64+m;
            if(tdc==11 || tdc==12) y=(tdc-11)*64+m;
            if(tdc==13 || tdc==14) x=(tdc-13)*64+m;
            if(tdc==15 || tdc==16) y=(tdc-15)*64+m;
            WCData wcc;
            wcc.x=x;wcc.y=y;wcc.t=t;wcc.wc=wc;
            WCdata.push_back(wcc);
          }
          
          int nx=0;int ny=0;int P[2]={0};int Pt[2]={1000,1000};
          int wcFound[4]={0};
          int Pp[4][2]={0};int Ppt[4][2]={{0}};
          for(int i1=0;i1<4;i1++){for(int i2=0;i2<2;i2++){Ppt[i1][i2]=1000;}}
          for(int i1=0;i1<WCdata.size();i1++)
          {
            if(WCdata[i1].x!=-1)
            {
              if(WCdata[i1].t<Ppt[WCdata[i1].wc-1][0]) {Ppt[WCdata[i1].wc-1][0]=WCdata[i1].t;Pp[WCdata[i1].wc-1][0]=WCdata[i1].x;}
            }
            if(WCdata[i1].y!=-1)
            {
              if(WCdata[i1].t<Ppt[WCdata[i1].wc-1][1]) {Ppt[WCdata[i1].wc-1][1]=WCdata[i1].t;Pp[WCdata[i1].wc-1][1]=WCdata[i1].y;}
            }
          }
          for(int i1=0;i1<4;i1++)
          {
            WCXY[i1]->Fill(Pp[i1][0],Pp[i1][1]);
          }
          
          for(int i1=0;i1<6;i1++){datae[i1].clear();}
          for(int i1=0;i1<6;i1++){datae[i1].push_back(dd[i1]);}
          nevt++;
        }
        else
        {
          for(int i1=0;i1<6;i1++){datae[i1].push_back(dd[i1]);}
        }
        for(int i1=0;i1<6;i1++){data[i1].push_back(dd[i1]);}
        nhits++;
      }
      else
      {
        //cout<<NS<<" : Spill "<<data[0].at(data[0].size()-1)<<" "<<nevt<<" events "<<nhits<<" hits"<<endl;
        cout<<(nevt+1)<<",";
        nhits=0;
        nevt=0;
        NS++;
        for(int i1=0;i1<6;i1++){data[i1].clear();}
        for(int i1=0;i1<6;i1++){data[i1].push_back(dd[i1]);}
      }
    }
    nline++;
    //if(nline>10000) break;
  }
  cout<<(nevt+1);
  infile.close();
  cout<<endl<<(NS-1)<<endl;
  outfile->Write();
  outfile->Close();
}

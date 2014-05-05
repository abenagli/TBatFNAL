// g++ -Wall -o plotterNew.exe `root-config --cflags --glibs` plotterNew.cc

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

std::string inFolder = "./WC_captures";



int main(int argc, char **argv)
{
  char infilename[500];char outfilename[500];
  char hname[200];
  
  string fno=argv[1];
  
  sprintf(infilename,"%s/WC_capture_%s.out",inFolder.c_str(),fno.c_str());
  sprintf(outfilename,"%s/WC_capture_%s.root",inFolder.c_str(),fno.c_str());
  cout << ">>> reading " << infilename << endl;
  cout << ">>> writing " << outfilename << endl;
  
  ifstream infile(infilename);
  
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
  std::string timeStamp;
  int spillNum;
  int eventNum;
  int tdcNum;
  int chNum;
  int tdcT;
  while(!infile.eof())
  {
    infile >> timeStamp >> spillNum >> eventNum >> tdcNum >> chNum >> tdcT;
    dd[0] = spillNum;
    dd[1] = eventNum;
    dd[2] = eventNum;
    dd[3] = tdcNum;
    dd[4] = chNum;
    dd[5] = tdcT;
    //cout << "spill: " << dd[0] << "   event: " << dd[1] << "    event: " << dd[2] << "    tdc: " << dd[3] << "    m: " << dd[4] << "    t: " << dd[5] << endl;
    
    if(nline==0) {for(int i1=0;i1<6;i1++){data[i1].push_back(dd[i1]);} }
    else
    {
      //cout << "spill check: " << dd[0] << " vs " << data[0].at(data[0].size()-1) << endl;
      //cout << "event check: " << dd[1] << " vs " << data[1].at(data[1].size()-1) << endl;

      if(dd[0]==data[0].at(data[0].size()-1)) // same spill
      {
        //cout << "same spill" << endl;
        if(dd[1]!=data[1].at(data[1].size()-1)) // different event
        {
          //cout << "different event" << endl;
          vector <WCData> WCdata;
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
            //cout << "wc: " << wcc.wc << "   tdc: " << tdc << "   m: " << m << "   x: " << wcc.x << "   y: " << wcc.y << "   t: " << wcc.t << endl;
          }
          
          int Pp[4][2]={{0}};int Ppt[4][2]={{0}};
          for(int i1=0;i1<4;i1++){for(int i2=0;i2<2;i2++){Ppt[i1][i2]=1999999999;}}
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
            //cout << "fill wc " << i1+1 << " with " << Pp[i1][0] << "," << Pp[i1][1] << endl;
          }
          
          for(int i1=0;i1<6;i1++){datae[i1].clear();}
          for(int i1=0;i1<6;i1++){datae[i1].push_back(dd[i1]);}
          nevt++;
        }
        else // same event
        {
          //cout << "same event" << endl;
          for(int i1=0;i1<6;i1++){datae[i1].push_back(dd[i1]);}
          //cout << "push back datae" << endl;
        }
        
        for(int i1=0;i1<6;i1++){data[i1].push_back(dd[i1]);}
        //cout << "push back data" << endl;
        
        nhits++;
      }
      else // new spill
      {
        //cout << "new spill" << endl;
        //cout<<NS<<" : Spill "<<data[0].at(data[0].size()-1)<<",   "<<nevt<<" events,   "<<nhits<<" hits"<<endl;
        //cout<<(nevt+1)<<",";
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
  //cout<<(nevt+1);
  infile.close();
  cout<<">>> number of spills processed: "<< (NS-1) << endl;
  outfile->Write();
  outfile->Close();
}

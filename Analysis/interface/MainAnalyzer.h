#ifndef MainAnalyzer_h
#define MainAnalyzer_h

#include "TtZAnalysis/DataFormats/src/classes.h" //gets all the dataformats
//#include "TtZAnalysis/DataFormats/interface/elecRhoIsoAdder.h"
#include "TopAnalysis/ZTopUtils/interface/PUReweighter.h"
#include "TtZAnalysis/plugins/leptonSelector2.h"
#include "TopAnalysis/ZTopUtils/interface/miscUtils.h"
#include "TtZAnalysis/Tools/interface/containerStackVector.h"
//#include "TtZAnalysis/Tools/interface/containerUF.h"
#include "TtZAnalysis/DataFormats/interface/NTJERAdjuster.h"
#include "TtZAnalysis/DataFormats/interface/NTJECUncertainties.h"
#include "TtZAnalysis/DataFormats/interface/NTBTagSF.h"
//#include "TtZAnalysis/Tools/interface/bTagSF.h"
#include "TTree.h"
#include "TFile.h"
#include <fstream>
#include <omp.h>
#include "TtZAnalysis/Analysis/interface/AnalysisUtils.h"

#include <stdlib.h>

bool testmode=true;



namespace ztop{
  typedef std::vector<ztop::NTElectron>::iterator NTElectronIt;
  typedef std::vector<ztop::NTMuon>::iterator NTMuonIt;
  typedef std::vector<ztop::NTJet>::iterator NTJetIt;
  typedef std::vector<ztop::NTTrack>::iterator NTTrackIt;
  typedef std::vector<ztop::NTSuClu>::iterator NTSuCluIt;
}

///// now available: removeContribution; use it to run on the systematics etc (only signal) or pdf stuff (remove nominal,for(i) add pdf[i], xsec, remove pdf[i])

//// run in batch mode otherwise it is dramatically slowed down by all the drawing stuff; the latter might also produce seg violations in the canvas libs.
//// name Z contribution "Z" or something similar for generator stuff and so on

//maybe the container filling uses a lot of cpu time, check that! checked. inlined and pow->sq




class MainAnalyzer{

public:
  MainAnalyzer();
  MainAnalyzer(const MainAnalyzer &);
 
  ~MainAnalyzer(){std::cout << "~MainAnalyzer" << std::endl;}

  void setChannel(TString chan){channel_=chan;}
  void setSyst(TString syst){syst_=syst;}
  void setEnergy(TString e){energy_=e;}
  void setOutFile(TString o){outfile_=o;}
  
 
  void setLumi(double Lumi){lumi_=Lumi;}

  // void replaceInAllOptions(TString replace, TString with){name_.ReplaceAll(replace,with);analysisplots_.setName(name_);}
  TString getChannel(){return channel_;}
  TString getSyst(){return syst_;}
  TString Energy(){return energy_;}


 
void analyze(TString, TString, int, double,size_t i=0);
   void analyze(size_t i);

  void setFileList(TString );
  void setDataSetDirectory(TString dir){datasetdirectory_=dir;}
  void setShowStatusBar(bool show){showstatusbar_=show;}

  ztop::container1DStackVector * getPlots(){return & analysisplots_;}

  int start();
  //  void start(TString);

  void clear(){analysisplots_.clear();}

  ztop::PUReweighter * getPUReweighter(){return & puweighter_;}
  ztop::NTJERAdjuster * getJERAdjuster(){return & jeradjuster_;}
  ztop::NTJECUncertainties * getJECUncertainties(){return & jecuncertainties_;}
  ztop::NTBTagSF * getBTagSF(){return &btagsf_;}

  MainAnalyzer & operator= (const MainAnalyzer &);

  

private:

  

  void copyAll(const MainAnalyzer &);


  bool showstatusbar_;

  TString name_,dataname_,channel_,syst_,energy_;
  TString datasetdirectory_;
  double lumi_;

  TString filelist_;
  
  ztop::PUReweighter  puweighter_;
  ztop::NTJERAdjuster  jeradjuster_;
  ztop::NTJECUncertainties  jecuncertainties_;
  ztop::NTBTagSF  btagsf_;

  ztop::container1DStackVector  analysisplots_;

  size_t filecount_;
  TString outfile_;

  //for parallel stuff

  std::vector<TString> infiles_,legentries_;
  std::vector<int> colz_;
  std::vector<double> norms_;

///communication pipes

  IPCPipes<int> p_idx;
  IPCPipes<int> p_finished;

  IPCPipes<int> p_allowwrite;
  IPCPipes<int> p_askwrite;

  bool writeAllowed_;

  // bool askForWrite();
  int  checkForWriteRequest();
  // void blockWrite();
  // void freeWrite();


pid_t PID_;
std::vector<pid_t> daughPIDs_;


};

#endif

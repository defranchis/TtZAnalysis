#ifndef MainAnalyzer_h
#define MainAnalyzer_h

#include "TtZAnalysis/DataFormats/src/classes.h" //gets all the dataformats
#include "TtZAnalysis/DataFormats/interface/elecRhoIsoAdder.h"
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
  void setOutFileAdd(TString o){outfileadd_=o;}
  
  TString getOutFileName(){
    if(outfileadd_=="") return channel_+"_"+energy_+"_"+syst_;
    else return channel_+"_"+energy_+"_"+syst_+"_"+outfileadd_;
  } 

  void setOutDir(TString dir){
    if(dir.EndsWith("/"))
      outdir_=dir;
    else
      outdir_=dir+"/";
  }

  TString getOutPath(){return outdir_+getOutFileName();}

  void setBTagSFFile(TString file){btagsffile_=file;}

  void setLumi(double Lumi){lumi_=Lumi;}

  // void replaceInAllOptions(TString replace, TString with){name_.ReplaceAll(replace,with);analysisplots_.setName(name_);}
  TString getChannel(){return channel_;}
  TString getSyst(){return syst_;}
  TString Energy(){return energy_;}


 
  void analyze(TString, TString, int, double,size_t,size_t i=0);
  void analyze(size_t i);

  void setFileList(TString fl){filelist_=fl;}
  void setFilePostfixReplace(TString file,TString pf,bool clear=false){
    if(clear){ fwithfix_.clear();ftorepl_.clear();} 
    ftorepl_.push_back(file); fwithfix_.push_back(pf);
  }

  void setFilePostfixReplace(std::vector<TString> files,std::vector<TString> pf){
    if(files.size() != pf.size()){std::cout << "setFilePostfixReplace: vectors have to be same size, exit!" << std::endl; std::exit(EXIT_FAILURE);} 
    ftorepl_=files;fwithfix_=pf;
  }

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
  void readFileList(); //run automatically when start() is called

  TString replaceExtension(TString );

  bool showstatusbar_;

  TString name_,dataname_,channel_,syst_,energy_;
  TString datasetdirectory_;
  double lumi_;

  TString filelist_;
  std::vector<TString> fwithfix_,ftorepl_;
  int freplaced_;
  
  ztop::PUReweighter  puweighter_;
  ztop::NTJERAdjuster  jeradjuster_;
  ztop::NTJECUncertainties  jecuncertainties_;
  ztop::NTBTagSF  btagsf_;

  ztop::container1DStackVector  analysisplots_;

  size_t filecount_;
  TString outfileadd_;
  TString outdir_;

  TString btagsffile_;

  //for parallel stuff

  std::vector<TString> infiles_,legentries_;
  std::vector<int> colz_;
  std::vector<double> norms_;
  std::vector<size_t> legord_;

///communication pipes

  IPCPipes<int> p_idx;
  IPCPipes<int> p_finished;

  IPCPipes<int> p_allowwrite;
  IPCPipes<int> p_askwrite;

  IPCPipes<int> p_status; //not  implemented, yet

  bool writeAllowed_;

  // bool askForWrite();
  int  checkForWriteRequest();
  // void blockWrite();
  // void freeWrite();


pid_t PID_;
std::vector<pid_t> daughPIDs_;


};

#endif

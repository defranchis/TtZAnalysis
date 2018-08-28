#include <iostream>
#include <fstream>
#include <iomanip>

#include "TString.h"
#include "TFile.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TKey.h"
#include "TDirectoryFile.h"                        
#include "TList.h"
#include "TROOT.h"
#include "TError.h"
#include "TSystem.h"

#include "TtZAnalysis/Tools/interface/histo1D.h"
#include "TtZAnalysis/Tools/interface/histoStack.h"
#include "TtZAnalysis/Tools/interface/histoStackVector.h"
#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "TtZAnalysis/Tools/interface/plotterMultiplePlots.h"

invokeApplication(){

    using namespace ztop ;

    histoStackVector * csv=new histoStackVector();
    csv->readFromFile("emu_13TeV_172.5_nominal.ztop");

    histoStack stack_multi=csv->getStack("forward jet multiplicity step 8");
    histoStack stack_pt   =csv->getStack("leading forward jet pt step 8"  );

    histo1D multi_sig = stack_multi.getSignalContainer()    ;
    histo1D multi_bkg = stack_multi.getBackgroundContainer();

    histo1D pt_sig    = stack_pt.getSignalContainer()    ;
    histo1D pt_bkg    = stack_pt.getBackgroundContainer();

    TFile * f = new TFile("forward.root","RECREATE");

    TH1D * h_multi_sig = multi_sig.getTH1D();
    TH1D * h_multi_bkg = multi_bkg.getTH1D();

    TH1D * h_pt_sig = pt_sig.getTH1D();
    TH1D * h_pt_bkg = pt_bkg.getTH1D();

    h_multi_sig->SetName("h_multi_sig");
    h_multi_bkg->SetName("h_multi_bkg");
    h_pt_sig->SetName("h_pt_sig");
    h_pt_bkg->SetName("h_pt_bkg");

    h_multi_sig->Write();
    h_multi_bkg->Write();

    h_pt_sig->Write();
    h_pt_bkg->Write();

    f->Close();
    delete f;

    return 0;

}

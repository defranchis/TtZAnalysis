/*
 * plotControlPlot.cc
 *
 *  Created on: Nov 24, 2014
 *      Author: kiesej
 */


#include "TtZAnalysis/Tools/interface/applicationMainMacro.h"
#include "TtZAnalysis/Tools/interface/plotterControlPlot.h"
#include "TtZAnalysis/Tools/interface/histoStackVector.h"
#include "TtZAnalysis/Tools/interface/fileReader.h"
#include "TtZAnalysis/Tools/interface/textFormatter.h"
#include "TCanvas.h"
#include "TFile.h"
/**
 * Small application to plot a control plot in a fast and efficient way. Returns a pdf file
 */
invokeApplication(){
    using namespace ztop;
    const TString infile = "emu_13TeV_172.5_nominal.ztop";

    TFile* f = new TFile("test_unfold.root","RECREATE");
    histoStackVector * csv = new histoStackVector();

    try{
        csv->readFromFile(infile.Data());
    }catch(...){
        std::cout << "No Plots found in file " << infile <<std::endl;
        delete csv;
        return -1;
    }

    histoStack stack = csv->getStack("m_tt kin reco fine step 8");
    histo1DUnfold h = stack.getSignalContainer1DUnfold();

    TH1D * purity = h.getPurity().getTH1D("purity",false,false,false);
    TH1D * stability = h.getStability().getTH1D("stability",false,false,false);
    TH2D * response = h.getResponseMatrix().getTH2D("response",false,false);

    purity->Write();
    stability->Write();
    response->Write();

    f->Close();
    delete csv;
    return 0;
}

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
    std::vector<TString> tmpv=parser->getRest<TString>();
    const TString infile = tmpv.at(0);
    TString outfile = infile;
    outfile.ReplaceAll(".ztop","_genplots.root");

    TFile* f = new TFile(outfile,"RECREATE");
    histoStackVector * csv = new histoStackVector();

    try{
        csv->readFromFile(infile.Data());
    }catch(...){
        std::cout << "No Plots found in file " << infile <<std::endl;
        delete csv;
        return -1;
    }

    // histoStack stack = csv->getStack("m_tt kin reco fine step 8");
    // histo1DUnfold h = stack.getSignalContainer1DUnfold();

    // TH1D * purity = h.getPurity().getTH1D("purity",false,false,false);
    // TH1D * stability = h.getStability().getTH1D("stability",false,false,false);
    // TH2D * response = h.getResponseMatrix().getTH2D("response",false,false);

    // purity->Write();
    // stability->Write();
    // response->Write();

    histoStack stack_0 = csv->getStack("gen m_ttbar full step 0");
    TH1D * gen_0 = stack_0.getSignalContainer().getTH1D("gen_0",false,false,false);
    gen_0->Write();

    // histoStack stack_1 = csv->getStack("gen m_ttbar mtt1 step 0");
    // TH1D * gen_1 = stack_1.getSignalContainer().getTH1D("gen_1",false,false,false);
    // gen_1->Write();

    // histoStack stack_2 = csv->getStack("gen m_ttbar mtt2 step 0");
    // TH1D * gen_2 = stack_2.getSignalContainer().getTH1D("gen_2",false,false,false);
    // gen_2->Write();

    // histoStack stack_3 = csv->getStack("gen m_ttbar mtt3 step 0");
    // TH1D * gen_3 = stack_3.getSignalContainer().getTH1D("gen_3",false,false,false);
    // gen_3->Write();

    // histoStack stack_4 = csv->getStack("gen m_ttbar mtt4 step 0");
    // TH1D * gen_4 = stack_4.getSignalContainer().getTH1D("gen_4",false,false,false);
    // gen_4->Write();

    histoStack stack_5 = csv->getStack("gen m_ttbar fine step 0");
    TH1D * gen_5 = stack_5.getSignalContainer().getTH1D("gen_5",false,false,false);
    gen_5->Write();

    histoStack stack_6 = csv->getStack("gen m_ttbar ln step 0");
    TH1D * gen_6 = stack_6.getSignalContainer().getTH1D("gen_6",false,false,false);
    gen_6->Write();


    f->Close();
    delete csv;
    return 0;
}

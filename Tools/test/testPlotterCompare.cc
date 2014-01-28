/*
 * testPlotterCompare.cc
 *
 *  Created on: Jan 14, 2014
 *      Author: kiesej
 */




#include "../interface/plotterCompare.h"

#include <vector>
#include "TRandom.h"
#include "../interface/plotterCompare.h"
#include "../interface/container.h"
#include "../interface/container2D.h"
#include "../interface/graph.h"
#include "TCanvas.h"
#include <iostream>
#include "../interface/containerStyle.h"
#include "../interface/plotStyle.h"
#include "../interface/fileReader.h"
#include "TFile.h"
#include "TStyle.h"

int main(){
    using namespace ztop;
    std::vector<float> bins;
    std::vector<float> otherbins;
    for(float i=0;i<4*21;i++)bins.push_back(i/4);

    for(float ji=0;ji<11;ji++)otherbins.push_back(ji*2);

    container2D c2d(bins,bins);

    container1D c1(bins),c2(bins),c3(bins),fdata(bins),cerr;
    TRandom * r=new TRandom(123);

    for(int j=0;j<100000;j++){
        c1.fill(r->Gaus(8,4));
        c2.fill(r->Gaus(8.1,3.9));
        c3.fill(r->Gaus(7,4.2));
        c2d.fill(r->Gaus(14,1),r->Gaus(15,3));
        if(j>950)
            fdata.fill(r->Gaus(15,3));
    }

    gStyle->SetOptStat(0);
    TCanvas *c=new TCanvas();//("","",400,1200);
   // plot::debug =true;
   // plotterCompare::debug=true;
   // textBoxes::debug=true;
    plotterCompare tp;
   // fileReader::debug=true;
    tp.usePad(c);
    tp.setTitle("testplotter Title");


    std::string stylePath="/afs/naf.desy.de/user/k/kieseler/recentProj/src/TtZAnalysis/Tools/styles/";

    tp.readStyleFromFile(stylePath+"comparePlots_3MC.txt");

    c1.setNames("c1 - nominal","xaxis[units]","yaxis[units]");
    c2.setNames("c2","xaxis[unit]","yaxis2[unit]");
    c3.setNames("c3","xaxis","yaxis3");

    c1.addErrorContainer("sys_up",c2*1.01);
    c1.addErrorContainer("sys_down",c2*0.99);
    container1D c4=(c3+c2)*0.5;
    c4.setName("c4");
    tp.setNominalPlot(&c1);
    tp.setComparePlot(&c2,0);
    tp.setComparePlot(&c3,1);
    tp.setComparePlot(&c4,2);

    TFile f("testPlotterCompare.root","RECREATE");

    tp.draw();
    c->Write();
    c->Print("testPlotterCompare.pdf");
    //tp.cleanMem();
    delete c;


    return 0;
}

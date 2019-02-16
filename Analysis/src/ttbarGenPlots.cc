/*
 * ttbarGenPlots.cc
 *
 *  Created on: Nov 1, 2013
 *      Author: kiesej
 */

#include "../interface/ttbarGenPlots.h"
#include "../interface/NTFullEvent.h"
#include "TtZAnalysis/DataFormats/interface/mathdefs.h"

#include "TtZAnalysis/DataFormats/src/classes.h"

#include <math.h>

namespace ztop{

/**
 * setbins, addplot, FILL
 *
 * NEVER restrict the addPlot statement by an (if) statement
 * 	-> SegFaults if plots are filled/init before variable is available.
 * Otherwise safe
 *
 */
void ttbarGenPlots::makeControlPlotsPriv(){




    SETBINS << 290 << 420 << 550 << 810 << 10000 ;
    addPlot("gen m_ttbar full", "#m_{tt}]","N_{l}/bw");
    if (event()->gentops){
        if (event()->gentops->size()>1){
            double mtt = (event()->gentops->at(0)->p4() + event()->gentops->at(1)->p4()).m();
            last()->fill(mtt ,*(event()->puweight));
        }
    }


    SETBINS << 290 << 420;
    addPlot("gen m_ttbar mtt1", "#m_{tt}]","N_{l}/bw");
    if (event()->gentops){
        if (event()->gentops->size()>1){
            double mtt = (event()->gentops->at(0)->p4() + event()->gentops->at(1)->p4()).m();
            last()->fill(mtt ,*(event()->puweight));
        }
    }


    SETBINS << 420 << 550;
    addPlot("gen m_ttbar mtt2", "#m_{tt}]","N_{l}/bw");
    if (event()->gentops){
        if (event()->gentops->size()>1){
            double mtt = (event()->gentops->at(0)->p4() + event()->gentops->at(1)->p4()).m();
            last()->fill(mtt ,*(event()->puweight));
        }
    }


    SETBINS << 550 << 810 ;
    addPlot("gen m_ttbar mtt3", "#m_{tt}]","N_{l}/bw");
    if (event()->gentops){
        if (event()->gentops->size()>1){
            double mtt = (event()->gentops->at(0)->p4() + event()->gentops->at(1)->p4()).m();
            last()->fill(mtt ,*(event()->puweight));
        }
    }


    SETBINSRANGE(9700,300,10000)
    addPlot("gen m_ttbar fine", "#m_{tt}]","N_{l}/bw");
    if (event()->gentops){
        if (event()->gentops->size()>1){
            double mtt = (event()->gentops->at(0)->p4() + event()->gentops->at(1)->p4()).m();
            last()->fill(mtt ,*(event()->puweight));
        }
    }

    SETBINSRANGE(5000,5,10)
    addPlot("gen m_ttbar ln", "#m_{tt}]","N_{l}/bw");
    if (event()->gentops){
        if (event()->gentops->size()>1){
            double mtt = (event()->gentops->at(0)->p4() + event()->gentops->at(1)->p4()).m();
            last()->fill(log(mtt) ,*(event()->puweight));
        }
    }


}

}//namespace


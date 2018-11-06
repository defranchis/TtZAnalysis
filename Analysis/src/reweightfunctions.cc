/*
 * reweightfunctions.cc
 *
 *  Created on: Dec 18, 2013
 *      Author: kiesej
 */

#include "TMath.h"
#include "../interface/reweightfunctions.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace ztop{

void reweightfunctions::setFunction(functiontype func){

   	type_=func;
   	if(type_==toppt){
   		std::cout << "reweightfunctions::setFunction: set to top pt" <<std::endl;
   	}
   	else if(type_==flat){
   		std::cout << "reweightfunctions::setFunction: set to flat (==1)" <<std::endl;
   	}
        if(type_==toppt_2016){
                std::cout << "reweightfunctions::setFunction: set to top pt" <<std::endl;
        }


}



void reweightfunctions::reWeight(const float& in, float& previousweight) {

    if(switchedoff_)
        return ;

    if(type_==flat){
        return ;
    }

    float newweight=1;

    if(type_ == toppt){
        if(syst_ == nominal)
            newweight= TMath::Exp(0.156-0.00137*in);
        if(syst_ == up)
            newweight= std::max(0.,1+(2.*(TMath::Exp(0.156-0.00137*in)-1)));
        if(syst_ == down)
            newweight= previousweight;
    }


    //base class calls
    setNewWeight(newweight);
    reWeight(previousweight);
}

void reweightfunctions::reWeight(const float& in1,const float& in2, float& previousweight) {
    if(switchedoff_)
        return;

    if(type_==flat){
        return;
    }
    float newweight=1;

    //this is the sqrt approach
    if(type_ == toppt){
        if(syst_ == nominal)
            newweight= sqrt(  TMath::Exp(0.156-0.00137*in1)  * TMath::Exp(0.156-0.00137*in2) );
        if(syst_ == up)
            newweight= sqrt( std::max(0.,1+(2.*(TMath::Exp(0.156-0.00137*in1)-1)))* std::max(0.,1+(2.*(TMath::Exp(0.156-0.00137*in2)-1))) );
        if(syst_ == down)
            newweight= previousweight;
    }

    if(type_ == toppt_2016){
        if(syst_ == nominal)
            newweight= sqrt(  TMath::Exp(0.0615-0.0005*in1)  * TMath::Exp(0.0615-0.0005*in2) );
        if(syst_ == up)
            newweight= sqrt( std::max(0.,1+(2.*(TMath::Exp(0.0615-0.0005*in1)-1)))* std::max(0.,1+(2.*(TMath::Exp(0.0615-0.0005*in2)-1))) );
        if(syst_ == down)
            newweight= previousweight;
    }

    //base class calls
    setNewWeight(newweight);
    reWeight(previousweight);
}


}

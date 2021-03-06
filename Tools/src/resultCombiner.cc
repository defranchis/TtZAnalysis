/*
 * resultCombiner.c
 *
 *  Created on: Aug 19, 2014
 *      Author: kiesej
 */



#include "../interface/resultCombiner.h"

namespace ztop{

bool resultCombiner::debug=false;

void resultCombiner::addInput(const histo1D& cont){
	if(distributions_.size()>0){
		if(distributions_.at(0).bins() != cont.getBins()){
			throw std::logic_error("resultCombiner::addInput: inputs must have same binning");
		}
		if(!temp_.hasSameLayerOrdering(cont)){
			throw std::logic_error("resultCombiner::addInput: inputs must have same systematics and ordering!");
		}
	}
	initialinputs_.push_back(cont);
	temp_=cont;
	histo1D contcp=cont;
	binsstart_=cont.getSystNameList().size();
	for(size_t i=0;i<contcp.getBins().size();i++)
		contcp.addGlobalRelError("bins_"+toTString(i),0); //fake variations foe each bin
	variateHisto1D temp;

	temp.import(contcp);

	distributions_.push_back(temp);
	sysforms_.resize(distributions_.at(0).getNDependencies(), rc_sysf_gaus);
	matrix m(distributions_.at(0).bins().size(),distributions_.at(0).bins().size());
	m.setDiagonal();
	statcorrelations_.push_back(m);
}

void resultCombiner::addInput(const graph&g ){
	histo1D h;
	h.import(g);
	addInput(h);
}

void resultCombiner::setSystForm(const TString& sys,rc_sys_forms form){
	if(distributions_.size()<1){
		throw std::logic_error("resultCombiner::setSystForm: first fill input distributions!");
	}


	std::vector<TString> sysnames=distributions_.at(0).getSystNames();
	for(size_t i=0;i<sysnames.size();i++){
		if(sysnames.at(i) == sys){
			sysforms_.at(i) = form;
			return;
		}
	}
	throw std::logic_error("resultCombiner::setSystForm: systematics name not found!");

}


void resultCombiner::reset(){

}

bool resultCombiner::minimize(){
	if(distributions_.size()<1)
		return false;
	//configure fitter
	std::vector<double> startparas(distributions_.at(0).getNDependencies() ,0);
	std::vector<TString> paranames=distributions_.at(0).getSystNames();

	//make sure to treat UF OF properly

	double integral=0;
	//add mean values as parameters to be extracted:
	for(size_t i=0;i<distributions_.at(0).bins().size();i++){ //no UF OF
		double mean=0;
		for(size_t j=0;j<distributions_.size();j++){
			mean+=distributions_.at(j).getBin(i)->getNominal();

		}
		mean/=(double)distributions_.size();
		//	std::cout << "mean in bin " << i << ": " << mean << std::endl;
		integral+= distributions_.at(0).getBin(i)->getNominal();
		startparas.at(binsstart_+i)=mean;
		if(debug)
			std::cout << "resultCombiner::minimize: setting start value for bin " << i << " " << mean<<std::endl;
	}

	forcednorm_=integral;

	std::vector<double> stepwidths;
	stepwidths.resize(startparas.size(),1);

	fitter_.setParameterNames(paranames);
	fitter_.setRequireFitFunction(false);
	fitter_.setParameters(startparas,stepwidths);

	fitter_.setMinimizer(simpleFitter::mm_minuit2);
	fitter_.setMaxCalls(4e5);
	//fitter_.setTolerance(0.5);//0.1);
	//first do a simple scan


	ROOT::Math::Functor f(this,&resultCombiner::getChi2,fitter_.getParameters()->size());
	for(size_t i=0;i<sysforms_.size();i++){
		if(sysforms_.at(i) == rc_sysf_box){
			fitter_.setParameterUpperLimit(i,1);
			fitter_.setParameterLowerLimit(i,-1);
		}
	}
	fitter_.setMinFunction(&f);
	fitter_.setStrategy(0);
	fitter_.setTolerance(1);
	fitter_.fit();
	if(fitter_.wasSuccess())
		fitter_.feedErrorsToSteps();
	fitter_.setStrategy(2);
	fitter_.setTolerance(0.1);

	std::vector<size_t> minoserrs;
	if(combminos_){
		for(size_t i=binsstart_; i< startparas.size();i++)
			fitter_.setAsMinosParameter(i,true);
	}


	fitter_.fit();



	//feed back

	std::vector<double> fittedbins(fitter_.getParameters()->begin() + binsstart_, fitter_.getParameters()->end());
	std::vector<double> fittederrsup(fitter_.getParameterErrUp()->begin() + binsstart_, fitter_.getParameterErrUp()->end());
	std::vector<double> fittederrsdown(fitter_.getParameterErrDown()->begin() + binsstart_, fitter_.getParameterErrDown()->end());

	std::vector<float> bins = distributions_.at(0).bins();
	bins.erase(bins.begin());

	combined_=histo1D(bins);
	combined_.setName("combined");
	combined_.addGlobalRelErrorUp("combined_err",0);
	combined_.addGlobalRelErrorDown("combined_err",0);

	for(size_t i=0;i<combined_.getBins().size();i++){
		combined_.setBinContent(i,fittedbins.at(i));

		combined_.setBinContent(i,fittedbins.at(i)+fittederrsup.at(i),0);
		combined_.setBinContent(i,fittedbins.at(i)+fittederrsdown.at(i),1);
	}
	return fitter_.wasSuccess();

}
void resultCombiner::produceWeightedMean(){
	if(initialinputs_.size()<1)
		throw std::logic_error("resultCombiner::produceWeightedMean: first read input");

	size_t nbins=initialinputs_.at(0).getBins().size();
	int nsys=initialinputs_.at(0).getSystSize();
	histo1D output=initialinputs_.at(0);
	for(size_t bin=0;bin<nbins;bin++){
		for(int sys=-1;sys<nsys;sys++){
			double weightsum=0;
			double weightedsum=0;
			for(size_t hist=0;hist<initialinputs_.size();hist++){
				const float & content=initialinputs_.at(hist).getBinContent(bin,sys);
				double weight=0;
				if(initialinputs_.at(hist).getBin(bin).getStat2()!=0)
					weight = 1/initialinputs_.at(hist).getBin(bin).getStat2();
				weightedsum+=content*weight;
				weightsum+=weight;
			}
			output.setBinContent(bin, weightedsum/weightsum,sys);
			if(sys<0)
				output.setBinStat(bin,1/weightsum);
			else
				output.setBinStat(bin,0,sys);
		}
	}
	combined_= output;
}

void resultCombiner::coutSystBreakDownInBin(size_t idx)const{
	if(distributions_.size()<1) return;
	if(combined_.isDummy()) return;


	float avgerror=(fabs(combined_.getBinErrorUp(idx,false))+fabs(combined_.getBinErrorDown(idx,false)))/2;
	float content=combined_.getBinContent(idx);

	std::cout << "para val\tpara err\tpara name\tcontr"<<std::endl;
	for(size_t i=0;i<getFitter()->getParameters()->size() - combined_.getBins().size();i++){
		float breakdown=0;
		breakdown=sqrt(sq(avgerror)-sq(avgerror * sqrt(1-sq(getFitter()->getCorrelationCoefficient(binsstart_+idx,i)))));
		breakdown/=content;
		breakdown*=100;
		std::cout  << getFitter()->getParameters()->at(i) << "\t"<< getFitter()->getParameterErr(i) << "\t"
				<<getFitter()->getParameterNames()->at(i)<< "\t"<< breakdown<<"%" <<std::endl;

	}


}


double resultCombiner::getNuisanceLogGaus(const double & in)const{
	return fitter_.nuisanceGaus(in);
}
double resultCombiner::getNuisanceLogBox(const double & in)const{
	return fitter_.nuisanceBox(in);
}
//is it ok to scale the errors here?
double resultCombiner::getNuisanceLogNormal(const double & in)const{
	return fitter_.nuisanceLogNormal(in);
}




double resultCombiner::getChi2(const double * variations){
	double chi2=0;
	const double * combbincontents=variations;
	combbincontents += binsstart_;

	//for norm use  forcednorm_ later and for last bin = forcednorm_-allbinssum
	for(size_t meas=0;meas<distributions_.size();meas++){
		double binsum=0;
		for(size_t i=0;i<distributions_.at(0).bins().size();i++){
			double binerror=distributions_.at(meas).getBinErr(i);
			if(binerror==0){
				chi2+=sq(combbincontents [i]*100); //let them go to zero
				continue;
			}
			if(additionalconstraint_==rc_addc_normalized){
				if(i<distributions_.at(0).bins().size()-1){
					chi2+= sq( combbincontents [i] - distributions_.at(meas).getBin(i)->getValue(variations))
																			/ sq(binerror); //this is gaussian
					binsum+=distributions_.at(meas).getBin(i)->getValue(variations);
				}
				else{
					chi2+= sq( combbincontents [i] - binsum)
																			/ sq(binerror); //this is gaussian
				}
			}
			else{
				chi2+= sq( combbincontents [i] -
						distributions_.at(meas).getBin(i)->getValue(variations)) /
								sq(binerror) ; //this is gaussian
			}
		}
	}

	//	return chi2;
	//nuisance parameters
	for(size_t i=0;i<binsstart_;i++){
		if(sysforms_.at(i) == rc_sysf_gaus)
			chi2+=sq(variations[i]);
		//replaced by limits
		//	else if(sysforms_.at(i) == rc_sysf_box)
		chi2+=0.01*sq(variations[i]);
	}
	return chi2;
}


}//ns

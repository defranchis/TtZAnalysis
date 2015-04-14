/*
 * variateHisto1D.cc
 *
 *  Created on: Aug 19, 2014
 *      Author: kiesej
 */

#include "../interface/variateHisto1D.h"

namespace ztop{

/*variateHisto1D(){}
	~variateHisto1D(){}

	void import(const histo1D&);

	//exports for a certain set of variations
	histo1D exportContainer(const std::vector<float> & variations);

private:
	std::vector<extendedVariable> contents_;
	std::vector<float> bins_;
 *
 */

bool variateHisto1D::debug=false;

void variateHisto1D::import(const histo1D& cont){

	if(debug)
		std::cout << "variateContainer1D::import" <<std::endl;

	contents_.clear();errsup_.clear(),errsdown_.clear();
	contents_.resize(cont.getBins().size());
	errsup_.resize(cont.getBins().size());
	errsdown_.resize(cont.getBins().size());
	bins_=cont.getBins();
	std::vector<TString> variations=cont.getSystNameList();

	for(size_t sys=0;sys<variations.size();sys++){
		const TString& sysname=variations.at(sys);
		size_t idxdown=cont.getSystErrorIndex(sysname+"_down");
		size_t idxup=cont.getSystErrorIndex(sysname+"_up");
		for(size_t bin=0;bin<bins_.size();bin++){
			const float & content=cont.getBinContent(bin);
			const float & sysup=cont.getBinContent(bin,idxup);
			const float & sysdown=cont.getBinContent(bin,idxdown);

			contents_.at(bin).addDependence(sysdown,content ,sysup ,sysname);
			if(debug)
				std::cout << "variateContainer1D::import added: " << sysdown << " " << content << " " << sysup << " "<< sysname << std::endl;

		}
	}
	if(variations.size() == 0){
		for(size_t bin=0;bin<bins_.size();bin++){
			const float & content=cont.getBinContent(bin);
			contents_.at(bin).addDependence(content,content ,content ,"dummy");
		}
	}
	//only stat!
	for(size_t bin=0;bin<bins_.size();bin++){
		if(debug){
			std::cout << "variateContainer1D::import adding error: "<<bin << " " <<  cont.getBinErrorUp(bin,true) <<std::endl;
		}
		errsup_.at(bin) = cont.getBinErrorUp(bin,true);
		errsdown_.at(bin) = cont.getBinErrorDown(bin,true);
	}



}


void variateHisto1D::setBinErrUp(size_t bin,const float& err){
	if(debug)
		std::cout << "variateContainer1D::setBinErrUp" <<std::endl;
	if(bin<=bins_.size())
		throw std::out_of_range("variateContainer1D::setBinErrUp: bin out of range");
	errsup_.at(bin)=err;
}
void variateHisto1D::setBinErrDown(size_t bin,const float& err){
	if(debug)
		std::cout << "variateContainer1D::setBinErrDown" <<std::endl;
	if(bin<=bins_.size())
		throw std::out_of_range("variateContainer1D::setBinErrDown: bin out of range");
	errsdown_.at(bin)=err;
}

double variateHisto1D::getIntegral(const double * vars)const{
	double out=0;

	for(size_t i=0;i<bins_.size();i++)
		out+=getBin(i)->getValue(vars);
	return out;
}
extendedVariable variateHisto1D::getIntegral()const{
	extendedVariable out;
	if(bins_.size()<1){
		throw std::logic_error("variateHisto1D::getIntegral: histogram has no bins");
	}
	std::vector<TString> names=contents_.at(0).getSystNames();
	std::vector<double> vars(names.size(),0);
	for(size_t sys=0;sys<names.size();sys++){
		double integralnom=0,integralup=0,integraldown=0;
		for(size_t i=0;i<bins_.size();i++){
			integralnom+=contents_.at(i).getValue(vars);
		}
		vars.at(sys)=+1;
		for(size_t i=0;i<bins_.size();i++){
			integralup+=contents_.at(i).getValue(vars);
		}
		vars.at(sys)=-1;
		for(size_t i=0;i<bins_.size();i++){
			integraldown+=contents_.at(i).getValue(vars);
		}
		vars.at(sys)=0;
		out.addDependence(integraldown,integralnom,integralup,names.at(sys));
	}
	return out;
}

variateHisto1D& variateHisto1D::operator *= (const variateHisto1D&rhs){
	checkCompat(rhs);
	for(size_t i=0;i<contents_.size();i++){
		contents_.at(i)*=rhs.contents_.at(i);
	}
	return *this;
}
variateHisto1D variateHisto1D::operator * (const variateHisto1D&rhs)const{
	variateHisto1D cp=*this;
	return cp*=rhs;
}

variateHisto1D& variateHisto1D::operator += (const variateHisto1D&rhs){
	checkCompat(rhs);
	for(size_t i=0;i<contents_.size();i++){
		contents_.at(i)+=rhs.contents_.at(i);
	}
	return *this;
}

variateHisto1D variateHisto1D::operator + (const variateHisto1D&rhs)const{
	variateHisto1D cp=*this;
	return cp+=rhs;
}

variateHisto1D& variateHisto1D::operator -= (const variateHisto1D&rhs){
	checkCompat(rhs);
	for(size_t i=0;i<contents_.size();i++){
		contents_.at(i)-=rhs.contents_.at(i);
	}
	return *this;
}

variateHisto1D variateHisto1D::operator - (const variateHisto1D&rhs)const{
	variateHisto1D cp=*this;
	return cp-=rhs;
}

variateHisto1D& variateHisto1D::operator /= (const variateHisto1D&rhs){
	checkCompat(rhs);
	for(size_t i=0;i<contents_.size();i++){
		contents_.at(i)/=rhs.contents_.at(i);
	}
	return *this;
}

variateHisto1D variateHisto1D::operator / (const variateHisto1D&rhs)const{
	variateHisto1D cp=*this;
	return cp/=rhs;
}


variateHisto1D& variateHisto1D::operator *= (const extendedVariable&v){
	for(size_t i=0;i<contents_.size();i++){
		contents_.at(i)*=v;
	}
	return *this;
}

variateHisto1D variateHisto1D::operator * (const extendedVariable&v)const{
	variateHisto1D cp=*this;
	return cp*=v;
}

variateHisto1D& variateHisto1D::operator /= (const extendedVariable&v){
	for(size_t i=0;i<contents_.size();i++){
		contents_.at(i)/=v;
	}
	return *this;
}

variateHisto1D variateHisto1D::operator / (const extendedVariable&v)const{
	variateHisto1D cp=*this;
	return cp/=v;
}



variateHisto1D& variateHisto1D::operator *= (const double&v){
	for(size_t i=0;i<contents_.size();i++){
		contents_.at(i)*=v;
	}
	return *this;
}
variateHisto1D variateHisto1D::operator * (const double&v)const{
	variateHisto1D cp=*this;
	return cp*=v;
}

variateHisto1D& variateHisto1D::operator /= (const double&v){
	for(size_t i=0;i<contents_.size();i++){
		contents_.at(i)/=v;
	}
	return *this;
}
variateHisto1D variateHisto1D::operator / (const double&v)const{
	variateHisto1D cp=*this;
	return cp/=v;
}


void variateHisto1D::checkCompat(const variateHisto1D& rhs)const{
	if(bins_!=rhs.bins_)
		throw std::out_of_range("variateHisto1D::checkCompat: bins dont match");
	if(bins_.size()<1 || contents_.at(0).getNDependencies() !=  rhs.contents_.at(0).getNDependencies() )
		throw std::out_of_range("variateHisto1D::checkCompat: n dependencies dont match");
}

histo1D variateHisto1D::exportContainer()const{
	if(contents_.size()<1){
		throw std::out_of_range("variateContainer1D::exportContainer: cannot export without input!");
	}
	std::vector<double> var;
	var.resize(contents_.at(0).getNDependencies(),0.);
	return exportContainer(var);

}

histo1D variateHisto1D::exportContainer(const std::vector<double> & variations)const{
	if(debug)
		std::cout << "variateContainer1D::exportContainer" <<std::endl;
	if(contents_.size()<1){
		throw std::out_of_range("variateContainer1D::exportContainer: cannot export without input!");
	}
	std::vector<float> bins=bins_;
	bins.erase(bins.begin()); //let UF be created for output
	histo1D out(bins);
	histo1D zerotmp=out;

	for(size_t i=0;i<bins_.size();i++){
		out.setBinContent(i,contents_.at(i).getValue(variations)  );
		out.setBinStat(i,(errsup_.at(i)+errsdown_.at(i))/2);
	}


	return out;
}
histo1D variateHisto1D::exportContainer(const std::vector<double> & variations,
		const std::vector<double> & symm_constraints,const std::vector<TString> & varnames)const{
	if(varnames.size() != variations.size() || symm_constraints.size()!=variations.size())
		throw std::out_of_range("variateHisto1D::exportContainer: sizes downt match");

	histo1D nominal=exportContainer(variations);
	std::vector<double> varcp=variations;
	for(size_t i=0;i<varnames.size();i++){
		TString name=varnames.at(i)+"_up";
		varcp.at(i)+=symm_constraints.at(i);
		histo1D err=exportContainer(varcp);
		nominal.addErrorContainer(name,err);
		name=varnames.at(i)+"_down";
		varcp.at(i)=variations.at(i);
		varcp.at(i)-=symm_constraints.at(i);
		err=exportContainer(varcp);
		nominal.addErrorContainer(name,err);
		varcp.at(i)=variations.at(i);//set back to default
	}
	return nominal;

}

}


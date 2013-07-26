#include "../interface/containerStack.h"

namespace ztop{

  std::vector<ztop::container1DStack*> container1DStack::cs_list;
  bool container1DStack::cs_makelist=false;
  bool container1DStack::batchmode=true;

  container1DStack::container1DStack(){
    if(cs_makelist)cs_list.push_back(this);
  }
  container1DStack::container1DStack(TString name) : name_(name), dataleg_("data") {
    if(cs_makelist) cs_list.push_back(this);
  }
  container1DStack::~container1DStack(){ 
    for(unsigned int i=0;i<cs_list.size();i++){
      if(cs_list[i] == this) cs_list.erase(cs_list.begin()+i);
      break;
    }
  }
  void container1DStack::push_back(ztop::container1D cont, TString legend, int color, double norm){
   bool wasthere=false;
   for(unsigned int i=0;i<legends_.size();i++){
     if(legend == legends_[i]){
       if(container1D::debug)
	 std::cout << "container1DStack::push_back: found same legend ("<<  legend <<"), adding " << cont.getName() << std::endl;
       containers_[i] = containers_[i] * norms_[i] + cont * norm;
       containers_[i].setName("c_" +legend);
       norms_[i]=1;
       wasthere=true;
       break;
     }
   }
   if(!wasthere){
     cont.setName("c_" +legend);
     containers_.push_back(cont);
     legends_.push_back(legend);
     colors_.push_back(color);
     norms_.push_back(norm);
   }
 }
  void container1DStack::removeContribution(TString legendname){
    bool found=false;
    std::vector<TString>::iterator leg=legends_.begin();
    std::vector<int>::iterator col=colors_.begin();
    std::vector<double>::iterator norm=norms_.begin();
    
    for(std::vector<ztop::container1D>::iterator cont=containers_.begin();cont<containers_.end();++cont){
      if(*leg == legendname){
	legends_.erase(leg);
	containers_.erase(cont);
	colors_.erase(col);
	norms_.erase(norm);
	found=true;
	break;
      }
      ++leg;++col;++norm;
      for(std::map<TString,size_t>::iterator lego=legorder_.begin(); lego!=legorder_.end();++lego){
	if(lego->first == legendname)
	  legorder_.erase(lego);
      }
    }
    if(!found) std::cout << "container1DStack::removeContribution: " << legendname << " not found." <<std::endl;

  }

 void container1DStack::mergeSameLegends(){
   //redundant
 }

 ztop::container1D container1DStack::getContribution(TString contr){
   ztop::container1D out;
   if(containers_.size() > 0){
     out=containers_[0];
     out.clear();
     int i=0;
     bool notfound=true;
     for(std::vector<TString>::iterator name=legends_.begin();name<legends_.end();++name){
       if(contr == *name){
	 out=out + containers_[i] * norms_[i] ;
	 notfound=false;
	 break;
       }
       i++;
     }
     if(notfound) std::cout << "container1DStack::getContribution: " << contr << " not found!" <<std::endl;
   }
   
   return out;
 }
  ztop::container1D container1DStack::getContributionsBut(TString contr){
   ztop::container1D out;
   if(containers_.size() > 0){
     out=containers_[0];
     out.clear();
     int i=0;
     for(std::vector<TString>::iterator name=legends_.begin();name<legends_.end();++name){
       if(contr != *name) out=out+containers_[i] * norms_[i];
     i++;
     }
   }
   return out;
 }

  ztop::container1D container1DStack::getContributionsBut(std::vector<TString> contr){
    ztop::container1D out;
    if(containers_.size() > 0){
      out=containers_[0];
      out.clear();
      for(unsigned int i=0;i<legends_.size();i++){
	bool get=true;
	for(std::vector<TString>::iterator name=contr.begin();name<contr.end();++name){
	  if(*name == legends_[i]){
	    get=false;
	    break;
	  }
	}
	if(get) out = out+ containers_[i] * norms_[i];
      }
    }
    return out;
  }

  
  void container1DStack::multiplyNorm(TString legendentry, double multi){
    int i=0;
    for(std::vector<TString>::iterator name=legends_.begin();name<legends_.end();++name){
      if(legendentry == *name){
	norms_[i] = norms_[i] * multi;
      }
      i++;
    }
  }

  void container1DStack::multiplyAllMCNorms(double multiplier){
    for(unsigned int i=0;i<norms_.size();i++){
      if(legends_.at(i)!=dataleg_)
	norms_.at(i)=norms_.at(i) * multiplier;
    }
  }

  void container1DStack::addGlobalRelMCError(TString sysname,double error){
    for(unsigned int i=0;i<containers_.size();i++){
      if(legends_[i]!=dataleg_) containers_[i].addGlobalRelError(sysname,error);
    }
  }
  void container1DStack::addMCErrorStack(TString sysname,container1DStack errorstack, bool ignoreMCStat){
    for(unsigned int i=0; i<size();i++){
      for(unsigned int j=i;j<errorstack.size();j++){
	errorstack.containers_[j] = errorstack.containers_[j] * errorstack.norms_[j]; //normalize (in case there is any remultiplication done or something)
	errorstack.norms_[j]=1;
	if(legends_[i] == errorstack.legends_[j] && legends_[i]!=dataleg_){
	  containers_[i] = containers_[i] * norms_[i];
	  norms_[i]=1;
	  containers_[i].addErrorContainer(sysname,errorstack.containers_[j],ignoreMCStat);
	}
      }
    }
  }

  void container1DStack::addRelSystematicsFrom(ztop::container1DStack stack){
    for(std::vector<ztop::container1D>::iterator cont=stack.containers_.begin();cont<stack.containers_.end();++cont){
      TString name=cont->getName();
      for(unsigned int i=0;i<containers_.size();i++){
	if(containers_[i].getName() == name){
	  containers_[i].addRelSystematicsFrom(*cont);
	  break;
	}
      }
    }
  }

  void container1DStack::removeError(TString sysname){
    for(unsigned int i=0; i<size();i++){
      containers_[i].removeError(sysname);
    }
  }

  void container1DStack::renameSyst(TString old, TString New){
    for(unsigned int i=0; i<size();i++){
      containers_[i].renameSyst(old,New);
    }
  }
  ztop::container1D & container1DStack::getContainer(TString name){
    bool found=false;
    for(unsigned int i=0;i<containers_.size();i++){
      if(containers_[i].getName() == name){
	containers_[i] = containers_[i] * norms_[i];
	norms_[i] = 1.;
	found=true;
	return containers_[i];
      }
    }
    if(!found) std::cout << "container1DStack::getContainer: container with name " << name << " not found, returning first container! all names are formatted: c_legendentry" << std::endl;
    return containers_[0];;
  }
  ztop::container1D container1DStack::getFullMCContainer(){
    container1D out=containers_[0];
    out.clear();
    for(unsigned int i=0;i<containers_.size();i++){
      if(legends_[i] != dataleg_) out = out + containers_[i]*norms_[i];
    }
    return out;
  }

  THStack * container1DStack::makeTHStack(TString stackname){
    if(stackname == "") stackname = name_+"_s";
    THStack *tstack = new THStack(stackname,stackname);

    std::vector<size_t> sorted=sortEntries(false);

    for(unsigned int it=0;it<size();it++){
    	size_t i=sorted.at(it);
    	if(getLegend(i) != dataleg_){
    		container1D tempcont = getContainer(i);
    		tempcont = tempcont * getNorm(i);
    		TH1D * h = (TH1D*)tempcont.getTH1D(getLegend(i)+" "+getName()+"_stack_h")->Clone();
    		h->SetFillColor(getColor(i));
    		for(int bin = 1 ; bin < h->GetNbinsX()+1;bin++){
    			h->SetBinError(bin,0);
    		}
    		tstack->Add(h);
    	}
    }
    return  tstack;
  }
  TLegend * container1DStack::makeTLegend(bool inverse){
    TLegend *leg = new TLegend((Double_t)0.65,(Double_t)0.50,(Double_t)0.95,(Double_t)0.90);
    leg->Clear();
    leg->SetFillStyle(0);
    leg->SetBorderSize(0);

    std::vector<size_t> sorted=sortEntries(inverse);

    for(unsigned int it=0;it<size();it++){
      size_t i=sorted.at(it);
      container1D tempcont = getContainer(i);
      tempcont = tempcont * getNorm(i);
      TH1D * h = (TH1D*)tempcont.getTH1D(getLegend(i)+" "+getName()+"_leg")->Clone();
      h->SetFillColor(getColor(i));
      if(getLegend(i) != dataleg_) leg->AddEntry(h,getLegend(i),"f");
      else leg->AddEntry(h,getLegend(i),"ep");
      //  delete h;
    }
    
    return leg;
  }
  void container1DStack::drawControlPlot(TString name, bool drawxaxislabels, double resizelabels){
    if(name=="") name=name_;
    int dataentry=0;
    bool gotdentry=false;
    bool gotuf=false;
    bool gotof=false;
    for(unsigned int i=0;i<size();i++){ // get datalegend and check if underflow or overflow in any container
      if(getLegend(i) == dataleg_){
        dataentry=i;
	gotdentry=true;
	if(gotof && gotuf) break;
      }
      if(containers_[i].getOverflow() < -0.9){
	gotof=true;
	if(gotdentry && gotuf) break;
      }
      if(containers_[i].getUnderflow() < -0.9){
	gotuf=true;
	if(gotdentry && gotof) break;
      }
    }
    float multiplier=1;
    double ylow,yhigh,xlow,xhigh;
    if(gPad){
      gPad->GetPadPar(xlow,ylow,xhigh,yhigh);
      multiplier = (float)1/(yhigh-ylow);
    }
    containers_[dataentry].setLabelSize(resizelabels * multiplier);
    TGraphAsymmErrors * g = containers_[dataentry].getTGraph(name);
    TH1D * h =containers_[dataentry].getTH1D(name+"_h"); // needed to be able to set log scale etc.

    float xmax=containers_[dataentry].getXMax();
    float xmin=containers_[dataentry].getXMin();
    h->Draw("AXIS");
    if(!drawxaxislabels){
      h->GetXaxis()->SetLabelSize(0);
    }
    else {
      if(gPad){
	gPad->SetLeftMargin(0.15);
	gPad->SetBottomMargin(0.15);
      }
    }
    g->Draw("P");
    makeTHStack(name)->Draw("same");
    makeMCErrors()->Draw("2");
    makeTLegend()->Draw("same");
    g->Draw("e1,P,same");
    if(gPad) gPad->RedrawAxis();

    if(gPad && drawxaxislabels && containers_[dataentry].getNBins() >0){
      float yrange=fabs(gPad->GetUymax()-gPad->GetUymin());
      float xrange = fabs(xmax-xmin);
      if(gotuf){
	TLatex * la = new TLatex(containers_[dataentry].getBinCenter(1)-xrange*0.06,gPad->GetUymin()-0.15*yrange,"#leq");
	la->Draw("same");
      }
      if(gotof){
	TLatex * la2 = new TLatex(containers_[dataentry].getBinCenter(containers_[dataentry].getNBins())+xrange*0.06,gPad->GetUymin()-0.15*yrange,"#leq");
	la2->Draw("same");
      }
    }

  }

  TGraphAsymmErrors * container1DStack::makeMCErrors(){
    TGraphAsymmErrors * g =getFullMCContainer().getTGraph();
    g->SetFillStyle(3005);
    return g;
  }

  void container1DStack::drawRatioPlot(TString name,double resizelabels){
    //prepare container
    if(name=="") name=name_;
    int dataentry=0;
    bool gotdentry=false;
    bool gotuf=false;
    bool gotof=false;
    for(unsigned int i=0;i<size();i++){ // get datalegend and check if underflow or overflow in any container
      if(getLegend(i) == dataleg_){
        dataentry=i;
	gotdentry=true;
	if(gotof && gotuf) break;
      }
      if(containers_[i].getOverflow() < -0.9){
	gotof=true;
	if(gotdentry && gotuf) break;
      }
      if(containers_[i].getUnderflow() < -0.9){
	gotuf=true;
	if(gotdentry && gotof) break;
      }
    }
    container1D data = containers_[dataentry];
    data.setShowWarnings(false);   
    container1D mc = getFullMCContainer();
    mc.setShowWarnings(false);   
    data.setDivideBinomial(false);
    mc.setDivideBinomial(false);
    container1D ratio=data;
    ratio.clear(); 
    ratio.setShowWarnings(false);   
    container1D relmcerr=data;
    relmcerr.clear();
    container1D mccopy=mc;
    for(int i=0; i<ratio.getNBins()+1; i++){ //includes underflow right now, doesn't matter
      relmcerr.setBinContent(i,1.);
      relmcerr.setBinErrorUp(i,mc.getBinErrorUp(i) / mc.getBinContent(i));
      relmcerr.setBinErrorDown(i,mc.getBinErrorDown(i) / mc.getBinContent(i));
      //set mc error to zero for the ratio plot
      mccopy.setBinErrorUp(i,0);
      mccopy.setBinErrorDown(i,0);
    }
    ratio = data / mccopy;
    float multiplier=1;
    double ylow,yhigh,xlow,xhigh;
    if(gPad){
      gPad->GetPadPar(xlow,ylow,xhigh,yhigh);
      multiplier = (float)1/(yhigh-ylow);
    }
    //   multiplier = multiplier * resizelabels;
    ratio.setLabelSize(multiplier * resizelabels);

    TGraphAsymmErrors * gratio = ratio.getTGraph(name,false,false,true); //don't divide by binwidth, no x errors
    // rescale axis titles etc.
    TH1D * h = ratio.getTH1D(name+"_h_r",false); //no bw div
    h->GetYaxis()->SetTitle("data/MC");
    h->GetYaxis()->SetRangeUser(0.5,1.5);
    h->GetYaxis()->SetNdivisions(505);
    h->GetXaxis()->SetTickLength(0.03 * multiplier);
    h->Draw("AXIS");
    gratio->Draw("P");
    TGraphAsymmErrors * gmcerr = relmcerr.getTGraph(name+"_relerr",false,false,false);
    gmcerr->SetFillStyle(3005);
    gmcerr->Draw("2,same");
    TLine * l = new TLine(mc.getXMin(),1,mc.getXMax(),1);
    l->Draw("same");
    float xmax=containers_[dataentry].getXMax();
    float xmin=containers_[dataentry].getXMin();
    if(gPad && containers_[dataentry].getNBins() >0){
      float yrange=fabs(gPad->GetUymax()-gPad->GetUymin());
      float xrange = fabs(xmax-xmin);
      if(gotuf){
	TLatex * la = new TLatex(containers_[dataentry].getBinCenter(1)-xrange*0.06,gPad->GetUymin()-0.15*yrange,"#leq");
	la->SetTextSize(la->GetTextSize() * multiplier * resizelabels);
	la->Draw("same");
      }
      if(gotof){
	TLatex * la2 = new TLatex(containers_[dataentry].getBinCenter(containers_[dataentry].getNBins())+xrange*0.06,gPad->GetUymin()-0.15*yrange,"#leq");
	la2->SetTextSize(la2->GetTextSize() * multiplier * resizelabels);
	la2->Draw("same");
      }
    }
  }

  TCanvas * container1DStack::makeTCanvas(bool drawratioplot){
    TCanvas * c = new TCanvas(name_+"_c",name_+"_c");
    if(container1DStack::batchmode)
      c->SetBatch();
    TH1D * htemp=new TH1D("sometemphisto"+name_,"sometemphisto"+name_,2,0,1); //creates all gPad stuff etc and prevents seg vio, which happens in some cases; weird
    htemp->Draw();
    if(!drawratioplot){
      c->cd();
      drawControlPlot();
    }
    else{
      double labelresize=1.;
      double divideat=0.3;
      c->Divide(1,2);
      c->SetFrameBorderSize(0);
      c->cd(1)->SetLeftMargin(0.15);
      c->cd(1)->SetBottomMargin(0.03);
      c->cd(1)->SetPad(0,divideat,1,1);
      drawControlPlot(name_,false,labelresize);
      c->cd(2)->SetBottomMargin(0.4 * 0.3/divideat);
      c->cd(2)->SetLeftMargin(0.15);
      c->cd(2)->SetTopMargin(0);
      c->cd(2)->SetPad(0,0,1,divideat);
      gStyle->SetOptTitle(0);
      drawRatioPlot("",labelresize);
      c->cd(1)->RedrawAxis();
    }
    if(gStyle){
       gStyle->SetOptTitle(0);
       gStyle->SetOptStat(0);
    }
    delete htemp;
    return c;
  }

  //just perform functions on the containers with same names
  ztop::container1DStack container1DStack::operator + (container1DStack stack){
    for(unsigned int i=0;i<containers_.size();i++){
      for(unsigned int j=0;j<stack.containers_.size();j++){
	if(containers_[i].getName() == stack.containers_[j].getName()){
	  stack.containers_[j] = containers_[i] + stack.containers_[j];
	}
      }
    }
    return stack;
  }
  ztop::container1DStack container1DStack::operator - (container1DStack stack){
    for(unsigned int i=0;i<containers_.size();i++){
      for(unsigned int j=0;j<stack.containers_.size();j++){
	if(containers_[i].getName() == stack.containers_[j].getName()){
	  stack.containers_[j] = containers_[i] - stack.containers_[j];
	}
      }
    }
    return stack;
  }
  ztop::container1DStack container1DStack::operator / (container1DStack  stack){
    for(unsigned int i=0;i<containers_.size();i++){
      for(unsigned int j=0;j<stack.containers_.size();j++){
	if(containers_[i].getName() == stack.containers_[j].getName()){
	  stack.containers_[j] = containers_[i] / stack.containers_[j];
	}
      }
    }
    return stack;
  }
  ztop::container1DStack container1DStack::operator * (container1DStack  stack){
    for(unsigned int i=0;i<containers_.size();i++){
      for(unsigned int j=0;j<stack.containers_.size();j++){
	if(containers_[i].getName() == stack.containers_[j].getName()){
	  stack.containers_[j] = containers_[i] * stack.containers_[j];
	}
      }
    }
    return stack;
  }
  ztop::container1DStack container1DStack::operator * (double scalar){
    ztop::container1DStack out=*this;
    for(unsigned int i=0;i<containers_.size();i++) containers_[i] = containers_[i] * scalar;
    return out;
  }
  ztop::container1DStack container1DStack::operator * (float scalar){
    return *this * (double)scalar;
  }
  ztop::container1DStack container1DStack::operator * (int scalar){
    return *this * (double)scalar;
  }

  //rovate memberfunc
  int container1DStack::checkLegOrder(){
    for(std::map<TString,size_t>::iterator it=legorder_.begin();it!=legorder_.end();++it){
      if(it->second > size()){
	std::cout << "container1DStack::checkLegOrder: "<< name_ << ": legend ordering numbers (" <<it->second << ") need to be <= size of stack! ("<< size()<<")" <<std::endl;
	return -1;
      }
    }
    return 0;
  }

  std::vector<size_t> container1DStack::sortEntries(bool inverse){

    std::vector<int> ordering;
    std::vector<size_t> dontuse;

    ordering.resize(size(),-1);
    for(unsigned int it=0;it<size();it++){ 
      if(checkLegOrder()>-1 && legorder_.find(legends_[it])!=legorder_.end()){
	ordering.at(legorder_.find(legends_[it])->second)=it;
	dontuse.push_back(it);
      }
    }
    

    for(size_t it=0;it<ordering.size();it++){ //fill rest
      int ord=ordering.at(it);
      if(ord<0){ //not yet filled
	for(size_t newit=0;newit<size();newit++){
	  bool use=true;
	  for(size_t j=0;j<dontuse.size();j++){
	    if(newit == dontuse.at(j))
	      use=false;
	  }
	  if(use){
	    ordering.at(it)=(int)newit;
	  }
	}
      }
    }

    std::vector<size_t> out;

    if(inverse){
      out.resize(ordering.size(),0);
      size_t outidx=ordering.size()-1;
      for(size_t i=0;i<ordering.size();i++){
	out.at(outidx)=ordering.at(i);
	outidx--;
      }
    }
    else{
      for(size_t i=0;i<ordering.size();i++)
	out.push_back((size_t) ordering.at(i));
    }
    return out;
  }

}//namespace

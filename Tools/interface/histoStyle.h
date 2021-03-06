/*
 * histoStyle.h
 *
 *  Created on: Sep 30, 2013
 *      Author: kiesej
 */

#ifndef HISTOSTYLE_H_
#define HISTOSTYLE_H_
/**
 * for slow migration to histoplotter class using these styles
 * in general set style form template like "ratiodata, ratioMC, controlplotdata, controlplotmc..." and then adapt
 */
#include <vector>
#include "TString.h"
#include <iostream>
#include "tObjectList.h"

class TVirtualPad;
class TH1;
class TGraphAsymmErrors;
class TGAxis;

namespace ztop{
class histoStyle;
class plot;
//helper classes
class axisStyle{
	friend class histoStyle;

public:
	axisStyle();
	~axisStyle();
	axisStyle(float titlesize,float labelsize,float titleoffset, float labeloffset, float ticklength,int ndivisions,float Max, float Min):
		titleSize(titlesize),labelSize(labelsize),titleOffset(titleoffset),labelOffset(labeloffset),tickLength(ticklength),ndiv(ndivisions),max(Max),min(Min),
		log(false),forcezero(false),name("USEDEF"),posx(0),posy(0),posz(0){}

	bool applyAxisRange() const; //if min>max false (default)

	static TString makeTexConsistentName(const TString& in);

	/**
	 * creates new object (handle memory!)
	 */
	TGAxis * makeTGAxis()const;

	float titleSize;
	float labelSize;
	float titleOffset;
	float labelOffset;
	float tickLength;
	int ndiv;

	float max,min; //if min > max, don't apply
	bool log;

	bool forcezero;

	TString name;

	//new

	TString title;
	float posx,posy,posz;

};
class histoStyle;
class textBox{
	friend class histoStyle;
public:
	textBox();
	textBox(float,float,const TString &,float textsize=0.05,int font=62,int align=11,int color=1);
	~textBox();
	void setText(const TString &);
	void setTextSize(float);
	void setCoords(float,float);
	void setFont(int);
	void setAlign(int);
	void setColor(int);
	const TString & getText() const;
	const float & getTextSize() const;
	const float & getX() const;
	const float &  getY() const;
	const int& getFont()const;
	const int& getAlign()const;
	const int& getColor()const;

private:
	float x_,y_;
	TString text_;
	float textsize_;
	int align_,font_;
	int color_;

};

class textBoxes: tObjectList{
public:
	textBoxes(){}
	~textBoxes(){}

	textBox & at(size_t idx){return boxes_.at(idx);}
	const textBox & at(size_t idx)const{return boxes_.at(idx);}

	void clear(){boxes_.clear();}
	textBox& add(float x,float y,const TString & text,float textsize=0.05,int font=62,int align=11,int color=1){
		boxes_.push_back(textBox(x,y,text,textsize,font,align,color));
		return boxes_.at(boxes_.size()-1);
	}
	size_t size(){return boxes_.size();}
	void readFromFileInCMSSW(const std::string & filename, const std::string & markername);
	void readFromFile(const std::string & filename, const std::string& markername);

	void drawToPad(TVirtualPad * p,bool NDC=true);

	static bool debug;
private:
	std::vector<textBox> boxes_;

};

class histoStyle{
public:


	histoStyle();
	~histoStyle();
	histoStyle(float markersize, int markerstyle,int markercolor, float linesize, int linestyle, int linecolor,
			int fillstyle, int fillcolor):
				markerSize(markersize),markerStyle(markerstyle),markerColor(markercolor),lineSize(linesize),lineStyle(linestyle),lineColor(linecolor),
				fillStyle(fillstyle),fillColor(fillcolor),sysFillStyle(1001),sysFillColor(1),legendDrawStyle("lpe"){}


	void readFromFile(const std::string & filename, const std::string& stylename, bool requireall=true);

	void multiplySymbols(float val);

	void applyContainerStyle(TH1*,bool sys)const;
	void applyContainerStyle(TGraphAsymmErrors*,bool sys)const;
	void applyContainerStyle(plot *)const;

	float markerSize;
	int markerStyle;
	int markerColor;

	float lineSize;
	int lineStyle;
	int lineColor;

	int fillStyle;
	int fillColor;

	int sysFillStyle;
	int sysFillColor;

	TString drawStyle,rootDrawOpt,sysRootDrawOpt,legendDrawStyle;

};



}//namespace

#endif /* CONTAINERSTYLE_H_ */

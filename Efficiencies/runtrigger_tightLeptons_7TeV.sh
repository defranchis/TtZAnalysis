#!/bin/sh

#runs analysis und puts analysis code and output to an extra directory with date flag

dirstring=$1"_tightlep_7TeV_trigger_$(date +%F_%H:%M)__GI"

echo '\n\nrunning in directory' "${dirstring}."

mkdir $dirstring
cd $dirstring
cp ../triggerAnalyzer_base3.h .
#cp ../prescaleList.h .
cp ../trigger_tightLeptons_7TeV.C .
cp ../histoStyle.h .
cp ../getSubSetRoot.C .
touch $HOSTNAME
nohup fastCompile.sh trigger_tightLeptons_7TeV.C > output.txt 
echo "done with ${dirstring}"

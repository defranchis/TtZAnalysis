#!/bin/sh

#runs analysis und puts analysis code and output to an extra directory with date flag

dirstring="${1}_$(date +%F_%H:%M)"

echo "\n\nrunning in directory ${dirstring}."

mkdir $dirstring
cd $dirstring
cp ../*inputfiles.txt .
sed 's/runInNotQuietMode=true/runInNotQuietMode=false/' <../Analyzer.C > Analyzer_temp.C
sed  's/testmode=true/testmode=false/' < Analyzer_temp.C > Analyzer.C
rm Analyzer_temp.C

nohup root -l -b -q Analyzer.C++ > output.txt
echo "done with ${dirstring}"

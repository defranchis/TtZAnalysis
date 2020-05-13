#!/bin/sh


channels=( "emu"
    # "ee"
    # "mumu"
     # "smu"
);
## scale and match variations will be ignored for other top masses then 172.5.. (hardcoded below)
topmasses=( "172.5"

     # "171.5"
     # "173.5"
# #agrohsje 
  # "175.5"
  #  "169.5"

#      "178.5"
#      "166.5"
);
#agrohsje 
pdfeigenvectors=28 # POWHEG:NNPDF:50; POWHEG:CT10:26; MG5_aMC@NLO:NNPDF:51   # if set to 0, no variation
# pdfeigenvectors=0 # POWHEG:NNPDF:50; POWHEG:CT10:26; MG5_aMC@NLO:NNPDF:51   # if set to 0, no variation

systs=(

"nominal"

# # agrohsje 
# # 	for testing	


    "TRIGGER_up"
    "TRIGGER_down"
    "ELECSF_up"
    "ELECSF_down"
##    "ELECTRACKSF_up"
##   "ELECTRACKSF_down"
    "MUONSF_up"
    "MUONSF_down"

#     "ELECBGSF_up"
#     "ELECBGSF_down"
#     "MUONBGSF_up"
#     "MUONBGSF_down"
#     "TRIGGERBGSF_up"
#     "TRIGGERBGSF_down"

    "ELECES_up"
    "ELECES_down"

    "ELECER_PHI_up"
    "ELECER_PHI_down"
    "ELECER_RHO_up"
    "ELECER_RHO_down"

    "DYSCALE_up"
    "DYSCALE_down"

    "MUONES_up"
    "MUONES_down"

    "PU_up"
    "PU_down"

    "JER_up"
    "JER_down"

# # #######JES groups according to toplhcwg use INSTEAD of global JES_up/down

    "JES_FlavorPureGluon_up"
    "JES_FlavorPureQuark_up"
    "JES_FlavorPureCharm_up"
    "JES_FlavorPureBottom_up"


    "JES_AbsoluteMPFBias_up"   

    # "JES_FlavorQCD_up"

    "JES_RelativeFSR_up"
    "JES_RelativeStatFSR_up"

    "JES_AbsoluteStat_up"
    "JES_AbsoluteScale_up"
    "JES_Fragmentation_up"
    "JES_SinglePionECAL_up"
    "JES_SinglePionHCAL_up"
    "JES_TimePtEta_up"
    "JES_RelativeJEREC1_up"
##    "JES_RelativeJEREC2_up"
##    "JES_RelativeJERHF_up"
    "JES_RelativePtBB_up"
    "JES_RelativePtEC1_up"
##    "JES_RelativePtEC2_up"
##    "JES_RelativePtHF_up"
    "JES_RelativeStatEC_up"
##    "JES_RelativeStatHF_up"
    "JES_PileUpDataMC_up"
    "JES_RelativeBal_up"

    "JES_PileUpPtBB_up"
    "JES_PileUpPtRef_up"
    "JES_PileUpPtEC1_up"
##    "JES_PileUpPtEC2_up"
##    "JES_PileUpPtHF_up"


# # ##down

    "JES_FlavorPureGluon_down"
    "JES_FlavorPureQuark_down"
    "JES_FlavorPureCharm_down"
    "JES_FlavorPureBottom_down"

    "JES_AbsoluteMPFBias_down"	

    # "JES_FlavorQCD_down"

    "JES_RelativeFSR_down"
    "JES_RelativeStatFSR_down"

    "JES_AbsoluteStat_down"
    "JES_AbsoluteScale_down"
    "JES_Fragmentation_down"
    "JES_SinglePionECAL_down"
    "JES_SinglePionHCAL_down"
    "JES_TimePtEta_down"
    "JES_RelativeJEREC1_down"
##    "JES_RelativeJEREC2_down"
##    "JES_RelativeJERHF_down"
    "JES_RelativePtBB_down"
    "JES_RelativePtEC1_down"
##    "JES_RelativePtEC2_down"
##    "JES_RelativePtHF_down"
    "JES_RelativeStatEC_down"
##    "JES_RelativeStatHF_down"
    "JES_PileUpDataMC_down"
    "JES_RelativeBal_down"

    "JES_PileUpPtBB_down"
    "JES_PileUpPtRef_down"
    "JES_PileUpPtEC1_down"
##    "JES_PileUpPtEC2_down"
##    "JES_PileUpPtHF_down"



# # ########

    # "BTAGH_up"
    # "BTAGH_down"
    "BTAGL_up"
    "BTAGL_down"

    "BTAGH_bfragmentation_up"
    "BTAGH_btempcorr_up"
    "BTAGH_cb_up"
    "BTAGH_cfragmentation_up"
    "BTAGH_dmux_up"
    "BTAGH_gluonsplitting_up"
    "BTAGH_jes_up"
    "BTAGH_jetaway_up"
    "BTAGH_ksl_up"
    "BTAGH_l2c_up"
    "BTAGH_ltothers_up"
    "BTAGH_mudr_up"
    "BTAGH_mupt_up"
    "BTAGH_sampledependence_up"
    "BTAGH_ptrel_up"
    "BTAGH_statistic_up"

    "BTAGH_bfragmentation_down"
    "BTAGH_btempcorr_down"
    "BTAGH_cb_down"
    "BTAGH_cfragmentation_down"
    "BTAGH_dmux_down"
    "BTAGH_gluonsplitting_down"
    "BTAGH_jes_down"
    "BTAGH_jetaway_down"
    "BTAGH_ksl_down"
    "BTAGH_l2c_down"
    "BTAGH_ltothers_down"
    "BTAGH_mudr_down"
    "BTAGH_mupt_down"
    "BTAGH_sampledependence_down"
    "BTAGH_ptrel_down"
    "BTAGH_statistic_down"

    
# #####csv rew section

    "TOPPT_up"
    "TOPPT_down"

    "TT_MATCH_down"
    "TT_MATCH_up"
    "TT_ISRSCALE_down"
    "TT_ISRSCALE_up"
    "TT_FSRSCALE_down"
    "TT_FSRSCALE_up"
    "TT_MESCALE_down"
    "TT_MESCALE_up"
    "TT_TTTUNE_down"
    "TT_TTTUNE_up"
    "ST_MESCALE_up"
    "ST_MESCALE_down"
    "ST_FSRSCALE_up"
    "ST_FSRSCALE_down"
    "ST_ISRSCALE_up"
    "ST_ISRSCALE_down"

    "TT_FRAG_down"
    "TT_FRAG_up"
    "TT_BRANCH_down"
    "TT_BRANCH_up"

    "TT_FRAG_PETERSON_down"
    "TT_FRAG_PETERSON_up"


    "TT_CRGLUON_up"
    "TT_CRGLUON_down"
    "TT_CRQCD_up"
    "TT_CRQCD_down"
    "TT_CRERD_up"
    "TT_CRERD_down"

    "TT_GENMCATNLO_up"     
    "TT_GENMCATNLO_down"     

    "TOPMASS_up"     
    "TOPMASS_down"     


    # "DY_GEN_down"
    # "DY_GEN_up"


);
energies=("13TeV"
#"7TeV"
);



##############################
##############################
##############################
####### USER INPUT ENDS ######
##############################
##############################
##############################

thisscript=$(pwd)/$0
dirname=$1
addParameters=$2

analysisDir=$CMSSW_BASE/src/TtZAnalysis/Analysis/
templatesDir=$CMSSW_BASE/src/TtZAnalysis/Analysis/templates


########add pdf variations#######

if [[ "${addParameters}" == *"-B"* ]]
then
    pdfeigenvectors=0
    echo
    echo "not running PDF variations when btag SF are derived"
    echo
fi

if [[ $pdfeigenvectors != "0" ]] && [[ $pdfeigenvectors ]]
then
    systs=( "${systs[@]}" "PDF_sysnominal" );
    for ((i = 1; i <= $pdfeigenvectors; i++));
    do
	systs=( "${systs[@]}" "PDF_sysnominal_PDF${i}_down" )
	systs=( "${systs[@]}" "PDF_sysnominal_PDF${i}_up" )
    done
fi
echo running on systematics: 
echo ${systs[@]}
echo 

sleep 3



JOBSONBATCH=`command -v condor_submit`
if [ ${INTERACTIVE_ANALYSIS_JOBS} ]
then
JOBSONBATCH=""
fi


cd $analysisDir
workdir=$(date +"%Y%m%d_%H%M")_${dirname}
if [[ $JOBSONBATCH ]] ;
then
    mkdir -p /nfs/dust/cms/user/$USER/AnalysisWorkdir
    if [[ ! -L "workdir" ]]
    then
	ln -s /nfs/dust/cms/user/$USER/AnalysisWorkdir workdir 
    fi
    cd workdir
    mkdir $workdir
else 
    mkdir -p $analysisDir/interactiveWorkdir  
    cd interactiveWorkdir
    mkdir $workdir
fi
if [[ -L "last" ]]
then
    rm last
fi
ln -s $workdir last
cd $workdir

echo "running in dir $workdir"
#make path absolute
workdir=`pwd`
BATCHDIR=$workdir/batch

#make directory tree

mkdir -p $workdir/scripts
mkdir -p $workdir/bin
mkdir -p $workdir/lib #not needed anymore
mkdir -p $workdir/data/analyse
mkdir -p $workdir/jobscripts
mkdir -p $workdir/stdout
mkdir -p $workdir/logs
mkdir -p $workdir/configs/analyse
mkdir -p $BATCHDIR

cp $thisscript scripts/

echo creating independent executable package...
echo "(code can be compiled while jobs are running)"
cd $workdir/bin
makePackage.sh analyse $workdir/bin/analyse& #can run in background..
cd $workdir

echo copying data files...
rsync -a $analysisDir/data/analyse data/ --exclude *_btags
cd $analysisDir/data/analyse
if [[ "${addParameters}" == *"-B"* ]]; then
    echo "not copying b-tagging eff. files"
else
    for d in *_btags
    do
        tar czf $workdir/data/analyse/$d.tar.gz $d &
    done
    wait
    cd $workdir/data/analyse
    for f in *_btags.tar.gz #maybe there are even more
    do
        tar xzf $f &
    done
fi
# do other stuff in the meantime...


cd $workdir
cp -r $analysisDir/configs/analyse configs/
cp -r $analysisDir/src .
cp $analysisDir/interface/MainAnalyzer.h src/
cp $analysisDir/bin/analyse.cc src/

echo -n "These files are just meant for reference (e.g. to check control plot configurations etc)" > src/README

cd $workdir


if [ $JOBSONBATCH ]
then
sed -e 's;##WORKDIR##;'${workdir}';g' < $templatesDir/check.sh > check.sh
else
sed -e 's;##WORKDIR##;'${workdir}';g' -e 's;##JOBSNOTONBATCH##;;g'< $templatesDir/check.sh > check.sh
fi
chmod +x check.sh

sed -e 's;##WORKDIR##;'${workdir}';g' < $templatesDir/reset_job.sh > reset_job.sh
chmod +x reset_job.sh

# wait for tars to be finished and delete
wait
for f in $workdir/data/analyse/*_btags.tar.gz
do
     rm -f $f &
done

echo "preparation done, start submission..."
cd $workdir
#check whether running on naf or wgs and do qsub or dirty "&"
for (( i=0;i<${#channels[@]};i++)); do
    for (( l=0;l<${#topmasses[@]};l++)); do
	for (( j=0;j<${#systs[@]};j++)); do
	    for (( k=0;k<${#energies[@]};k++)); do

		channel=${channels[${i}]}
		topmass=${topmasses[${l}]}
		syst=${systs[${j}]}
		energy=${energies[${k}]}

##dont run on scale,top pt and matching syst with other top mass
		# if [[ "${syst}" == *"SCALE"* ]] ||  [[ "${syst}" == *"MATCH"* ]] || [[ "${syst}" == *"TOPPT"* ]] ||  [[ "${syst}" == *"_sysnominal"* ]] ||  [[ "${syst}" == *"TT_GEN"* ]]  ||  [[ "${syst}" == *"BJESNUDEC"* ]]  ||  [[ "${syst}" == *"BJESRETUNE"*  ]]
                if [[ "${syst}" != "nominal" ]]
		then
		    if [[ "${topmass}" != "172.5" ]] 
		    then
			continue
		    fi
		fi
		if [[ "${syst}" == *"P11_sysnominal"* ]] && [[ "${energy}" = "7TeV" ]] 
		    then
		    continue
		fi

#### dont submit b variations if effieicnes are derived
		if [[ "${addParameters}" == *"-B"* ]] && [[ "${syst}" == *"BTAG"* ]]
		then
		    echo "not running btag systematics when btag SF are derived"
		    continue
		fi
		
##here do qsub or dirty &
		outname=${channel}_${energy}_${topmass}_${syst};
	   # array=( "${array[@]}" "jack" )
		outnames=( "${outnames[@]}" "${outname}" );
                cp $templatesDir/job_HTCondor_looseKinReco.sh jobscripts
		sed -e "s;##WORKDIR##;${workdir};" < $templatesDir/job_HTCondor_looseKinReco.sh > jobscripts/job_HTCondor_looseKinReco.sh
		chmod u+x jobscripts/job_HTCondor_looseKinReco.sh
		sed -e "s;##WORKDIR##;${workdir};" -e "s/#CHANNEL#/${channel}/" -e "s/#ENERGY#/${energy}/" -e "s/#TOPMASS#/${topmass}/" -e "s/#SYST#/${syst}/" -e "s/#ADDPARAMS#/${addParameters}/" -e "s/#BATCHNAME#/${outname}/" < $templatesDir/HTCondor_looseKinReco.submit > jobscripts/${outname}
                chmod +x jobscripts/${outname}
		if [[ $JOBSONBATCH ]] ;
		then
		    cd $BATCHDIR
		    condor_submit $workdir/jobscripts/${outname}
		    cd $workdir
		else
		    all=`ps ax | grep -E 'analyse' | wc -l`
		    defunct=`ps ax | grep -E 'analyse' | grep -E 'defunct' | wc -l`
		    running=`expr $all - $defunct`
		    while [ $running  -gt 10 ]; do
			sleep 2;
			all=`ps ax | grep -E 'analyse' | wc -l`
			defunct=`ps ax | grep -E 'analyse' | grep -E 'defunct' | wc -l`
			running=`expr $all - $defunct`
		    done
		    echo "starting ${outname}"
		    ./jobscripts/${outname} &
		    sleep 10;
		fi

###make the merge line

###what about check script?
	    done
	done
    done
done

function contcheck(){

    $workdir/check.sh
    while [ $? -ne 0 ]; 
    do
	sleep 150
	echo
	echo time: $(date +"%H:%M")
	$workdir/check.sh
    done

}

## continuous checking

if [[ $3 == "c" ]]
then
    echo "swiching to continuous checking mode"
    contcheck


else
    echo "to check the status run ${workdir}/check.sh"
fi


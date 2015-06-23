import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
import sys 
import os 

### parsing job options 

options = VarParsing.VarParsing()#'standard')

options.register('runOnAOD', False, VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.bool, "run on AOD")
options.register('runOnMC',True,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool,"run on MC")
options.register('channel', 'emu', VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.string, "which final state")
options.register('isSignal',False,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool,"is SignalMC")
options.register('maxEvents',-1,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.int,"maximum events")
options.register('skipEvents', 0, VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.int, "skip N events")
options.register('inputScript','TopAnalysis.Configuration.Phys14.miniAOD.TT_Tune4C_13TeV_pythia8_tauola_Phys14DR_PU20bx25_tsg_PHYS14_25_V1_v1_cff',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"input Script")
#options.register('inputScript', '', VarParsing.VarParsing.multiplicity.singleton, VarParsing.VarParsing.varType.string, "python file with input source")
options.register('json','nojson',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"json files")
options.register('outputFile','def_out',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"output File (w/o .root)")
options.register('globalTag','',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"global tag")
options.register('genFilter','none',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"gen Filter")
options.register('genFilterString','none',VarParsing.VarParsing.multiplicity.list,VarParsing.VarParsing.varType.string,"gen Filter selection string")
options.register('genFilterMassLow',0,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.int,"gen Filter mass range low")
options.register('genFilterMassHigh',0,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.int,"gen Filter mass range high")
options.register('genFilterInvert',False,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool,"invert gen Filter")
options.register('includePDFWeights',False,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool,"includes pdf weights info for event")
options.register('PDF','CT10',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"pdf set for weights")
options.register('nominalPDF','1001',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"weight index of nominal pdf")
options.register('beginPDFVar','2001',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"first weight index for pdf variation")
options.register('endPDFVar','2052',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"last weight index for pdf variation")
options.register('reportEvery',1000,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.int,"report every")
options.register('wantSummary',True,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool,"prints trigger summary")
options.register('debug',False,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool,"enters debug mode")

print '\nStart generating ntuple\n***********************\n'


print 'Reading command line \n'

if hasattr(sys, "argv"):
    options.parseArguments()

runOnAOD=options.runOnAOD
if runOnAOD:
    print 'Configuration for data type: AOD'
else:
    print 'Configuration for data type: miniAOD'
runOnMC=options.runOnMC                     # True
isSignal=options.isSignal
if not runOnMC:
    isSignal=False     
globalTag=options.globalTag                 # START53_V11
reportEvery=options.reportEvery             # 1000
maxEvents=options.maxEvents                 # -1. overwrites the one specified in input script. be careful!!
skipEvents=options.skipEvents
outputFile=options.outputFile               # def_out
genFilter=options.genFilter                 # 'none'
genFilterString=options.genFilterString     # 'none'
genFilterInvert=options.genFilterInvert     # False
includePDFWeights=options.includePDFWeights # False
PDF=options.PDF                             # CT10
nominalPDF=options.nominalPDF
beginPDFVar=options.beginPDFVar
endPDFVar=options.endPDFVar
inputScript=options.inputScript             # TtZAnalysis.Configuration.samples.mc.DYJetsToLL_M-50_TuneZ2Star_8TeV-madgraph-tarball_Summer12-PU_S7_START52_V9-v2_cff
print 'Using input script: ', inputScript
json=options.json                           # parse absolute paths here!
print 'Using json path: ', json
wantSummary=options.wantSummary
debug=options.debug
useBHadrons=False                           #will be changes for top filter sequence!

print '\nDone with reading command line!\n'

minleptons=2

print 'Minimum 2 leptons required!'


# if sets are not in official paths
if includePDFWeights:
    newlha = os.environ['CMSSW_BASE']+'/src/TtZAnalysis/Data/data/PDFSets'


###############################################################
## Set Options for physics Objects
import TopAnalysis.Configuration.objectDefinitions13Tev.definitionOptions_cff as opt

pfpostfix = "PFlow"

# All Options are saved as dicts and have to be added in the definitionsOptions as empty dicts
opt.globalOptions = {'runOnAod':options.runOnAOD,
                 'signal':isSignal}
opt.primaryVertexOptions = {'inputCollectionAod':'offlinePrimaryVertices',  # Input Collection for AOD
                 'inputCollectionMiniAod':'offlineSlimmedPrimaryVertices', # Input Collection for MiniAOD
                 'outputCollection':''}       # The Collection which is Produced, is set in the apropriate cff File
opt.muonOptions={'inputCollectionAod':'selectedPatMuons'+pfpostfix,
                 'inputCollectionMiniAod': 'slimmedMuons',
                 'outputCollection':'',
                 'Cuts':False} #Only very loose cuts
opt.electronOptions={'inputCollectionAod':'selectedPatElectrons'+pfpostfix,
                 'inputCollectionMiniAod':'slimmedElectrons',
                 'outputCollection':'',
                 'idName':'Phys14-medium', # Name of the Id in the IdVector (on the Electrons)
                 'Cuts':False}

####################################################################
## Define the process

process = cms.Process("Yield")
process.options = cms.untracked.PSet( 
    wantSummary = cms.untracked.bool(wantSummary),
    allowUnscheduled = cms.untracked.bool(True) 
    )


####################################################################
## Define input 

if inputScript!='':
    process.load(inputScript)
else:
    print 'need an input script'
    exit(8889)

process.maxEvents = cms.untracked.PSet( 
    input = cms.untracked.int32 (maxEvents) 
)

if skipEvents > 0:
    process.source.skipEvents = cms.untracked.uint32(skipEvents)

if not runOnMC and not (json=="nojson"):
    import FWCore.PythonUtilities.LumiList as LumiList
    import FWCore.ParameterSet.Types as CfgTypes
    myLumis = LumiList.LumiList(filename = json).getCMSSWString().split(',')
    process.source.lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())
    process.source.lumisToProcess.extend(myLumis)


####################################################################
## Configure message logger           


process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.destinations = ['cout', 'cerr']
process.MessageLogger.cerr.threshold = 'INFO'
#process.MessageLogger.suppressWarning=['particleFlowDisplacedVertexCandidate','The interpolated laser correction is <= zero! (0). Using 1. as correction factor.']
process.MessageLogger.cerr.FwkReport.reportEvery = reportEvery


####################################################################
### define output (agrohsje: check why needed) 

if runOnAOD:
    process.out = cms.OutputModule("PoolOutputModule", 
                                   outputCommands =  cms.untracked.vstring(), 
                                   fileName = cms.untracked.string(outputFile+'_PatTuple'))

####################################################################
### Geometry and Detector Conditions  


process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('Configuration.StandardSequences.MagneticField_cff') # contains https://cmssdt.cern.ch/SDT/lxr/source/Configuration/StandardSequences/python/MagneticField_cff.py MagneticField_38T_cff

if globalTag != '':
    print "Setting global tag to the command-line value"
    process.GlobalTag.globaltag = cms.string(globalTag + '::All')
else:
    print "Determine global tag automatically"
    if options.runOnMC:
        process.GlobalTag.globaltag = cms.string('MCRUN2_74_V7D::All')
        #agrohsje process.GlobalTag.globaltag = cms.string('PHYS14_50_V2::All') 
    else:
        process.GlobalTag.globaltag = cms.string('FT53_V21A_AN6::All')
        
print "Using global tag: ", process.GlobalTag.globaltag


jecFile=os.path.relpath( os.environ['CMSSW_BASE']+'/src/TtZAnalysis/Data/Run2/PHYS14_V4_MC.db')
from CondCore.DBCommon.CondDBSetup_cfi import *
process.jec = cms.ESSource("PoolDBESSource",CondDBSetup,
    connect = cms.string('sqlite_file:'+jecFile),
    toGet =  cms.VPSet(
    cms.PSet(record = cms.string("JetCorrectionsRecord"),
        tag = cms.string("JetCorrectorParametersCollection_PHYS14_V4_MC_AK4PF"),
        Label = cms.untracked.string("AK4PF")),
        cms.PSet(record = cms.string("JetCorrectionsRecord"),
        tag =cms.string("JetCorrectorParametersCollection_PHYS14_V4_MC_AK4PFchs"),
        Label = cms.untracked.string("AK4PFchs"))
    )
)
process.es_prefer_jec = cms.ESPrefer("PoolDBESSource","jec")



####################################################################
## Configure TFileService    


outFileName = outputFile + '.root'
print 'Using output file ' + outFileName

process.TFileService = cms.Service("TFileService", 
                                   fileName = cms.string(outFileName))


####################################################################
## Weights 


## agrohsje mc systematic weights 
process.load("TopAnalysis.ZTopUtils.EventWeightMCSystematic_cfi")
process.nominal = cms.EDProducer("EventWeightMCSystematic",
                                 genEventInfoTag=cms.InputTag("generator"),
                                 lheEventInfoTag=cms.InputTag("externalLHEProducer"),
                                 weightID=cms.string("0000"), 
                                 printLHE=cms.bool(False)
                                 )
process.scaleUp = cms.EDProducer("EventWeightMCSystematic",
                                 genEventInfoTag=cms.InputTag("generator"),
                                 lheEventInfoTag=cms.InputTag("externalLHEProducer"),
                                 weightID=cms.string("1005"), 
                                 printLHE=cms.bool(False)                                 
                                 )
process.scaleDown = cms.EDProducer("EventWeightMCSystematic",
                                   genEventInfoTag=cms.InputTag("generator"),
                                   lheEventInfoTag=cms.InputTag("externalLHEProducer"),
                                   weightID=cms.string("1009"), 
                                   printLHE=cms.bool(False)
                                   )
process.hDampUp = cms.EDProducer("EventWeightMCSystematic",
                                 genEventInfoTag=cms.InputTag("generator"),
                                 lheEventInfoTag=cms.InputTag("externalLHEProducer"),
                                 weightID=cms.string("5014"),
                                 printLHE=cms.bool(False)
                                 )
process.hDampDown = cms.EDProducer("EventWeightMCSystematic",
                                   genEventInfoTag=cms.InputTag("generator"),
                                   lheEventInfoTag=cms.InputTag("externalLHEProducer"),
                                   weightID=cms.string("5027"),
                                   printLHE=cms.bool(False)
                                   )

if isSignal:
    additionalWeights = cms.vstring("nominal","scaleUp","scaleDown")
else:
    additionalWeights = cms.vstring("nominal")

## PDF weights if required 
if includePDFWeights:
    process.pdfWeights = cms.EDProducer("EventWeightPDF",
                                        lheEventInfoTag=cms.InputTag("externalLHEProducer"),
                                        PDFName=cms.vstring(PDF),
                                        nominalWeightID=cms.vstring(nominalPDF),
                                        beginWeightID=cms.vstring(beginPDFVar),
                                        endWeightID=cms.vstring(endPDFVar),
                                        printLHE=cms.bool(False)
                                        )
#cms.EDProducer("PdfWeightProducer",
#PdfInfoTag = cms.untracked.InputTag("generator"),
#PdfSetNames = cms.untracked.vstring(PDF+".LHgrid"))

## Infos for proper normalization
process.load('TtZAnalysis.TreeWriter.puinfo_cff')

process.PUInfo.includePDFWeights = includePDFWeights
process.PUInfo.pdfWeights = "pdfWeights:"+PDF
if runOnAOD: 
    process.PUInfo.vertexSrc='offlinePrimaryVertices'
else:
    process.PUInfo.vertexSrc='offlineSlimmedPrimaryVertices'
    
process.preCutPUInfo = process.PUInfo.clone()
process.preCutPUInfo.treeName = 'preCutPUInfo'

process.postCutPUInfo = process.PUInfo.clone()
process.postCutPUInfo.treeName = 'PUTreePostCut'
process.postCutPUInfo.includePDFWeights = False


    
#################################################################### 
## Generator-level objects 
    

## Define names     
genParticleCollection = ''
genJetInputParticleCollection = ''

if runOnAOD:
    genParticleCollection = 'genParticles'
    genJetInputParticleCollection = 'genParticles'
else:
    genParticleCollection = 'prunedGenParticles'
    genJetInputParticleCollection = 'packedGenParticles'

genJetCollection = 'ak4GenJetsNoNuNoLepton'
genJetFlavourInfoCollection = 'ak4GenJetFlavourPlusLeptonInfos'

## Create objects 
## Details in: PhysicsTools/JetExamples/test/printJetFlavourInfo.cc, PhysicsTools/JetExamples/test/printJetFlavourInfo.py
## and in: https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideBTagMCTools#New_jet_flavour_definition  

## Supply PDG ID to real name resolution of MC particles
process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")

## Input particles for gen jets (stable gen particles to be used in clustering, excluding electrons, muons and neutrinos from hard interaction)
from RecoJets.Configuration.GenJetParticles_cff import genParticlesForJets
process.genParticlesForJetsNoNuNoLepton = genParticlesForJets.clone(
    src = genJetInputParticleCollection,
    excludeResonances = True,
    excludeFromResonancePids = [11, 12, 13, 14, 16],
)

from RecoJets.JetProducers.ak4GenJets_cfi import ak4GenJets
process.ak4GenJetsNoNuNoLepton = ak4GenJets.clone(src = "genParticlesForJetsNoNuNoLepton")

# Ghost particle collection for matching to gen jets (b/c hadrons + leptons)
from PhysicsTools.JetMCAlgos.HadronAndPartonSelector_cfi import selectedHadronsAndPartons
process.selectedHadronsAndPartons = selectedHadronsAndPartons.clone(particles = genParticleCollection)

# Flavour info: jet collection with all associated ghosts
# For the moment leptons need to be specified explicitely here, until lepton access can be made more generic in miniAOD
# This is only needed as long as the jetConstituents are not accessible directly in miniAOD, then it should be fixed
# by using the leptons from the constituents, instead of feeding them as ghosts into the jets 

from PhysicsTools.JetMCAlgos.AK4PFJetsMCFlavourInfos_cfi import ak4JetFlavourInfos
process.ak4GenJetFlavourPlusLeptonInfos = ak4JetFlavourInfos.clone(
    jets = genJetCollection,
    leptons = cms.InputTag("selectedHadronsAndPartons", "leptons")
)


####################################################################
## Generator-level selection 

if genFilter == "ttbar":
    process.load('TtZAnalysis.TreeWriter.topDecayFilter_cff')
    process.topDecayFilter.src=genParticleCollection
    process.topDecayFilter.invert=genFilterInvert
    process.fsFilterSequence = cms.Sequence(process.topDecayFilter)
else :
    process.fsFilterSequence = cms.Sequence()
    
####################################################################
## Prefilter sequence


if runOnMC:
    process.prefilterSequence = cms.Sequence()
else:
    ## HCAL noise filter
    process.load('CommonTools/RecoAlgos/HBHENoiseFilter_cfi')
    process.HBHENoiseFilter.minIsolatedNoiseSumE = cms.double(999999.)
    process.HBHENoiseFilter.minNumIsolatedNoiseChannels = cms.int32(999999)
    process.HBHENoiseFilter.minIsolatedNoiseSumEt = cms.double(999999.)
    ## Beam scraping filter 
    process.scrapingFilter = cms.EDFilter("FilterOutScraping",
                                          applyfilter = cms.untracked.bool(True),
                                          debugOn = cms.untracked.bool(False),
                                          numtrack = cms.untracked.uint32(10),
                                          thresh = cms.untracked.double(0.25)
                                          )
    ## ECAL laser correction filter 
    process.load("RecoMET.METFilters.ecalLaserCorrFilter_cfi")
    
    
    process.prefilterSequence = cms.Sequence(
        process.HBHENoiseFilter *
        process.scrapingFilter *
        process.ecalLaserCorrFilter
        )


####################################################################
## Primary vertex filtering
process.load('TopAnalysis.Configuration.objectDefinitions13Tev.primaryVertex_cff')


#################################################################### 
## Trigger information  


if runOnAOD:    
    from PhysicsTools.PatAlgos.tools.trigTools import * 
    switchOnTrigger( process ) 
    process.patTrigger.onlyStandAlone = False
    process.triggerSequence = cms.Sequence(process.patTrigger*
                                           process.patTriggerEvent)
else:
    process.triggerSequence = cms.Sequence()
    
    
####################################################################
## Full configuration for PF2PAT 


if runOnMC:
    jetCorr =('AK4PFchs', ['L1FastJet','L2Relative','L3Absolute'], 'None')
else:
    jetCorr = ('AK4PFchs', ['L1FastJet','L2Relative','L3Absolute', 'L2L3Residual'])

#process.userPatSequence = cms.Sequence() ;

## Create and define reco objects 
if runOnAOD:
    muonTag = 'patMuons' + pfpostfix  
    metTag = 'patMETs' + pfpostfix  
    
    ## Output module for edm files (needed for PAT sequence, even if not used in EndPath) 
    from Configuration.EventContent.EventContent_cff import FEVTEventContent
    process.out = cms.OutputModule("PoolOutputModule",
                                   FEVTEventContent,
                                   dataset = cms.untracked.PSet(dataTier = cms.untracked.string('RECO')),
                                   fileName = cms.untracked.string("eh.root"),
                                   )
    # add particle flow
    process.load("PhysicsTools.PatAlgos.patSequences_cff")
    from PhysicsTools.PatAlgos.tools.pfTools import usePF2PAT
    usePF2PAT(process, runPF2PAT=True, jetAlgo='AK4', runOnMC=runOnMC, postfix=pfpostfix, 
              jetCorrections=jetCorr, pvCollection=cms.InputTag('offlinePrimaryVertices'),typeIMetCorrections=True) 

    getattr(process, 'pfPileUp'+pfpostfix).checkClosestZVertex = False
    
    # switch off all top projections
    getattr(process,'pfNoMuon'+pfpostfix).enable = False
    getattr(process,'pfNoElectron'+pfpostfix).enable = False
    getattr(process,'pfNoJet'+pfpostfix).enable = False
    getattr(process,'pfNoTau'+pfpostfix).enable = False
    # agrohsje don't require isolation on cmsRun level !!!
    # tarndt especially don't require it in the default pf2pat process
    #        PFBRECO is subset of pf2pat processes 
    getattr(process,'pfIsolatedElectronsPFBRECO'+pfpostfix).cut=''
    getattr(process,'pfElectronsFromVertexPFBRECO'+pfpostfix).d0Cut = cms.double(999999.)
    getattr(process,'pfElectronsFromVertexPFBRECO'+pfpostfix).dzCut = cms.double(999999.) 
    getattr(process,'pfIsolatedMuonsPFBRECO'+pfpostfix).cut = ''
    getattr(process,'pfMuonsFromVertexPFBRECO'+pfpostfix).d0Cut = cms.double(999999.)
    getattr(process,'pfMuonsFromVertexPFBRECO'+pfpostfix).dzCut = cms.double(999999.)


else :
    #default values for MiniAODs 
    muonTag = 'slimmedMuons' 
    metTag = 'slimmedMETs'

    #redo jets to adjust b-tagging/JEC info  
    from RecoJets.JetProducers.ak4PFJets_cfi import ak4PFJets
    from RecoJets.JetProducers.ak4GenJets_cfi import ak4GenJets
    process.pfCHS = cms.EDFilter("CandPtrSelector", 
                                 src = cms.InputTag("packedPFCandidates"), 
                                 cut = cms.string("fromPV"))
    process.ak4PFJetsCHS = ak4PFJets.clone(src = 'pfCHS', doAreaFastjet = True)
    
    

###########################################################################
# Electron IDs are calculated in a function
import TopAnalysis.Configuration.objectDefinitions13Tev.electron_cff as electron
electron.electron(process)

#################################################################### 
## Use SwitchJetCollection in order to rerun the btagging


## b-tag infos
#bTagInfos = ['impactParameterTagInfos','secondaryVertexTagInfos']
## b-tag discriminators
bTagDiscriminators = ['pfCombinedInclusiveSecondaryVertexV2BJetTags'
                      ]
## Jets, tracks, and vertices 
if runOnAOD:
    jetSource = 'pfJets'+pfpostfix
    pfCandidates = 'particleFlow'
    pvSource = 'offlinePrimaryVertices'
    svSource = cms.InputTag('inclusiveCandidateSecondaryVertices')
    electronSource=cms.InputTag(opt.electronOptions['inputCollectionAod'])
    muonSource=cms.InputTag(opt.muonOptions['inputCollectionAod'])
else:
    jetSource = 'ak4PFJetsCHS' 
    pfCandidates = 'packedPFCandidates'
    pvSource = 'offlineSlimmedPrimaryVertices'
    svSource = cms.InputTag('slimmedSecondaryVertices')
    electronSource=cms.InputTag(opt.electronOptions['inputCollectionMiniAod'])
    muonSource=cms.InputTag(opt.muonOptions['inputCollectionMiniAod'])
    
####################################################################
## Lepton information and dilepton filter at reco level 

process.load('TopAnalysis.Configuration.objectDefinitions13Tev.muon_cff')

from PhysicsTools.PatAlgos.selectionLayer1.electronSelector_cfi import *
from PhysicsTools.PatAlgos.selectionLayer1.jetSelector_cfi import *

process.kinElectrons = selectedPatElectrons.clone(
    src = opt.electronOptions["outputCollection"],
    cut = 'pt > 8  && abs(eta) < 2.7' # because of ECalP4 to be on the safe side
    )        

process.muonEleMerge = cms.EDProducer("CandViewMerger",
                                     src = cms.VInputTag(cms.InputTag("kinElectrons"),cms.InputTag(opt.muonOptions['outputCollection']                                           ))
                                     )

## filter on at least two opposite charge elecs/muons or emu
process.filterkinLeptons = cms.EDFilter("SimpleCounter",
                                        src = cms.VInputTag(cms.InputTag("muonEleMerge")), 
                                        requireOppoQ = cms.bool(True),
                                        minNumber = cms.uint32(minleptons),
                                        debug=cms.bool(debug)
                                        )
if isSignal:
    process.kinLeptonFilterSequence = cms.Sequence()
else:
    process.kinLeptonFilterSequence = cms.Sequence(process.filterkinLeptons)

from PhysicsTools.PatAlgos.tools.jetTools import switchJetCollection
# Switch the default jet collection (done in order to use the above specified b-tag infos and discriminators)
#switchJetCollection(
#    process,
#    jetSource = cms.InputTag('ak4PFJetsCHS'),#jetSource),
#    pfCandidates = cms.InputTag(pfCandidates),
#    pvSource = cms.InputTag(pvSource),
#    svSource = svSource,
#    elSource = electronSource,
#    muSource = muonSource,
#    btagInfos = bTagInfos,
#    btagDiscriminators = bTagDiscriminators,
#    jetCorrections = jetCorr,
#    genJetCollection = cms.InputTag(genJetCollection),
#    postfix = pfpostfix
#)


#getattr(process,'patJetPartons'+pfpostfix).particles = cms.InputTag(genParticleCollection)
#getattr(process,'patJetPartonMatch'+pfpostfix).matched = cms.InputTag(genParticleCollection)
#getattr(process,'jetTracksAssociatorAtVertex'+pfpostfix).tracks = cms.InputTag(pfCandidates)
#getattr(process,'inclusiveSecondaryVertexFinderTagInfos'+pfpostfix).extSVCollection = svSource

#from PhysicsTools.PatAlgos.tools.pfTools import *
### Adapt primary vertex collection
#adaptPVs(process, pvCollection=cms.InputTag(pvSource))

#patJets = ['patJets'+pfpostfix]
#for m in patJets:
#    if hasattr(process,m):
#        print "Switching 'addTagInfos' for " + m + " to 'True'"
#        setattr( getattr(process,m), 'addTagInfos', cms.bool(True) )
#        print "Switching 'addJetFlavourInfo' for " + m + " to 'True'"
#        setattr( getattr(process,m), 'addJetFlavourInfo', cms.bool(True) )


from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import patJetCorrFactorsUpdated
process.patJetCorrFactorsReapplyJEC = patJetCorrFactorsUpdated.clone(
  src = cms.InputTag("slimmedJets"),
  levels = ['L1FastJet', 
        'L2Relative', 
        'L3Absolute'],
  payload = 'AK4PFchs' ) # Make sure to choose the appropriate levels and payload here!

from PhysicsTools.PatAlgos.producersLayer1.jetUpdater_cff import patJetsUpdated
process.patJetsReapplyJEC = patJetsUpdated.clone(
  jetSource = cms.InputTag("slimmedJets"),
  jetCorrFactorsSource = cms.VInputTag(cms.InputTag("patJetCorrFactorsReapplyJEC"))
)





####################################################################
## Jet information 

process.treeJets = selectedPatJets.clone( 
    src="patJetsReapplyJEC",   #'patJets'+pfpostfix, 
    cut='eta < 5 && pt>5 &&neutralHadronEnergyFraction < 0.99 && chargedEmEnergyFraction < 0.99 && neutralEmEnergyFraction < 0.99 && chargedHadronEnergyFraction > 0.0 && chargedMultiplicity > 0.0') # unfortunately starting at 10 GeV are needed for MET rescaling 8GeV should be ok as corrected pt 
    ### cut at uncorrected pt > 10 GeV on tree writer level! for MET rescaling - might be obsolete for 13 TeV (and when not using MET...)
    

#################################################################### 
## Configure ntuple 
process.PFTree   = cms.EDAnalyzer('TreeWriterTtZ',

                                  debugmode = cms.bool(debug),
                                  runOnAOD  = cms.bool(runOnAOD),
                                  #general input collections
                                  treeName = cms.string('pfTree'),
                                  muonSrc = cms.InputTag(muonTag),
                                  keepElecIdOnly = cms.string("Phys14-medium"),
                                  elecGSFSrc = cms.InputTag(opt.electronOptions["outputCollection"]), #just the same here to make it run. this can be prevented by a try{}catch(...){} in treewriter for getByLabel
                                  elecPFSrc = cms.InputTag(opt.electronOptions["outputCollection"]),
                                  jetSrc = cms.InputTag('treeJets'),  #jetTag), # ('treeJets'),
                                  btagAlgo = cms.string('pfCombinedInclusiveSecondaryVertexV2BJetTags'), ###combinedSecondaryVertexBJetTags'),
                                  metSrc = cms.InputTag(metTag),  #here also try, catch statements
                                  mvaMetSrc = cms.InputTag(metTag), 
                                  metT1Src   =cms.InputTag(metTag),
                                  metT0T1TxySrc        =cms.InputTag(metTag),
                                  metT0T1Src           =cms.InputTag(metTag),
                                  metT1TxySrc          =cms.InputTag(metTag),
                                  vertexSrc = cms.InputTag(pvSource ),
                                  
                                  #block for extended information needed for efficiency studies
                                  includeReco = cms.bool(False),
                                  recoMuonSrc = cms.InputTag('muons'),
                                  isPFMuonCand = cms.bool(False),
                                  recoElecSrc  = cms.InputTag('gsfElectrons'),
                                  isPFElecCand = cms.bool(False),
                                  recoTrackSrc = cms.InputTag('generalTracks'),
                                  recoSuCluSrc = cms.InputTag('superClusters'),
                                                                    
                                  #block for trigger information
                                  includeTrigger = cms.bool(True),
                                  triggerResults = cms.InputTag('TriggerResults','','HLT'), #needed for both AOD and MiniAOD
                                  triggerEvent   = cms.InputTag('patTriggerEvent'), ### "selectedPatTrigger" for MiniAODs
                                  triggerObjects = cms.vstring(""),
                                  
                                  #block for event information.  Essential (PU)
                                  PUInfo = cms.InputTag('addPileupInfo'),
                                  includePDFWeights = cms.bool(includePDFWeights),
                                  pdfWeights = cms.InputTag("pdfWeights:"+PDF),
                                  additionalWeights = additionalWeights, #can be any vector of strings, generated by EDProducer: EventWeightMCSystematic
                                  #agrohsje fix  
                                  rhoIso = cms.InputTag("fixedGridRhoFastjetAll"), #dummy for 13 TeV AOD
                                  
                                  includeGen = cms.bool(isSignal),
                                  genParticles = cms.InputTag(genParticleCollection),
                                  # agrohsje fix later 
                                  genJets = cms.InputTag("ak4GenJetsNoNuNoLepton"), 
                                  #cms.InputTag(""), # in miniaod:slimmedGenJets, in aod:ak4GenJets
                                  

                                  ##improved jet-hadron matching block. Not implemented in the configuration above!
                                  useBHadrons = cms.bool(False),
                                  BHadronVsJet = cms.InputTag(""),
                                  AntiBHadronVsJet = cms.InputTag(""),
                                  BJetIndex     = cms.InputTag(""),
                                  AntiBJetIndex = cms.InputTag(""),
                                  BHadJetIndex = cms.InputTag(""),
                                  AntiBHadJetIndex = cms.InputTag(""),
                                  BHadrons = cms.InputTag(""),
                                  AntiBHadrons = cms.InputTag(""),
                                  BHadronFromTopB = cms.InputTag(""),
                                  AntiBHadronFromTopB = cms.InputTag(""),
                                  genBHadPlusMothers = cms.InputTag(""),
                                  genBHadPlusMothersIndices = cms.InputTag(""),
                                  genBHadIndex = cms.InputTag(""),
                                  genBHadFlavour = cms.InputTag(""),
                                  genBHadJetIndex = cms.InputTag(""),
                                  
                                  #old switches, can be removed at some point
                                  isSusy =  cms.bool(False),
                                  isJPsi = cms.bool(False),

                                  #setting PartonShower to Pythia8
                                  partonShower = cms.string("pythia8"),
                                  )


####################################################################
## Path (filter and analyzer only)

process.dump=cms.EDAnalyzer('EventContentAnalyzer')

process.path = cms.Path( 
    process.preCutPUInfo*
    process.fsFilterSequence*
    process.postCutPUInfo*
#    process.goodOfflinePrimaryVertices *   
    process.prefilterSequence * #for data only 
    #    process.dump *
    process.kinLeptonFilterSequence * # agrohsje 
    process.PFTree
    )

#process.outpath    = cms.EndPath()


####################################################################
## Signal catcher for more information on errors and to resume jobs    


process.load("TopAnalysis.TopUtils.SignalCatcher_cfi")



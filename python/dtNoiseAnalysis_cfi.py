import FWCore.ParameterSet.Config as cms

dtNoiseAnalysisMonitor = cms.EDAnalyzer("DTNoiseAnalysisTest",
                                        noisyCellDef = cms.untracked.int32(500),
                                        doSynchNoise = cms.untracked.bool(False),
                                        maxSynchNoiseRate = cms.untracked.double(0.001),
                                        nEventsCert = cms.untracked.int32(1000)
                                        )




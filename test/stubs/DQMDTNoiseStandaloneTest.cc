/*
 * \file DTTestPulseRange.cc
 *
 * $Date: 2006/06/28 13:49:18 $
 * $Revision: 1.1 $
 * \author M. Zanetti - INFN Padova
 *
 */

#include <DQM/DTMonitorClient/test/stubs/DQMDTNoiseStandaloneTest.h>

// Framework
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/Handle.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Framework/interface/MakerMacros.h>
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include "FWCore/MessageLogger/interface/MessageLogger.h"

// Digis
#include <DataFormats/DTDigi/interface/DTDigi.h>
#include <DataFormats/DTDigi/interface/DTDigiCollection.h>
#include <DataFormats/MuonDetId/interface/DTLayerId.h>

// DQM
#include "DQMServices/QualityTests/interface/QCriterionRoot.h"


// DB
#include <CondFormats/DTObjects/interface/DTStatusFlag.h>
#include <CondFormats/DataRecord/interface/DTStatusFlagRcd.h>
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

// Geometry
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/DTGeometry/interface/DTLayer.h"
#include "DataFormats/MuonDetId/interface/DTWireId.h"
#include "DataFormats/MuonDetId/interface/DTSuperLayerId.h"
#include "DataFormats/MuonDetId/interface/DTChamberId.h"

// ROOT Staff
#include "TROOT.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TAxis.h"

using namespace edm;
using namespace std;


DQMDTNoiseStandaloneTest::DQMDTNoiseStandaloneTest(const edm::ParameterSet& ps): parameters(ps) {

  LogInfo("NoiseTestPrintOut")<<"[DQMDTNoiseStandaloneTest]: Constructor";

  outputFile = ps.getUntrackedParameter<string>("outputFile", "DTNoiseTest.root");

  criterionName = "noiseTest";

  // get hold of back-end interface
  dbe = edm::Service<DaqMonitorBEInterface>().operator->();

  // instantiate Monitor UI without connecting to any monitoring server
  // (i.e. "standalone mode")
  mui = new MonitorUIRoot();


}

DQMDTNoiseStandaloneTest::~DQMDTNoiseStandaloneTest() {

  if ( outputFile.size() != 0 ) dbe->save(outputFile);

  delete mui;
}


void DQMDTNoiseStandaloneTest::beginJob(const edm::EventSetup& context){

  LogInfo("NoiseTestPrintOut")<<"[DQMDTNoiseStandaloneTest]: Begin Job";
  
  nevents = 0;
  
  // Get the geometry
  context.get<MuonGeometryRecord>().get(muonGeom);
}


void DQMDTNoiseStandaloneTest::bookHistos(const DTLayerId& dtLayer) {

  LogInfo("NoiseTestPrintOut")<<"[DQMDTNoiseStandaloneTest]: Booking";

  stringstream wheel; wheel << dtLayer.wheel();	
  stringstream station; station << dtLayer.station();	
  stringstream sector; sector << dtLayer.sector();	
  stringstream superLayer; superLayer << dtLayer.superlayer();	
  stringstream layer; layer << dtLayer.layer();	

  string histoName =  
    + "NoiseOccupancy_W" + wheel.str() 
    + "_St" + station.str() 
    + "_Sec" + sector.str() 
    + "_SL" + superLayer.str() 
    + "_L" + layer.str();
  LogInfo("NoiseTestPrintOut")<<"[DQMDTNoiseStandaloneTest]: histoname "<<histoName;

  int nWires = muonGeom->layer(DTLayerId(dtLayer.wheel(),
					 dtLayer.station(),
					 dtLayer.sector(),
					 dtLayer.superlayer(),
					 dtLayer.layer()))->specificTopology().channels();
  
  occupancyHistos[DTLayerId(dtLayer.wheel(),
			    dtLayer.station(),
			    dtLayer.sector(),
			    dtLayer.superlayer(),
			    dtLayer.layer()).rawId()] = 
    dbe->book1D(histoName,histoName,nWires,0,nWires);

  histoNamesCollection.push_back(histoName);
  
}



void DQMDTNoiseStandaloneTest::analyze(const edm::Event& e, const edm::EventSetup& c){

  nevents++;
  if (nevents%1000 == 0) 
    LogInfo("NoiseTestPrintOut")<<"[DQMDTNoiseStandaloneTest]: event analyzed"<<nevents;

  // Create and associate the QT only when all the histos have been booked (hopefully)
  if (nevents%parameters.getUntrackedParameter<int>("eventToAnalyze", 30000) == 0) 
    createQualityTests();
    
  edm::Handle<DTDigiCollection> dtdigis;
  e.getByLabel("dtunpacker", dtdigis);
  
  DTDigiCollection::DigiRangeIterator dtLayerId_It;
  for (dtLayerId_It=dtdigis->begin(); dtLayerId_It!=dtdigis->end(); ++dtLayerId_It){
    
    for (DTDigiCollection::const_iterator digiIt = ((*dtLayerId_It).second).first;
	 digiIt!=((*dtLayerId_It).second).second; ++digiIt){
      
      // for clearness..
      uint32_t index = ((*dtLayerId_It).first).rawId();

      // Occupancies
      if (occupancyHistos.find(index) != occupancyHistos.end()) {
	occupancyHistos.find(index)->second->Fill((*digiIt).wire());
      } else {
	bookHistos((*dtLayerId_It).first);
	occupancyHistos.find(index)->second->Fill((*digiIt).wire());
      }
	
    }
  }
}

    
void DQMDTNoiseStandaloneTest::createQualityTests() {


  theNoiseTest = dynamic_cast<MENoisyChannelROOT*> (mui->createQTest(NoisyChannelROOT::getAlgoName(), 
								     criterionName ));
  
  for (vector<string>::iterator n_it = histoNamesCollection.begin(); 
       n_it != histoNamesCollection.end(); n_it++) 
    mui->useQTest((*n_it), criterionName);
  
  // set tolerance for noisy channel
  theNoiseTest->setTolerance(parameters.getUntrackedParameter<double>("tolerance",0.30));
  // set # of neighboring channels for calculating average (default: 1)
  theNoiseTest->setNumNeighbors(parameters.getUntrackedParameter<int>("neighboringChannels",2));
  
}



void DQMDTNoiseStandaloneTest::runTest() {

  mui->runQTests(); // mui->update() would have the same result

  // determine the "global" status of the system
  int status = mui->getSystemStatus();
  switch(status)
    {
    case dqm::qstatus::ERROR:
      LogError("NoiseTestPrintOut")<<" Error: Some channels have been found to be noisy";
      break;
    case dqm::qstatus::WARNING:
      LogWarning("NoiseTestPrintOut")<<" Warning: Some channels have been found to be noisy";
      cout << " Warning: Some channels have been found to be noisy";
      break;
    case dqm::qstatus::OTHER:
      LogWarning("NoiseTestPrintOut")<<"  Some tests did not run";
      break; 
    default:
      LogInfo("NoiseTestPrintOut")<<"  No channels have been found to be noisy";
    }

  // looping of the existent noise histograms
  for (map< uint32_t, MonitorElement*>::iterator h_it = occupancyHistos.begin();
       h_it != occupancyHistos.end(); h_it++) {

    const DTLayerId theLayer((*h_it).first);

    // get the Quality Tests report
    const QReport * theQReport = (*h_it).second->getQReport(criterionName);

    if(theQReport) {

      vector<dqm::me_util::Channel> badChannels = theQReport->getBadChannels();
      for (vector<dqm::me_util::Channel>::iterator ch_it = badChannels.begin(); 
	   ch_it != badChannels.end(); ch_it++) {
	
	theNoisyChannels.push_back(DTWireId(theLayer, (*ch_it).getBin()-1));

      }
    }
  }
  
}


void DQMDTNoiseStandaloneTest::endJob() {

  // run the tests
  runTest();

  string theTag = parameters.getUntrackedParameter<string>("theTag","commissioning_StatusFlag");
  DTStatusFlag * statusFlag = new DTStatusFlag(theTag);

  for ( vector<DTWireId>::iterator ch_it = theNoisyChannels.begin();
	ch_it != theNoisyChannels.end(); ch_it++) {
    LogWarning("NoiseTestPrintOut")<<" The noisy channels are:"<<(*ch_it);
    statusFlag->setCellNoise((*ch_it),true); 
  }

  // commit to the DB
//   edm::Service<cond::service::PoolDBOutputService> myDBService;
//   if( myDBService.isAvailable() ){
//     try{
//       myDBService->newValidityForNewPayload<DTStatusFlag>(statusFlag,myDBService->endOfTime());
//     }catch(const cond::Exception& er){
//       std::cout<<er.what()<<std::endl;
//     }catch(const std::exception& er){
//       std::cout<<"caught std::exception "<<er.what()<<std::endl;
//     }catch(...){
//       std::cout<<"Funny error"<<std::endl;
//     }
//   }else{
//     std::cout<<"Service is unavailable"<<std::endl;
//   }



  // commit to the DB
  edm::Service<cond::service::PoolDBOutputService> myDBservice;
  if( !myDBservice.isAvailable() ){
    std::cout<<"Service is unavailable"<<std::endl;
    return;
  }
  size_t callbackToken=myDBservice->callbackToken("DTStatusFlag");
  try{
    myDBservice->newValidityForNewPayload<DTStatusFlag>(statusFlag,myDBservice->endOfTime(),callbackToken);
  }catch(const cond::Exception& er){
    std::cout<<er.what()<<std::endl;
  }catch(const std::exception& er){
    std::cout<<"caught std::exception "<<er.what()<<std::endl;
  }catch(...){
    std::cout<<"Funny error"<<std::endl;
  }

}

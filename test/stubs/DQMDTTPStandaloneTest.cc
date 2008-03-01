/*
 * \file DTTestPulseRange.cc
 *
 * $Date: 2006/06/28 13:49:18 $
 * $Revision: 1.1 $
 * \author M. Zanetti - INFN Padova
 *
 */

#include <DQM/DTMonitorClient/test/stubs/DQMDTTPStandaloneTest.h>

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
#include "DQMServices/Core/interface/QTest.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"


// DB
#include <CondFormats/DTObjects/interface/DTRangeT0.h>
#include <CondFormats/DataRecord/interface/DTRangeT0Rcd.h>
#include <CondFormats/DTObjects/interface/DTT0.h>
#include <CondFormats/DataRecord/interface/DTT0Rcd.h>
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


DQMDTTPStandaloneTest::DQMDTTPStandaloneTest(const edm::ParameterSet& ps): parameters(ps) {

  LogInfo("TPTestPrintOut")<<"[DQMDTTPStandaloneTest]: Constructor";

  outputFile = ps.getUntrackedParameter<string>("outputFile", "DTTPTest.root");

  criterionName = "meanWithinTheRangeTest";

  // get hold of back-end interface
  dbe = edm::Service<DQMStore>().operator->();

  // instantiate Monitor UI without connecting to any monitoring server
  // (i.e. "standalone mode")
  mui = new DQMOldReceiver();


}

DQMDTTPStandaloneTest::~DQMDTTPStandaloneTest() {

  if ( outputFile.size() != 0 ) dbe->save(outputFile);

  delete mui;
}


void DQMDTTPStandaloneTest::beginJob(const edm::EventSetup& context){

  LogInfo("TPTestPrintOut")<<"[DQMDTTPStandaloneTest]: Begin Job";
  
  nevents = 0;
  
  // Get the geometry
  context.get<MuonGeometryRecord>().get(muonGeom);

  // Get the TP-range
  context.get<DTRangeT0Rcd>().get(tpRange);
}


void DQMDTTPStandaloneTest::bookHistos(const DTLayerId& dtLayer) {

  LogInfo("TPTestPrintOut")<<"[DQMDTTPStandaloneTest]: Booking";

  stringstream wheel; wheel << dtLayer.wheel();	
  stringstream station; station << dtLayer.station();	
  stringstream sector; sector << dtLayer.sector();	
  stringstream superLayer; superLayer << dtLayer.superlayer();	
  stringstream layer; layer << dtLayer.layer();	


  const int nWires = muonGeom->layer(DTLayerId(dtLayer.wheel(),
					       dtLayer.station(),
					       dtLayer.sector(),
					       dtLayer.superlayer(),
					       dtLayer.layer()))->specificTopology().channels();
  
    
  string histoName =  
    + "TPProfile_W" + wheel.str() 
    + "_St" + station.str() 
    + "_Sec" + sector.str() 
    + "_SL" + superLayer.str() 
    + "_L" + layer.str();
  
  // Setting the range 
  if ( parameters.getUntrackedParameter<bool>("readDB", false) ) 
    tpRange->slRangeT0( dtLayer.superlayerId() , tpValidRange.first, tpValidRange.second);

  cout<<"t0sRangeLowerBound "<<tpValidRange.first<<"; "
      <<"t0sRangeUpperBound "<<tpValidRange.second<<endl;
  
    
  testPulsesProfiles[DTLayerId(dtLayer.wheel(),
			       dtLayer.station(),
			       dtLayer.sector(),
			       dtLayer.superlayer(),
			       dtLayer.layer()).rawId()] =
    dbe->bookProfile(histoName,histoName,
		     nWires, 0, nWires, // Xaxis: channels
		     tpValidRange.first - tpValidRange.second, tpValidRange.first, tpValidRange.second); // Yaxis: times
  
  
  pair<int, int> pippo; 
  pippo = tpValidRange;

  histoNamesMap[histoName] = tpValidRange;


}



void DQMDTTPStandaloneTest::analyze(const edm::Event& e, const edm::EventSetup& c){

  nevents++;
  if (nevents%1000 == 0) 
    LogInfo("TPTestPrintOut")<<"[DQMDTTPStandaloneTest]: event analyzed"<<nevents;

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
      int layerIndex = ((*dtLayerId_It).first).rawId();
      
      
      // Profiles
      if (testPulsesProfiles.find(layerIndex) != testPulsesProfiles.end())
	testPulsesProfiles.find(layerIndex)->second->Fill((*digiIt).channel(),(*digiIt).countsTDC());
      else {
	bookHistos( (*dtLayerId_It).first );
	testPulsesProfiles.find(layerIndex)->second->Fill((*digiIt).channel(),(*digiIt).countsTDC());
      }
      
    }
  }
}

    
void DQMDTTPStandaloneTest::createQualityTests() {

  // loop over the histograms
  for (map<string, pair<int,int> >::iterator n_it = histoNamesMap.begin(); 
       n_it != histoNamesMap.end(); n_it++) {

    criterionName = (*n_it).first + "_test";

    testsWithinRange[criterionName] = dynamic_cast<ContentsProfWithinRange*> 
      (mui->createQTest(ContentsProfWithinRange::getAlgoName(), criterionName ));
    
    mui->useQTest((*n_it).first, criterionName);
    
    testsWithinRange[criterionName]->setMeanRange( ((*n_it).second).first, ((*n_it).second).second);
    
    testsWithinRange[criterionName]->setRMSRange(0,parameters.getUntrackedParameter<double>("RMSUpperBound",3));
  }

}



void DQMDTTPStandaloneTest::runTest() {

  mui->runQTests(); // mui->update() would have the same result

  // determine the "global" status of the system
  int status = mui->getSystemStatus();
  switch(status)
    {
    case dqm::qstatus::ERROR:
      LogError("TPTestPrintOut")<<" Error: Some channels have been found to be noisy";
      break;
    case dqm::qstatus::WARNING:
      LogWarning("TPTestPrintOut")<<" Warning: Some channels have been found to be noisy";
      cout << " Warning: Some channels have been found to be noisy";
      break;
    case dqm::qstatus::OTHER:
      LogWarning("TPTestPrintOut")<<"  Some tests did not run";
      break; 
    default:
      LogInfo("TPTestPrintOut")<<"  No channels have been found to be noisy";
    }

  // looping of the existent noise histograms
  for (map< uint32_t, MonitorElement*>::iterator h_it = testPulsesProfiles.begin();
       h_it != testPulsesProfiles.end(); h_it++) {

    const DTLayerId theLayer((*h_it).first);


    stringstream wheel; wheel << theLayer.wheel();	
    stringstream station; station << theLayer.station();	
    stringstream sector; sector << theLayer.sector();	
    stringstream superLayer; superLayer << theLayer.superlayer();	
    stringstream layer; layer << theLayer.layer();	

    criterionName =  
      + "TPProfile_W" + wheel.str() 
      + "_St" + station.str() 
      + "_Sec" + sector.str() 
      + "_SL" + superLayer.str() 
      + "_L" + layer.str() + "_test";
    

    // get the Quality Tests report
    const QReport * theQReport = (*h_it).second->getQReport(criterionName);

    if (theQReport) {

      vector<dqm::me_util::Channel> badChannels = theQReport->getBadChannels();
      for (vector<dqm::me_util::Channel>::iterator ch_it = badChannels.begin(); 
	   ch_it != badChannels.end(); ch_it++) {
	
	theBadChannels.insert(DTWireId(theLayer, (*ch_it).getBin()));

      }
    }
  }
  
}



void DQMDTTPStandaloneTest::endJob() {
  
  // run the tests
  runTest();

  string theTag = parameters.getUntrackedParameter<string>("theTag","commissioning_TP");
  DTT0 * t0 = new DTT0(theTag);
  

  // looping of the existent noise histograms
  for (map< uint32_t, MonitorElement*>::iterator h_it = testPulsesProfiles.begin();
       h_it != testPulsesProfiles.end(); h_it++) {

    const DTLayerId theLayer((*h_it).first);

    // FIXME: get the root obj to get the number of bins
    int nBins = 0;
    MonitorElementT<TNamed>* ob = dynamic_cast<MonitorElementT<TNamed>*> ((*h_it).second);
    if(ob) {
      TProfile * profile = dynamic_cast<TProfile *> (ob->operator->());
      if (profile) nBins = profile->GetNbinsX();
    }

    for (int b = 0; b < nBins; b++) {
      if ( theBadChannels.find(DTWireId(theLayer, b)) ==  theBadChannels.end() )
	t0->setCellT0(DTWireId(theLayer, b), 
		      ((*h_it).second)->getBinContent(b),  
		      ((*h_it).second)->getBinError(b));
    } 
  }
  
  
//   // commit to the DB
//   edm::Service<cond::service::PoolDBOutputService> myDBService;
//   if( myDBService.isAvailable() ){
//     try{
//       myDBService->newValidityForNewPayload<DTT0>(t0,myDBService->endOfTime());
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
  size_t callbackToken=myDBservice->callbackToken("DTT0");
  try{
    myDBservice->newValidityForNewPayload<DTT0>(t0,myDBservice->endOfTime(),callbackToken);
  }catch(const cond::Exception& er){
    std::cout<<er.what()<<std::endl;
  }catch(const std::exception& er){
    std::cout<<"caught std::exception "<<er.what()<<std::endl;
  }catch(...){
    std::cout<<"Funny error"<<std::endl;
  }

}

/*
 * \file DQMDTTPStandaloneTest.h
 *
 * $Date: 2006/05/24 17:21:37 $
 * $Revision: 1.4 $
 * \author M. Zanetti - INFN Padova
 *
*/

//#include "PluginManager/ModuleDef.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Handle.h>
#include <FWCore/Framework/interface/ESHandle.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/Framework/interface/MakerMacros.h>
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DQMServices/Core/interface/DaqMonitorBEInterface.h"
#include "DQMServices/Daemon/interface/MonitorDaemon.h"
#include "DQMServices/Core/interface/QTestStatus.h"
#include "DQMServices/Core/interface/DQMDefinitions.h"
#include "DQMServices/UI/interface/MonitorUIRoot.h"
#include "DQMServices/QualityTests/interface/QCriterionRoot.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

class DTGeometry;
class DTLayerId;
class DTWireId;
class DTRangeT0;

using namespace edm;
using namespace std;


class DQMDTTPStandaloneTest: public edm::EDAnalyzer{

public:
  
  /// Constructor
  DQMDTTPStandaloneTest(const edm::ParameterSet& ps);
  
  /// Destructor
  virtual ~DQMDTTPStandaloneTest();

protected:

  /// BeginJob (needed?)
  void beginJob(const edm::EventSetup& c);

  /// Book the ME
  void bookHistos(const DTLayerId& dtLayer);

  /// Analyze
  void analyze(const edm::Event& e, const edm::EventSetup& c);

  /// End job. Here write the bad channels on the DB
  void endJob();

  /// create the quality tests
  void createQualityTests(void);

  /// run quality tests;
  void runTest(void);

  /// show channels that failed test
  void showBadChannels(QCriterion *qc);

private:

  int nevents;
  string outputFile;
  string criterionName;

  edm::ParameterSet parameters;

  // the geometry
  edm::ESHandle<DTGeometry> muonGeom;

  // the TPRanges to be used (from DB)
  edm::ESHandle<DTRangeT0> tpRange;

  // back-end interface
  DaqMonitorBEInterface * dbe;

  // Monitor UI
  MonitorUserInterface * mui;

  // the range whithin looking for the TP peak
  pair<int,int> tpValidRange;

  // histograms: < DetID, Histogram >
  map< uint32_t , MonitorElement* > testPulsesProfiles;

  // map of histograms' names and tpRange: < histoname, tpValidityRange >
  map< string, pair<int,int> > histoNamesMap;

  // the collection of noisy channels
  set<DTWireId> theBadChannels;
  
  // quality tests: <  testName, test > (needed because of different tolerances) 
  map< string, ContentsProfWithinRangeROOT* > testsWithinRange;

};

DEFINE_FWK_MODULE(DQMDTTPStandaloneTest)

/*
 * \file DQMDTNoiseStandaloneTest.h
 *
 * $Date: 2006/08/10 16:27:18 $
 * $Revision: 1.2 $
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

#include "DQMServices/Core/interface/MonitorElement.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/QTestStatus.h"
#include "DQMServices/Core/interface/DQMDefinitions.h"
#include "DQMServices/Core/interface/DQMOldReceiver.h"
#include "DQMServices/Core/interface/QTest.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

class DTGeometry;
class DTLayerId;
class DTWireId;

using namespace edm;
using namespace std;


class DQMDTNoiseStandaloneTest: public edm::EDAnalyzer{

public:
  
  /// Constructor
  DQMDTNoiseStandaloneTest(const edm::ParameterSet& ps);
  
  /// Destructor
  virtual ~DQMDTNoiseStandaloneTest();

protected:

  /// BeginJob (needed?)
  void beginJob(const edm::EventSetup& c);

  /// Book the ME
  void bookHistos(const DTLayerId& dtLayer, string histoTag);

  /// Analyze
  void analyze(const edm::Event& e, const edm::EventSetup& c);

  /// End job. Here write the bad channels on the DB
  void endJob();

  /// create the quality tests
  void createQualityTests(void);

  /// tune cuts for quality tests
  void tuneCuts(void);

  /// run quality tests;
  void runDQMTest(void);
  void runStandardTest(void);

  /// show channels that failed test
  void drawSummaryNoise();

private:

  int nevents;
  string outputFile;
  string criterionName;

  edm::ParameterSet parameters;

  edm::ESHandle<DTGeometry> muonGeom;

  // back-end interface
  DQMStore * dbe;

  // Monitor UI
  DQMOldReceiver * mui;

  // histograms: < DetID, Histogram >
  std::map< uint32_t , MonitorElement* > occupancyHistos;

  // collection of histograms' names
  std::vector<string> histoNamesCollection;

  // the collection of noisy channels
  std::vector<DTWireId> theNoisyChannels;
  
  std::map<DTLayerId, float> noiseStatistics;

  std::map< uint32_t , MonitorElement* > noiseAverageHistos;

  // quality tests
  NoisyChannelROOT* theNoiseTest;

};

DEFINE_FWK_MODULE(DQMDTNoiseStandaloneTest)

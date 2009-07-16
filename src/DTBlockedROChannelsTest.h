#ifndef DTMonitorModule_DTBlockedROChannelsTest_H
#define DTMonitorModule_DTBlockedROChannelsTest_H

/** \class DTBlockedROChannelsTest
 * *
 *  DQM Client to Summarize LS by LS the status of the Read-Out channels.
 *
 *  $Date: 2009/05/20 14:17:01 $
 *  $Revision: 1.12 $
 *  \author G. Cerminara - University and INFN Torino
 *   
 */
#include <FWCore/Framework/interface/EDAnalyzer.h>
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include <FWCore/Framework/interface/Event.h>
#include "FWCore/Framework/interface/ESHandle.h"
#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/LuminosityBlock.h>
#include "DataFormats/MuonDetId/interface/DTChamberId.h"

class DQMStore;
class MonitorElement;
class DTReadOutMapping;

class DTBlockedROChannelsTest: public edm::EDAnalyzer{

public:

  /// Constructor
  DTBlockedROChannelsTest(const edm::ParameterSet& ps);

 /// Destructor
 ~DTBlockedROChannelsTest();

protected:

  /// BeginJob
  void beginJob(const edm::EventSetup& c);

  /// BeginRun
  void beginRun(const edm::Run& run, const edm::EventSetup& c);
 
  /// Analyze
  void analyze(const edm::Event& e, const edm::EventSetup& c);

  /// Endjob
  void endJob();

  void beginLuminosityBlock(edm::LuminosityBlock const& lumiSeg, edm::EventSetup const& context) ;

  /// DQM Client Diagnostic
  void endLuminosityBlock(edm::LuminosityBlock const& lumiSeg, edm::EventSetup const& c);

private:
  int readOutToGeometry(int dduId, int rosNumber, int& wheel, int& sector);

private:

  //Number of onUpdates
  int nupdates;

  // prescale on the # of LS to update the test
  int prescaleFactor;

  int nevents;
  unsigned int nLumiSegs;

  int run;


  DQMStore* dbe;
  edm::ESHandle<DTReadOutMapping> mapping;
  

  // Monitor Elements
  std::map<int, MonitorElement*> wheelHitos;
  MonitorElement *summaryHisto;



  class DTRobBinsMap {
  public:
    DTRobBinsMap(const int fed, const int ros, const DQMStore* dbe);

    DTRobBinsMap();


    ~DTRobBinsMap();
    
    // add a rob to the set of robs
    void addRobBin(int robBin);
    
    bool robChanged(int robBin);

    double getChamberPercentage();


    
  private:
    int getValueRobBin(int robBin) const;

    std::map<int, int> robsAndValues;

    const MonitorElement* meROS;

  };

  std::map<DTChamberId, DTRobBinsMap> chamberMap;

 };

#endif
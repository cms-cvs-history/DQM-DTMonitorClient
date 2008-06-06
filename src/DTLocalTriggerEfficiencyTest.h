#ifndef DTLocalTriggerEfficiencyTest_H
#define DTLocalTriggerEfficiencyTest_H


/** \class DTLocalTriggerEfficiencyTest
 * *
 *  DQM Test Client
 *
 *  $Date: 2008/05/22 10:49:59 $
 *  $Revision: 1.1 $
 *  \author  C. Battilana S. Marcellini - INFN Bologna
 *   
 */


#include "DQM/DTMonitorClient/src/DTLocalTriggerBaseTest.h"



class DTLocalTriggerEfficiencyTest: public DTLocalTriggerBaseTest{

public:

  /// Constructor
  DTLocalTriggerEfficiencyTest(const edm::ParameterSet& ps);
  
  /// Destructor
  virtual ~DTLocalTriggerEfficiencyTest();

protected:

  /// Book the new MEs (for each chamber)
  void bookChambHistos(DTChamberId chambId, std::string htype );

  /// Compute efficiency plots
  void makeEfficiencyME(TH1D* numerator, TH1D* denominator, MonitorElement* result);

  /// Compute 2D efficiency plots
  void makeEfficiencyME2D(TH2F* numerator, TH2F* denominator, MonitorElement* result);

  /// Begin Job
  void beginJob(const edm::EventSetup& c);

  /// DQM Client Diagnostic
  void endLuminosityBlock(edm::LuminosityBlock const& lumiSeg, edm::EventSetup const& c);



 private:

  std::map<uint32_t,std::map<std::string,MonitorElement*> > chambME;

};

#endif
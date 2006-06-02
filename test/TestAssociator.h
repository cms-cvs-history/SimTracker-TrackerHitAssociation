#ifndef TestAssociator_h
#define TestAssociator_h

/* \class TestAssociator
 *
 * \author Patrizia Azzi (INFN PD), Vincenzo Chiochia (Uni Zuerich)
 *
 *
 ************************************************************/

#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
 
#include "DataFormats/Common/interface/EDProduct.h"
 
#include "FWCore/ParameterSet/interface/ParameterSet.h"
 
//--- for SimHit
#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "Geometry/CommonDetUnit/interface/GeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/StripGeomDetUnit.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include "Geometry/CommonDetUnit/interface/GeomDetType.h"

#include "SimTracker/TrackerHitAssociation/interface/TrackerHitAssociator.h"                                                                                                                          
//needed for the geometry:
#include "DataFormats/DetId/interface/DetId.h"


class TestAssociator : public edm::EDAnalyzer
{
 public:
  
  explicit TestAssociator(const edm::ParameterSet& conf);
  
  virtual ~TestAssociator();

  virtual void analyze(const edm::Event& e, const edm::EventSetup& c);
  
  std::vector<PSimHit> matched;

 private:
  
  edm::ParameterSet conf_;
  const StripTopology* topol;
  int numStrips;    // number of strips in the module

};



#endif

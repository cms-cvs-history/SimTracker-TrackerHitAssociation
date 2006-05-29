// File: TestAssociator.cc
// Author:  P. Azzi
// Creation Date:  PA May 2006 Initial version.
//                 Pixel RecHits added by V.Chiochia - 18/5/06
//
//--------------------------------------------
#include <memory>
#include <string>
#include <iostream>

#include "SimTracker/TrackerHitAssociation/test/TestAssociator.h"

//--- for SimHit
#include "SimDataFormats/TrackingHit/interface/PSimHit.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

//--- for Strip RecHit
#include "DataFormats/SiStripCluster/interface/SiStripCluster.h"
#include "DataFormats/SiStripCluster/interface/SiStripClusterCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DLocalPosCollection.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DMatchedLocalPosCollection.h"
#include "DataFormats/Common/interface/OwnVector.h"

//--- for Pixel RecHit
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHitCollection.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelClusterCollection.h"

//--- for StripDigiSimLink
#include "SimDataFormats/TrackerDigiSimLink/interface/StripDigiSimLink.h"

//--- framework stuff
#include "FWCore/Framework/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

//--- for Geometry:
#include "DataFormats/DetId/interface/DetId.h"

#include "Geometry/Vector/interface/LocalPoint.h"
#include "Geometry/Vector/interface/GlobalPoint.h"

#include "Geometry/CommonDetUnit/interface/GeomDetType.h"
#include "Geometry/CommonDetUnit/interface/GeomDetUnit.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/TrackerNumberingBuilder/interface/GeometricDet.h"
#include "Geometry/CommonTopologies/interface/PixelTopology.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetType.h"
#include "Geometry/TrackerGeometryBuilder/interface/StripGeomDetType.h"
#include "Geometry/TrackerGeometryBuilder/interface/StripGeomDetUnit.h"

//---------------
// Constructor --
//---------------

using namespace std;
using namespace edm;

  void TestAssociator::analyze(const edm::Event& e, const edm::EventSetup& es) {
    
    using namespace edm;
    bool pixeldebug = true;
    int pixelcounter = 0;

    //std::string rechitProducer = conf_.getParameter<std::string>("RecHitProducer");
    
    // Step A: Get Inputs 
    edm::Handle<SiStripRecHit2DMatchedLocalPosCollection> rechitsmatched;
    edm::Handle<SiStripRecHit2DLocalPosCollection> rechitsrphi;
    edm::Handle<SiStripRecHit2DLocalPosCollection> rechitsstereo;
    edm::Handle<SiPixelRecHitCollection> pixelrechits;
    std::string  rechitProducer = "LocalMeasurementConverter";
    e.getByLabel(rechitProducer,"matchedRecHit", rechitsmatched);
    e.getByLabel(rechitProducer,"rphiRecHit", rechitsrphi);
    e.getByLabel(rechitProducer,"stereoRecHit", rechitsstereo);
    e.getByType(pixelrechits);
    
    //first instance tracking geometry
    edm::ESHandle<TrackerGeometry> pDD;
    es.get<TrackerDigiGeometryRecord> ().get (pDD);
    
    // loop over detunits
    for(TrackerGeometry::DetContainer::const_iterator it = pDD->dets().begin(); it != pDD->dets().end(); it++){
      uint32_t myid=((*it)->geographicalId()).rawId();       
      DetId detid = ((*it)->geographicalId());
      
      //construct the associator object
      TrackerHitAssociator  associate(e);
      
      edm::OwnVector<SiStripRecHit2DLocalPos> collector; 
      if(myid!=999999999){ //if is valid detector

	SiStripRecHit2DLocalPosCollection::range rechitRange = (rechitsrphi.product())->get((detid));
	SiStripRecHit2DLocalPosCollection::const_iterator rechitRangeIteratorBegin = rechitRange.first;
	SiStripRecHit2DLocalPosCollection::const_iterator rechitRangeIteratorEnd   = rechitRange.second;
	SiStripRecHit2DLocalPosCollection::const_iterator iter=rechitRangeIteratorBegin;

	SiPixelRecHitCollection::range pixelrechitRange = (pixelrechits.product())->get((detid));
	SiPixelRecHitCollection::const_iterator pixelrechitRangeIteratorBegin = pixelrechitRange.first;
	SiPixelRecHitCollection::const_iterator pixelrechitRangeIteratorEnd   = pixelrechitRange.second;
	SiPixelRecHitCollection::const_iterator pixeliter = pixelrechitRangeIteratorBegin;

	// Do the pixels
	for ( ; pixeliter != pixelrechitRangeIteratorEnd; ++pixeliter) {
	  pixelcounter++;
	  if(pixeldebug) {
	    cout << pixelcounter <<") Pixel RecHit DetId " << detid.rawId() << " Pos = " << pixeliter->localPosition() << endl;
	  }
	  matched.clear();
	  matched = associate.associateHit(*pixeliter);
	}

	// Do the strips
	for(iter=rechitRangeIteratorBegin;iter!=rechitRangeIteratorEnd;++iter){//loop on the rechit
	  SiStripRecHit2DLocalPos const rechit=*iter;
	  int i=0;
	  //--- add matching code here for testing
	  //cout << "---> try association RPHI! " << endl;
	  matched.clear();
	  matched = associate.associateHit(rechit);
	  if(!matched.empty()){
	    cout << " detector =  " << myid << " Rechit = " << rechit.localPosition() << endl; 
		    cout << " matched = " << matched.size() << endl;
	    for(vector<PSimHit>::const_iterator m=matched.begin(); m<matched.end(); m++){
	      cout << " hit  ID = " << (*m).trackId() << " Simhit x = " << (*m).localPosition() << endl;
	    }  
	  }
	  i++;
	} 
      }
    } 
    cout << " === calling end job " << endl;  
  }


TestAssociator::TestAssociator(edm::ParameterSet const& conf) : conf_(conf) 
{
  cout << " Constructor " << endl;
}

  TestAssociator::~TestAssociator() 
  {
    cout << " Destructor " << endl;
  }



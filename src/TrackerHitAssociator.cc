// File: TrackerHitAssociator.cc

#include <memory>
#include <string>
#include <vector>

#include "SimTracker/TrackerHitAssociation/interface/TrackerHitAssociator.h"
//--- for Geometry:
#include "DataFormats/Common/interface/Ref.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"

//for accumulate
#include <numeric>

using namespace std;
using namespace edm;


std::vector<PSimHit> TrackerHitAssociator::associateHit(const TrackingRecHit & thit) 
{
  
  //vector with the matched SimHit
  std::vector<PSimHit> result; 
  simtrackid.clear();
  
  //get the Detector type of the rechit
  DetId detid=  thit.geographicalId();
  uint32_t detID = detid.rawId();
  //cout << "Associator ---> get Detid " << detID << endl;
  //check we are in the strip tracker
  if(detid.subdetId() == StripSubdetector::TIB ||
     detid.subdetId() == StripSubdetector::TOB || 
     detid.subdetId() == StripSubdetector::TID ||
     detid.subdetId() == StripSubdetector::TEC) 
    {
      //check if it is a simple SiStripRecHit2DLocalPos
      if(const SiStripRecHit2DLocalPos * rechit = 
	 dynamic_cast<const SiStripRecHit2DLocalPos *>(&thit))
	{	  
	  simtrackid = associateSimpleRecHit(rechit);
	}
      //check if it is a matched SiStripRecHit2DMatchedLocalpos
      if(const SiStripRecHit2DMatchedLocalPos * rechit = 
	 dynamic_cast<const SiStripRecHit2DMatchedLocalPos *>(&thit))
	{	  
	  simtrackid = associateMatchedRecHit(rechit);
	}
    }
  //check we are in the pixel tracker
  if( detid.subdetId() == PixelSubdetector::PixelBarrel || 
      detid.subdetId() == PixelSubdetector::PixelEndcap) 
    {
      if(const SiPixelRecHit * rechit = dynamic_cast<const SiPixelRecHit *>(&thit))
	{	  
	  simtrackid = associatePixelRecHit(rechit);
	}
    }
  
  
  //now get the SimHit from the trackid
  vector<PSimHit> simHit; 
  std::map<unsigned int, std::vector<PSimHit> >::const_iterator it = SimHitMap.find(detID);
  simHit.clear();
  if (it!= SimHitMap.end()){
    simHit = it->second;
    vector<PSimHit>::const_iterator simHitIter = simHit.begin();
    vector<PSimHit>::const_iterator simHitIterEnd = simHit.end();
    for (;simHitIter != simHitIterEnd; ++simHitIter) {
      const PSimHit ihit = *simHitIter;
      unsigned int simHitid = ihit.trackId();
      for(size_t i=0; i<simtrackid.size();i++){
	//cout << " Associator -->  check sihit id's = " << simHitid << endl;
	//	if(simHitid == simtrackid[i] && simtrackid[i]!= 65535){ //exclude the geant particles. they all have the same id
	if(simHitid == simtrackid[i]){ //exclude the geant particles. they all have the same id
	  // cout << "Associator ---> ID" << ihit.trackId() << " Simhit x= " << ihit.localPosition().x() 
	  //	   << " y= " <<  ihit.localPosition().y() << " z= " <<  ihit.localPosition().x() << endl;	    
	  result.push_back(ihit);
	}
      }
    }
  }
  return result;  
}


std::vector<unsigned int> TrackerHitAssociator::associateHitId(const TrackingRecHit & thit) 
{
  
  //vector with the matched SimTrackID 
  simtrackid.clear();
  
  //get the Detector type of the rechit
  DetId detid=  thit.geographicalId();
  uint32_t detID = detid.rawId();
  //cout << "Associator ---> get Detid " << detID << endl;
  //check we are in the strip tracker
  if(detid.subdetId() == StripSubdetector::TIB ||
     detid.subdetId() == StripSubdetector::TOB || 
     detid.subdetId() == StripSubdetector::TID ||
     detid.subdetId() == StripSubdetector::TEC) 
    {
      //check if it is a simple SiStripRecHit2DLocalPos
      if(const SiStripRecHit2DLocalPos * rechit = 
	 dynamic_cast<const SiStripRecHit2DLocalPos *>(&thit))
	{	  
	  simtrackid = associateSimpleRecHit(rechit);
	}
      //check if it is a matched SiStripRecHit2DMatchedLocalpos
      if(const SiStripRecHit2DMatchedLocalPos * rechit = 
	 dynamic_cast<const SiStripRecHit2DMatchedLocalPos *>(&thit))
	{	  
	  simtrackid = associateMatchedRecHit(rechit);
	}
    }
  //check we are in the pixel tracker
  if( detid.subdetId() == PixelSubdetector::PixelBarrel || 
      detid.subdetId() == PixelSubdetector::PixelEndcap) 
    {
      if(const SiPixelRecHit * rechit = dynamic_cast<const SiPixelRecHit *>(&thit))
	{	  
	  simtrackid = associatePixelRecHit(rechit);
	}
    }
  //move here the choice of the id of the closest hit...??? 
  return simtrackid;  
}


std::vector<unsigned int>  TrackerHitAssociator::associateSimpleRecHit(const SiStripRecHit2DLocalPos * simplerechit)
{
  DetId detid=  simplerechit->geographicalId();
  uint32_t detID = detid.rawId();

  //to store temporary charge information
  std::vector<unsigned int> cache_simtrackid; 
  cache_simtrackid.clear();
  std::map<unsigned int, vector<float> > temp_simtrackid;
  float chg;
  temp_simtrackid.clear();

  edm::DetSetVector<StripDigiSimLink>::const_iterator isearch = stripdigisimlink->find(detID); 
  if(isearch != stripdigisimlink->end()) {  //if it is not empty
    //link_detset is a structure, link_detset.data is a std::vector<StripDigiSimLink>
    edm::DetSet<StripDigiSimLink> link_detset = (*stripdigisimlink)[detID];
    //cout << "Associator ---> get digilink! in Detid n = " << link_detset.data.size() << endl;
    
    const edm::Ref<edm::DetSetVector<SiStripCluster>, SiStripCluster, edm::refhelper::FindForDetSetVector<SiStripCluster> > clust=simplerechit->cluster();
    //cout << "Associator ---> get cluster info " << endl;
    int clusiz = clust->amplitudes().size();
    int first  = clust->firstStrip();     
    int last   = first + clusiz;
    float cluchg = std::accumulate(clust->amplitudes().begin(), clust->amplitudes().end(),0);
    // cout << "Associator ---> Clus size = " << clusiz << " first = " << first << "  last = " << last << "  tot charge = " << cluchg << endl;
    
    for(edm::DetSet<StripDigiSimLink>::const_iterator linkiter = link_detset.data.begin(); linkiter != link_detset.data.end(); linkiter++){
      StripDigiSimLink link = *linkiter;
      if( link.channel() >= first  && link.channel() < last ){
	cache_simtrackid.push_back(link.SimTrackId());
	//get the charge released in the cluster
	chg = 0;
	int mychan = link.channel()-first;
	chg = (clust->amplitudes()[mychan])*link.fraction();
	temp_simtrackid[link.SimTrackId()].push_back(chg);
      }
    }
    
    vector<float> tmpchg;
    float simchg;
    float simfraction;
    std::map<float, unsigned int> temp_map;
    
    //loop over the unique ID's
    vector<unsigned int>::iterator new_end = unique(cache_simtrackid.begin(),cache_simtrackid.end());
    for(vector<unsigned int>::iterator i=cache_simtrackid.begin(); i != new_end; i++){
      std::map<unsigned int, vector<float> >::const_iterator it = temp_simtrackid.find(*i);
      if(it != temp_simtrackid.end()){
	tmpchg = it->second;
	for(size_t ii=0; ii<tmpchg.size(); ii++){
	  simchg +=tmpchg[ii];
	}
	simfraction = simchg/cluchg;
	//cout << " Track id = " << *i << " Total fraction = " << simfraction << endl;
	temp_map.insert(std::pair<float, unsigned int> (simfraction,*i));
      }
    }	
    //copy the list of ID's ordered in the charge fraction 
    for(std::map<float , unsigned int>::const_iterator it = temp_map.begin(); it!=temp_map.end(); it++){
      //      cout << " Final simtrackid list = " << it->second << endl;
      //      if(it->second > 50000) std::cout << " Secondary simtrackid = " << it->second << endl;
      simtrackid.push_back(it->second);
    }
  }    
  
  return simtrackid;
  
}


std::vector<unsigned int>  TrackerHitAssociator::associateMatchedRecHit(const SiStripRecHit2DMatchedLocalPos * matchedrechit)
{
  //to be written
  return simtrackid;
}

std::vector<unsigned int>  TrackerHitAssociator::associatePixelRecHit(const SiPixelRecHit * pixelrechit)
{
  //
  // Pixel associator: work in progress...
  //
  DetId detid=  pixelrechit->geographicalId();
  uint32_t detID = detid.rawId();
  edm::DetSetVector<PixelDigiSimLink>::const_iterator isearch = pixeldigisimlink->find(detID); 
  if(isearch != pixeldigisimlink->end()) {  //if it is not empty
    edm::DetSet<PixelDigiSimLink> link_detset = (*pixeldigisimlink)[detID];
    edm::Ref< edm::DetSetVector<SiPixelCluster>, SiPixelCluster> const& cluster = pixelrechit->cluster();
    int minPixelRow = (*cluster).minPixelRow();
    int maxPixelRow = (*cluster).maxPixelRow();
    int minPixelCol = (*cluster).minPixelCol();
    int maxPixelCol = (*cluster).maxPixelCol();    
    //std::cout << "    Cluster minRow " << minPixelRow << " maxRow " << maxPixelRow << std::endl;
    //std::cout << "    Cluster minCol " << minPixelCol << " maxCol " << maxPixelCol << std::endl;
    edm::DetSet<PixelDigiSimLink>::const_iterator linkiter = link_detset.data.begin();
    int dsl = 0;
    unsigned int simtrackid_cache = 9999999;
    for( ; linkiter != link_detset.data.end(); linkiter++) {
      dsl++;
      std::pair<int,int> pixel_coord = PixelDigi::channelToPixel(linkiter->channel());
      //std::cout << "    " << dsl << ") Digi link: row " << pixel_coord.first << " col " << pixel_coord.second << std::endl;      
      if(  pixel_coord.first  <= maxPixelRow && 
	   pixel_coord.first  >= minPixelRow &&
	   pixel_coord.second <= maxPixelCol &&
	   pixel_coord.second >= minPixelCol ) {
	//std::cout << "      !-> trackid   " << linkiter->SimTrackId() << endl;
	//std::cout << "          fraction  " << linkiter->fraction()   << endl;
	if(linkiter->SimTrackId() != simtrackid_cache) {  // Add each trackid only once
	  simtrackid.push_back(linkiter->SimTrackId());
	  //cout    << "          Adding TrackId " << linkiter->SimTrackId() << endl;
	  simtrackid_cache = linkiter->SimTrackId();
	}
      } 
    }
  }
  
  return simtrackid;
}

//constructor
TrackerHitAssociator::TrackerHitAssociator(const edm::Event& e)  : myEvent_(e)  {
  //  using namespace edm;
  
  //get stuff from the event
  //changed to detsetvector
  //edm::Handle< edm::DetSetVector<StripDigiSimLink> >  stripdigisimlink;
  //    e.getByLabel("stripdigi", "stripdigi", stripdigisimlink);
  e.getByLabel("stripdigi", stripdigisimlink);
  e.getByLabel("pixdigi",   pixeldigisimlink);
  //cout << "Associator : get digilink from the event" << endl;
  
  theStripHits.clear();
  edm::Handle<edm::PSimHitContainer> TIBHitsLowTof;
  edm::Handle<edm::PSimHitContainer> TIBHitsHighTof;
  edm::Handle<edm::PSimHitContainer> TIDHitsLowTof;
  edm::Handle<edm::PSimHitContainer> TIDHitsHighTof;
  edm::Handle<edm::PSimHitContainer> TOBHitsLowTof;
  edm::Handle<edm::PSimHitContainer> TOBHitsHighTof;
  edm::Handle<edm::PSimHitContainer> TECHitsLowTof;
  edm::Handle<edm::PSimHitContainer> TECHitsHighTof;
  
  e.getByLabel("SimG4Object","TrackerHitsTIBLowTof", TIBHitsLowTof);
  e.getByLabel("SimG4Object","TrackerHitsTIBHighTof", TIBHitsHighTof);
  e.getByLabel("SimG4Object","TrackerHitsTIDLowTof", TIDHitsLowTof);
  e.getByLabel("SimG4Object","TrackerHitsTIDHighTof", TIDHitsHighTof);
  e.getByLabel("SimG4Object","TrackerHitsTOBLowTof", TOBHitsLowTof);
  e.getByLabel("SimG4Object","TrackerHitsTOBHighTof", TOBHitsHighTof);
  e.getByLabel("SimG4Object","TrackerHitsTECLowTof", TECHitsLowTof);
  e.getByLabel("SimG4Object","TrackerHitsTECHighTof", TECHitsHighTof);
  
  theStripHits.insert(theStripHits.end(), TIBHitsLowTof->begin(), TIBHitsLowTof->end()); 
  theStripHits.insert(theStripHits.end(), TIBHitsHighTof->begin(), TIBHitsHighTof->end());
  theStripHits.insert(theStripHits.end(), TIDHitsLowTof->begin(), TIDHitsLowTof->end()); 
  theStripHits.insert(theStripHits.end(), TIDHitsHighTof->begin(), TIDHitsHighTof->end());
  theStripHits.insert(theStripHits.end(), TOBHitsLowTof->begin(), TOBHitsLowTof->end()); 
  theStripHits.insert(theStripHits.end(), TOBHitsHighTof->begin(), TOBHitsHighTof->end());
  theStripHits.insert(theStripHits.end(), TECHitsLowTof->begin(), TECHitsLowTof->end()); 
  theStripHits.insert(theStripHits.end(), TECHitsHighTof->begin(), TECHitsHighTof->end());
  
  SimHitMap.clear();
  for (std::vector<PSimHit>::iterator isim = theStripHits.begin();
       isim != theStripHits.end(); ++isim){
    SimHitMap[(*isim).detUnitId()].push_back((*isim));
  }
  
  thePixelHits.clear();
  
  edm::Handle<edm::PSimHitContainer> PixelBarrelHitsLowTof;
  edm::Handle<edm::PSimHitContainer> PixelBarrelHitsHighTof;
  edm::Handle<edm::PSimHitContainer> PixelEndcapHitsLowTof;
  edm::Handle<edm::PSimHitContainer> PixelEndcapHitsHighTof;
  
  e.getByLabel("SimG4Object","TrackerHitsPixelBarrelLowTof", PixelBarrelHitsLowTof);
  e.getByLabel("SimG4Object","TrackerHitsPixelBarrelHighTof", PixelBarrelHitsHighTof);
  e.getByLabel("SimG4Object","TrackerHitsPixelEndcapLowTof", PixelEndcapHitsLowTof);
  e.getByLabel("SimG4Object","TrackerHitsPixelEndcapHighTof", PixelEndcapHitsHighTof);
  
  thePixelHits.insert(thePixelHits.end(), PixelBarrelHitsLowTof->begin(), PixelBarrelHitsLowTof->end()); 
  thePixelHits.insert(thePixelHits.end(), PixelBarrelHitsHighTof->begin(), PixelBarrelHitsHighTof->end());
  thePixelHits.insert(thePixelHits.end(), PixelEndcapHitsLowTof->begin(), PixelEndcapHitsLowTof->end()); 
  thePixelHits.insert(thePixelHits.end(), PixelEndcapHitsHighTof->begin(), PixelEndcapHitsHighTof->end());

  for (std::vector<PSimHit>::iterator isim = thePixelHits.begin();
       isim != thePixelHits.end(); ++isim){
    SimHitMap[(*isim).detUnitId()].push_back((*isim));
  }
  
}



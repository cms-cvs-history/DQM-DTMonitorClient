
/*
 *  See header file for a description of this class.
 *
 *  $Date: 2008/07/02 16:32:48 $
 *  $Revision: 1.1 $
 *  \author G. Cerminara - INFN Torino
 */

#include "DTOccupancyClusterBuilder.h"

#include "TCanvas.h"
#include "TH2F.h"

#include <sstream>
#include <iostream>

using namespace std;


DTOccupancyClusterBuilder::  DTOccupancyClusterBuilder() : maxMean(-1.),
							   maxRMS(-1.) {
  debug = false; //FIXME: remove it

}

DTOccupancyClusterBuilder::~DTOccupancyClusterBuilder(){}



void DTOccupancyClusterBuilder::addPoint(const DTOccupancyPoint& point) {
  // loop over points already stored
  for(set<DTOccupancyPoint>::const_iterator pt = thePoints.begin(); pt != thePoints.end(); ++pt) {
    theDistances[(*pt).distance(point)] = make_pair(*pt, point);
  }
  //   cout << "[DTOccupancyClusterBuilder] Add point with mean: " << point.mean()
  //        << " RMS: " << point.rms() << endl;
  thePoints.insert(point);
}


void DTOccupancyClusterBuilder::buildClusters() {
  //   cout << "[DTOccupancyClusterBuilder] buildClusters" << endl;
  while(buildNewCluster()) {
    //     cout << "New cluster builded" << endl;
    //     cout << "# of remaining points: " << thePoints.size() << endl;
    if(thePoints.size() <= 1) break;
  }
    
  // build single point clusters with the remaining points
  for(set<DTOccupancyPoint>::const_iterator pt = thePoints.begin(); pt != thePoints.end();
      ++pt) {
    DTOccupancyCluster clusterCandidate(*pt);
    theClusters.push_back(clusterCandidate);
    // store the range for building the histograms later
    if(clusterCandidate.maxMean() > maxMean) maxMean = clusterCandidate.maxMean();
    if(clusterCandidate.maxRMS() > maxRMS) maxRMS = clusterCandidate.maxRMS();
  }
  if(debug) cout << " # of valid clusters: " << theClusters.size() << endl;
  sortClusters();
  
}


void DTOccupancyClusterBuilder::drawClusters(std::string canvasName) {
  int nBinsX = 100;
  int nBinsY = 100;
  int colorMap[12] = {632, 600, 800, 400, 820, 416, 432, 880, 616, 860, 900, 920};

  //   cout << "Draw clusters: " << endl;
  //   cout << "    max mean: " << maxMean << " max rms: " << maxRMS << endl;

  TCanvas *canvas = new TCanvas(canvasName.c_str(),canvasName.c_str()); 
  canvas->cd();
  for(vector<DTOccupancyCluster>::const_iterator cluster = theClusters.begin();
      cluster != theClusters.end(); ++cluster) {
    stringstream stream;
    stream << canvasName << "_" << cluster-theClusters.begin();
    string histoName = stream.str();
    TH2F *histo = (*cluster).getHisto(histoName, nBinsX, 0, maxMean+3*maxMean/100.,
				      nBinsY, 0, maxRMS+3*maxRMS/100., colorMap[cluster-theClusters.begin()]);
    if(cluster == theClusters.begin()) 
      histo->Draw("box");
    else
      histo->Draw("box,same");
  }
}


std::pair<DTOccupancyPoint, DTOccupancyPoint> DTOccupancyClusterBuilder::getInitialPair() {
  return theDistances.begin()->second;
}

void DTOccupancyClusterBuilder::computePointToPointDistances() {
  theDistances.clear();
  for(set<DTOccupancyPoint>::const_iterator pt_i = thePoints.begin(); pt_i != thePoints.end();
      ++pt_i) { // i loopo
    for(set<DTOccupancyPoint>::const_iterator pt_j = thePoints.begin(); pt_j != thePoints.end();
	++pt_j) { // j loop
      if(*pt_i != *pt_j) {
	theDistances[pt_i->distance(*pt_j)] = make_pair(*pt_i, *pt_j);
      }
    }
  }
}



void DTOccupancyClusterBuilder::computeDistancesToCluster(const DTOccupancyCluster& cluster) {
  theDistancesFromTheCluster.clear();
  for(set<DTOccupancyPoint>::const_iterator pt = thePoints.begin(); pt != thePoints.end(); ++pt) {
    theDistancesFromTheCluster[cluster.distance(*pt)] = *pt;
  }
}


bool DTOccupancyClusterBuilder::buildNewCluster() {
  if(debug) cout << "--------- New Cluster Candidate ----------------------" << endl;
  pair<DTOccupancyPoint, DTOccupancyPoint> initialPair = getInitialPair();
  if(debug) cout << "   Initial Pair: " << endl;
  if(debug) cout << "           point1: mean " << initialPair.first.mean()
       << " rms " << initialPair.first.rms() << endl;
  if(debug) cout << "           point2: mean " << initialPair.second.mean()
       << " rms " << initialPair.second.rms() << endl;
  DTOccupancyCluster clusterCandidate(initialPair.first, initialPair.second);
  if(clusterCandidate.isValid()) {
    //     cout <<   " cluster candidate is valid" << endl;
    // remove already used pair
    thePoints.erase(initialPair.first);
    thePoints.erase(initialPair.second);
    if(thePoints.size() != 0) {
      computeDistancesToCluster(clusterCandidate);
      while(clusterCandidate.addPoint(theDistancesFromTheCluster.begin()->second)) {
	thePoints.erase(theDistancesFromTheCluster.begin()->second);
	if(thePoints.size() ==0) break;
	computeDistancesToCluster(clusterCandidate);
      }
    }
  } else {
    return false;
  }
  theClusters.push_back(clusterCandidate);
  // store the range for building the histograms later
  if(clusterCandidate.maxMean() > maxMean) maxMean = clusterCandidate.maxMean();
  if(clusterCandidate.maxRMS() > maxRMS) maxRMS = clusterCandidate.maxRMS();
  computePointToPointDistances();
  return true;
}
  


void DTOccupancyClusterBuilder::sortClusters() {
  if(debug) cout << " sorting" << endl;
  sort(theClusters.begin(), theClusters.end(), clusterIsLessThan);
  // we save the detid of the clusters which are not the best one
  for(vector<DTOccupancyCluster>::const_iterator cluster = ++(theClusters.begin());
      cluster != theClusters.end(); ++cluster) { // loop over clusters skipping the first
    set<DTLayerId> clusterLayers = (*cluster).getLayerIDs();
    if(debug) cout << "     # layers in the cluster: " << clusterLayers.size() << endl;
    theProblematicLayers.insert(clusterLayers.begin(), clusterLayers.end());
  }
  if(debug) cout << " # of problematic layers: " << theProblematicLayers.size() << endl;
}


DTOccupancyCluster DTOccupancyClusterBuilder::getBestCluster() const {
  return theClusters.front();
}

bool DTOccupancyClusterBuilder::isProblematic(DTLayerId layerId) const {
  if(theProblematicLayers.find(layerId) != theProblematicLayers.end()) {
    return true;
  }
  return false;
}
//construct tracker misalignment document from displacement data

//usage: call it with a plane configuration file,
//root -l 'constructTrackerMisalignments.C("planes.txt")'

void constructTrackerMisalignments(std::string fname = "planes.txt"){
  
  ofstream out;
  std::string outname = "TrackerMisalignments-" + fname;
  //check if this file exist already, remove the file if it does
  auto exist = static_cast<bool>(std::ifstream(outname));
  if(exist) std::remove(outname.c_str());
  out.open(outname,std::ios_base::app);

  ///////// ******************* ////////////////
  ///////// TRACK MISALIGNMENTS ////////////////
  ///////// ******************* ////////////////

  //This is to be added later, for now no misalignments
  out<<"# Tracker with some misalignments"<<std::endl;
  out<< "#"<<std::endl;
  out<< "#"<<std::endl;
  out<< "#"<<std::endl;
  out<<"TABLE TrkAlignTracker"<<std::endl;
  out<<"0, 0_0_0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0"<<std::endl;


  ///////// ******************* ////////////////
  ///////// PLANE MISALIGNMENTS ////////////////
  ///////// ******************* ////////////////

  //This is to be added later, for now no misalignments
  out<< "#"<<std::endl;
  out<< "#"<<std::endl;
  out<< "#"<<std::endl;
  out<<"TABLE TrkAlignPlane"<<std::endl;
  out<<"#row, strawid, dx, dy, dz, rx, ry, rz"<<std::endl;
  std::string PlaneId;
  for(int ipl=0;ipl<36;ipl++){
    PlaneId = std::to_string(ipl) + "_0_0";
    out<< ipl<<","<<PlaneId<<","<< 0. << "," << 0. << "," << 0. << ","
                                << 0. << "," << 0. << "," << 0. <<std::endl;     
  } 

  ///////// ******************* ////////////////
  ///////// PANEL MISALIGNMENTS ////////////////
  ///////// ******************* ////////////////

  //This is to be added later, for now no misalignments
  out<< "#"<<std::endl;
  out<< "#"<<std::endl;
  out<< "#"<<std::endl;
  out<<"TABLE TrkAlignPanel"<<std::endl;
  out<<"#row, strawid, dU, dV, dW, rU, rV, rW"<<std::endl;

  std::string PanelId;
  for(int ipl=0;ipl<36;ipl++){
    for(int ipnl=0;ipnl<6;ipnl++){
      PanelId = std::to_string(ipl) + "_" + std::to_string(ipnl) + "_0";
      out<< ipl*6 + ipnl <<","<<PanelId<<","<< 0. << "," << 0. << "," << 0. << ","
                                            << 0. << "," << 0. << "," << 0. <<std::endl;     
    }
  } 


  ///////// ******************* ////////////////
  ///////// STRAW MISALIGNMENTS ////////////////
  ///////// ******************* ////////////////
  std::string p;
  ifstream inPlanes;
  ifstream inDisplacements;
  inPlanes.open(fname);

  std::array<std::string,6> panels;
  std::string StrawId;
  std::string trail_inDisplacements = "/displacements.txt";

  int plane =0;
  int strawNo = 0;
  double wire_cal_dV,wire_cal_dW,wire_hv_dV,wire_hv_dW,straw_cal_dV,straw_cal_dW,straw_hv_dV,straw_hv_dW;
  
  out<< "#"<<std::endl;
  out<< "#"<<std::endl;
  out<< "#"<<std::endl;
  out<<"TABLE TrkAlignStraw"<<std::endl;
  out<<"#index,StrawId,wire_cal_dV,wire_cal_dW,wire_hv_dV,wire_hv_dW,straw_cal_dV,straw_cal_dW,straw_hv_dV,straw_hv_dW"<<std::endl;

  while(!inPlanes.eof()){

    inPlanes >> plane >> panels.at(0) >> panels.at(1) >> panels.at(2) >> panels.at(3) >> panels.at(4) >> panels.at(5);
    if(inPlanes.eof()) break;
    for(int ix = 0; ix<panels.size();ix++){

      //Fill in misalignment data according to the list provided, MN999 is used for non-existing planes
      if(panels.at(ix) == "MN999"){

        for(int is=0;is<96;is++){

          StrawId = std::to_string(plane) + "_" + std::to_string(plane*6 + ix) + "_" + std::to_string(is);
          out<< (plane*6+ix)*96 + is <<","<<StrawId<<","<< 0. << "," << 0. <<","
                                                        << 0. << "," << 0. <<","
                                                        << 0. << "," << 0. <<","
                                                        << 0. << "," << 0. << std::endl;
        }
      }
      else{

        std::cout<<"Fetching straw misalignments for "<<panels.at(ix)<<std::endl;

        std::string filename = panels.at(ix) + trail_inDisplacements;
        inDisplacements.open(filename.c_str());
        while(!inDisplacements.eof()){
    
          inDisplacements >> strawNo >> wire_cal_dV >> wire_cal_dW >> wire_hv_dV >> wire_hv_dW >> straw_cal_dV >> straw_cal_dW >>  straw_hv_dV >> straw_hv_dW;
          if(inDisplacements.eof()) break;
            StrawId = std::to_string(plane) + "_" + std::to_string(plane*6 + ix) + "_" + std::to_string(strawNo);
            out<< (plane*6+ix)*96 + strawNo  <<","<<StrawId<<","<< wire_cal_dV << "," << wire_cal_dW <<","
                                                                << wire_hv_dV << "," << wire_hv_dW <<","
                                                                << straw_cal_dV << "," << straw_cal_dW <<","
                                                                << straw_hv_dV << "," << straw_hv_dW << std::endl;

        }
        inDisplacements.close();

        }

      }

    }
    inPlanes.close();
    

}


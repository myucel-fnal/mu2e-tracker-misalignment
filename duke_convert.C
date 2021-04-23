//duke x-ray converter

#include "fit.h"
#include "Straw.h"

ROOT::Math::SMatrix<double,3> CalculateRotationMatrix(ROOT::Math::SVector<double,3> hv_, ROOT::Math::SVector<double,3> mid_, ROOT::Math::SVector<double,3> cal_){

  using namespace ROOT::Math;

  auto hsurvey = new TH3F("hsurvey","hsurvey",100,0,1400,100,0,375,100,-0.1,0.1);
  hsurvey->Fill(cal_(0),cal_(1),cal_(2));
  hsurvey->Fill(hv_(0),hv_(1),hv_(2));
  hsurvey->Fill(mid_(0),mid_(1),mid_(2));
  auto rotation = fit(hsurvey);
  std::cout<<"angle between planes(degrees) = "<<TMath::RadToDeg()*rotation.angle<<std::endl;
  SVector<double,3> u(rotation.axis.X(),rotation.axis.Y(),rotation.axis.Z());

  auto uxu = TensorProd(u, u);
  double ux_[9] = {0.,-rotation.axis.Z(),rotation.axis.Y(),rotation.axis.Z(),0.,-rotation.axis.X(),-rotation.axis.Y(),rotation.axis.X(),0.};
  SMatrix<double,3> ux(ux_,9);
  SMatrix<double,3> I =  SMatrixIdentity();

  //std::cout<<"u : "<<u<<std::endl;
  //std::cout<<"uxu : "<<uxu<<std::endl;
  //std::cout<<"ux : "<<ux<<std::endl;

  SMatrix<double,3> R = (TMath::Cos(rotation.angle)*I) + (TMath::Sin(rotation.angle)*ux) + ((1-TMath::Cos(rotation.angle))*uxu);
  //std::cout<<"R : "<<R<<std::endl;

  return R;

}


void duke_convert(std::string panels = "panels.txt"){

  using namespace ROOT::Math;

  std::vector<std::string> panel_list;


  std::string p;
  ifstream inList;
  inList.open(panels);
  while(!inList.eof()){

    inList >> p;
    panel_list.push_back(p);

  }
    
  for(auto panel : panel_list){

    std::string trail_in = "/TrackingDB_BC.txt";
    std::string trail_out = "/displacements.txt";
    std::string filename = panel + trail_in;
    std::string outname = panel + trail_out;

  ifstream in1;
  in1.open(filename.c_str());
  std::cout<<"/////////////////////////////////////////////////////"<<std::endl;
  std::cout<<"//////    PROCESSING PANEL "<<panel<<"    ///////////"<<std::endl;
  std::cout<<"/////////////////////////////////////////////////////"<<std::endl;

  SVector<double,3> scal;//Cal side survey coor
  SVector<double,3> shv;// HV side survey coor
  SVector<double,3> sm1;// Middle survey meas 1 coor
  SVector<double,3> sm2;// Middle survey meas 2 coor
  SVector<double,3> sm;// Middle survey coor
  SVector<double,3> eshv(0.0, 0.0, 0.0);//Expected HV side survey coor
  SVector<double,3> esm(689.539, 374.374, 0.0);// Expected Middle side survey coor
  SVector<double,3> escal(1379.078, 0.0, 0.0);// Expected Cal survey meas coor

  double y0 = -67.826;//straw/wire center for straw #0 with respect to survey holes, in mm.
  double straw0_y0 = 380.;// from docdb 888
  double straw95_y0 = 676.88;//  from docdb 888
  double spacing = (straw95_y0-straw0_y0)/95.;
  double z0 = 0.55;//straw/wire center, in mm NEED TO BE CONFIRMED

  //initialize straws
  std::array<Straw*,96> straws;
  for(int i=0;i<96;i++) straws.at(i) = new Straw(i);

  //read through Duke's data and fill straws
  size_t line = 0;
  int measType_, strawNo_, measNo_;
  double x_, y_, z_;

  while(!in1.eof()){
    
    if (line < 4){

      in1 >> x_ >> y_ >> z_;
      std::vector<double> v_ = {x_, y_, z_};
      if( line == 0) shv.SetElements(v_.begin(),v_.end());
      else if( line == 1) sm1.SetElements(v_.begin(),v_.end());
      else if( line == 2) sm2.SetElements(v_.begin(),v_.end());
      else scal.SetElements(v_.begin(),v_.end());
      v_.clear();

    }
    else{

      in1 >> measType_ >> strawNo_ >> measNo_>> x_ >> y_ >> z_;
      if(x_ == -100 || y_ == -100 || z_ == -100) continue;
      straws.at(strawNo_)->AddMeas(measType_,measNo_,x_,y_,z_);

      }

    line++;
  }  

  //Translate survey coors to expected coors where cal side is at 0,0,0
  auto T = shv;
  scal = scal - T;
  sm = sm1 + sm2; sm *= 0.5; sm = sm - T;
  shv = shv - T;

  std::cout<<"Translated survey coordinates"<<std::endl;
  std::cout<<shv<<std::endl;
  std::cout<<sm<<std::endl;
  std::cout<<scal<<std::endl;

  //check plane angle with respect to expected
  //Rotation...
  auto R = CalculateRotationMatrix(shv,sm,scal);
  shv = R*shv;
  sm = R*sm;
  scal = R*scal;

  //std::cout<<"Rotated survey coordinates"<<std::endl;
  //std::cout<<shv<<std::endl;
  //std::cout<<sm<<std::endl;
  //std::cout<<scal<<std::endl;


  //Translate & Rotate straw measurements
  std::vector<ROOT::Math::SVector<double,3>> new_wirecenters;
  std::vector<ROOT::Math::SVector<double,3>> new_strawcenters;
  for(auto& straw:straws){

    for(auto center:straw->GetWireCenter()) {
      center = R*center - T;
      new_wirecenters.push_back(center);
    }
    straw->SetWireCenter(new_wirecenters);
    new_wirecenters.clear();

    for(auto center:straw->GetStrawCenter()) {
      center = R*center - T;
      new_strawcenters.push_back(center);
    }
    straw->SetStrawCenter(new_strawcenters);
    new_strawcenters.clear();

  }

  //Calculate End Points
  for(auto& straw:straws) straw->CalculateEndPoints();

  //Let's draw the translated points
  auto grPointsXY = new TGraph();
  auto grPointsYZ = new TGraph();
  for(auto straw:straws){

    auto wirecenters = straw->GetWireCenter();
    for(auto center:wirecenters) {
      grPointsXY->SetPoint(grPointsXY->GetN(),center(0),center(1));
      grPointsYZ->SetPoint(grPointsYZ->GetN(),center(1),center(2));
    }

  }
  auto cPoints = new TCanvas("cPoints","cPoints",1200,900);
  cPoints->Divide(1,2);
  cPoints->cd(1);
  grPointsXY->Draw("AC");
  cPoints->cd(2);
  grPointsYZ->SetMarkerStyle(kFullDotLarge);
  grPointsYZ->SetMarkerSize(0.4);
  grPointsYZ->Draw("AP");

  cPoints->Close();


  //Let's draw the U,V,W(x,y,z) differences
  auto hHvWireEndPointY = new TH1F("hHvWireEndPointV","hHvWireEndPointV",100,-5,5);
  auto hHvWireEndPointZ = new TH1F("hHvWireEndPointW","hHvWireEndPointW",100,-5,5);
  auto hCalWireEndPointY = new TH1F("hCalWireEndPointV","hCalWireEndPointV",100,-5,5);
  auto hCalWireEndPointZ = new TH1F("hCalWireEndPointW","hCalWireEndPointW",100,-5,5);
  auto hHvStrawEndPointY = new TH1F("hHvStrawEndPointV","hHvStrawEndPointV",100,-5,5);
  auto hHvStrawEndPointZ = new TH1F("hHvStrawEndPointW","hHvStrawEndPointW",100,-5,5);
  auto hCalStrawEndPointY = new TH1F("hCalStrawEndPointV","hCalStrawEndPointV",100,-5,5);
  auto hCalStrawEndPointZ = new TH1F("hCalStrawEndPointW","hCalStrawEndPointW",100,-5,5);

  std::array<double,4> WireEndPoints;
  std::array<double,4> StrawEndPoints;
  std::array<double,4> NominalEndPoints;

  for(auto straw:straws){

    WireEndPoints = straw->GetWireEndPoints();//HvY,HvZ,CalY,CalZ
    StrawEndPoints = straw->GetStrawEndPoints();//HvY,HvZ,CalY,CalZ
    NominalEndPoints = straw->GetNominalEndPoints();//HvNomX,CalNomX,NomY,NomZ

    hHvWireEndPointY->Fill(NominalEndPoints.at(2)-WireEndPoints.at(0));
    //std::cout<<"hHvWireEndPointY : "<<NominalEndPoints.at(2)<<"\t"<<WireEndPoints.at(0)<<std::endl;
    hHvWireEndPointZ->Fill(NominalEndPoints.at(3)-WireEndPoints.at(1));
    //std::cout<<"hHvWireEndPointZ : "<<NominalEndPoints.at(3)<<"\t"<<WireEndPoints.at(1)<<std::endl;
    hCalWireEndPointY->Fill(NominalEndPoints.at(2)-WireEndPoints.at(2));
    //std::cout<<"hCalWireEndPointY : "<<NominalEndPoints.at(2)<<"\t"<<WireEndPoints.at(2)<<std::endl;
    hCalWireEndPointZ->Fill(NominalEndPoints.at(3)-WireEndPoints.at(3));
    //std::cout<<"hCalWireEndPointZ : "<<NominalEndPoints.at(3)<<"\t"<<WireEndPoints.at(3)<<std::endl;
    hHvStrawEndPointY->Fill(NominalEndPoints.at(2)-StrawEndPoints.at(0));
    //std::cout<<"hHvStrawEndPointY : "<<NominalEndPoints.at(2)<<"\t"<<StrawEndPoints.at(0)<<std::endl;
    hHvStrawEndPointZ->Fill(NominalEndPoints.at(3)-StrawEndPoints.at(1));
    //std::cout<<"hHvStrawEndPointZ : "<<NominalEndPoints.at(3)<<"\t"<<StrawEndPoints.at(1)<<std::endl;
    hCalStrawEndPointY->Fill(NominalEndPoints.at(2)-StrawEndPoints.at(2));
    //std::cout<<"hCalStrawEndPointY : "<<NominalEndPoints.at(2)<<"\t"<<StrawEndPoints.at(2)<<std::endl;
    hCalStrawEndPointZ->Fill(NominalEndPoints.at(3)-StrawEndPoints.at(3));
    //std::cout<<"hCalStrawEndPointZ : "<<NominalEndPoints.at(3)<<"\t"<<StrawEndPoints.at(3)<<std::endl;


  }

  //Mean and std
  std::cout<<"Wire Z displacement;"<<std::endl;
  std::cout<<"hHvWireEndPointZ Mean: "<<hHvWireEndPointZ->GetMean()<<"Std dev : "<<hHvWireEndPointZ->GetRMS()<<std::endl;
  std::cout<<"hCalWireEndPointZ Mean: "<<hCalWireEndPointZ->GetMean()<<"Std dev : "<<hCalWireEndPointZ->GetRMS()<<std::endl;
  std::cout<<"Straw Z displacement;"<<std::endl;
  std::cout<<"hHvStrawEndPointZ Mean: "<<hHvStrawEndPointZ->GetMean()<<"Std dev : "<<hHvStrawEndPointZ->GetRMS()<<std::endl;
  std::cout<<"hCalStrawEndPointZ Mean: "<<hCalStrawEndPointZ->GetMean()<<"Std dev : "<<hCalStrawEndPointZ->GetRMS()<<std::endl;

  auto cEndPoints = new TCanvas("cEndPoints","cEndPoints",1200,900);
  cEndPoints->Divide(2,4);
  cEndPoints->cd(1);
  hHvWireEndPointY->Draw("");
  cEndPoints->cd(2);
  hHvWireEndPointZ->Draw("");
  cEndPoints->cd(3);
  hCalWireEndPointY->Draw("");
  cEndPoints->cd(4);
  hCalWireEndPointZ->Draw("");
  cEndPoints->cd(5);
  hHvStrawEndPointY->Draw("");
  cEndPoints->cd(6);
  hHvStrawEndPointZ->Draw("");
  cEndPoints->cd(7);
  hCalStrawEndPointY->Draw("");
  cEndPoints->cd(8);
  hCalStrawEndPointZ->Draw("");

  cEndPoints->Close();

  //Create output
  ofstream out;
  out.open(outname.c_str());
  double wire_cal_dV,wire_cal_dW,wire_hv_dV,wire_hv_dW,straw_cal_dV,straw_cal_dW,straw_hv_dV,straw_hv_dW;
  for(auto straw:straws){


    WireEndPoints = straw->GetWireEndPoints();//HvY,HvZ,CalY,CalZ
    StrawEndPoints = straw->GetStrawEndPoints();//HvY,HvZ,CalY,CalZ
    NominalEndPoints = straw->GetNominalEndPoints();//HvNomX,CalNomX,NomY,NomZ

    wire_cal_dV = WireEndPoints.at(2) == -100 ? -10000 : NominalEndPoints.at(2)-WireEndPoints.at(2);
    wire_cal_dW = WireEndPoints.at(3) == -100 ? -10000 : NominalEndPoints.at(3)-WireEndPoints.at(3);
    wire_hv_dV = WireEndPoints.at(0) == -100 ? -10000 : NominalEndPoints.at(2)-WireEndPoints.at(0);
    wire_hv_dW = WireEndPoints.at(1) == -100 ? -10000 : NominalEndPoints.at(3)-WireEndPoints.at(1);
    straw_cal_dV = StrawEndPoints.at(2) == -100 ? -10000 : NominalEndPoints.at(2)-StrawEndPoints.at(2);
    straw_cal_dW = StrawEndPoints.at(3) == -100 ? -10000 : NominalEndPoints.at(3)-StrawEndPoints.at(3);
    straw_hv_dV = StrawEndPoints.at(0) == -100 ? -10000 : NominalEndPoints.at(2)-StrawEndPoints.at(0);
    straw_hv_dW = StrawEndPoints.at(1) == -100 ? -10000 : NominalEndPoints.at(3)-StrawEndPoints.at(1);

    out<< straw->GetStrawNo() <<" "<< wire_cal_dV << " " << wire_cal_dW <<" "
                                   << wire_hv_dV << " " << wire_hv_dW <<" "
                                   << straw_cal_dV <<" " << straw_cal_dW <<" "
                                   << straw_hv_dV <<" " << straw_hv_dW << std::endl;
  }
  out.close();

  std::cout<<"//////    FINISHED    ///////////"<<std::endl;
  std::cout<<"////"<<std::endl;
  std::cout<<"//"<<std::endl;
  std::cout<<"/"<<std::endl;

  }

}


class Straw{

public:

  Straw(int n) : StrawNo(n){

  	NomHvX = SurveyMiddleX - std::pow(std::pow(InnerRadius,2)-std::pow(Straw0Y+(SpacingY*n),2),0.5);
  	NomCalX = SurveyMiddleX + std::pow(std::pow(InnerRadius,2)-std::pow(Straw0Y+(SpacingY*n),2),0.5);
  	NomY = Straw0SurveyY + SpacingY*n;
  	NomZ = n%2 == 0 ? StrawMidZ + SpacingZ/2. + StrawRadius : StrawMidZ - SpacingZ/2. - StrawRadius;

  };
  Straw();
  ~Straw();

  void SetStrawNo(int n){StrawNo = n;};
  int GetStrawNo(){return StrawNo;};
  void AddMeas(int type, int meas, double mx, double my, double mz){
    if(type == 0){
      ROOT::Math::SVector<double,3> v(mx,my,mz);
      WireCenters.push_back(v);      
    }
    else if(type == 1) {
      ROOT::Math::SVector<double,3> v(mx,my,mz);
      StrawCenters.push_back(v);  
    }
    else if(type == 2){
    	if(meas < StrawCenters.size()){
      		StrawCenters.at(meas)(0) = (StrawCenters.at(meas)(0) + mx)/2.;
      		StrawCenters.at(meas)(1) = (StrawCenters.at(meas)(1) + my)/2.;
      		StrawCenters.at(meas)(2) = (StrawCenters.at(meas)(2) + mz)/2.;
    	}

    }
  };
  std::vector<ROOT::Math::SVector<double,3>> GetStrawCenter(){return StrawCenters;};
  std::vector<ROOT::Math::SVector<double,3>> GetWireCenter(){return WireCenters;};
  void SetStrawCenter(std::vector<ROOT::Math::SVector<double,3>> sc){StrawCenters = sc;};
  void SetWireCenter(std::vector<ROOT::Math::SVector<double,3>> wc){WireCenters = wc;};


  void CalculateEndPoints(){

  	auto grY = new TGraph();
  	auto grZ = new TGraph();
  	double success;

  	// Calculate end points for wires

  	for(auto center:WireCenters){
  		grY->SetPoint(grY->GetN(),center(0),center(1));
  		grZ->SetPoint(grZ->GetN(),center(0),center(2));
  	}
  	success = grY->Fit("pol1","Q");
  	if(success < 0) {

      HvWireEndPointY = -100;
      CalWireEndPointY = -100;
      std::cout<<"linear fit failed on wire "<<StrawNo<<" X-Y distribution"<<std::endl;

    }
    else{

      HvWireEndPointY = grY->GetFunction("pol1")->Eval(NomHvX);
      CalWireEndPointY = grY->GetFunction("pol1")->Eval(NomCalX);

    }


  	success = grZ->Fit("pol1","Q");
  	if(success < 0) {

    HvWireEndPointZ = -100;
    CalWireEndPointZ = -100;
    std::cout<<"linear fit failed on wire "<<StrawNo<<" X-Z distribution"<<std::endl;

    }
    else{

      HvWireEndPointZ = grZ->GetFunction("pol1")->Eval(NomHvX);
      CalWireEndPointZ = grZ->GetFunction("pol1")->Eval(NomCalX);

    }

  	grY->Clear();
  	grZ->Clear();
  		
  	// Calculate end points for straws
  	for(auto center:StrawCenters){
  		grY->SetPoint(grY->GetN(),center(0),center(1));
  		grZ->SetPoint(grZ->GetN(),center(0),center(2));
  	}

  	success = grY->Fit("pol1","Q");
  	if(success < 0) {

      HvStrawEndPointY = -100;
      CalStrawEndPointY = -100;
      std::cout<<"linear fit failed on straw "<<StrawNo<<" X-Y distribution"<<std::endl;

    }
    else{
      
      HvStrawEndPointY = grY->GetFunction("pol1")->Eval(NomHvX);
      CalStrawEndPointY = grY->GetFunction("pol1")->Eval(NomCalX);

    }

  	success = grZ->Fit("pol1","Q");
  	if(success < 0) {

      HvStrawEndPointZ = -100;
      CalStrawEndPointZ = -100;
      std::cout<<"linear fit failed on straw "<<StrawNo<<" X-Z distribution"<<std::endl;
    }
    else{

      HvStrawEndPointZ = grZ->GetFunction("pol1")->Eval(NomHvX);
      CalStrawEndPointZ = grZ->GetFunction("pol1")->Eval(NomCalX);

    }


  	grY->Clear();
  	grZ->Clear();

  }
  std::array<double,4> GetWireEndPoints() {
  	std::array<double,4> endpoints{ {HvWireEndPointY,HvWireEndPointZ,CalWireEndPointY,CalWireEndPointZ} };
  	return endpoints;
  };
  std::array<double,4> GetStrawEndPoints() {
  	std::array<double,4> endpoints{ {HvStrawEndPointY,HvStrawEndPointZ,CalStrawEndPointY,CalStrawEndPointZ} };
  	return endpoints;
  };
  std::array<double,4> GetNominalEndPoints() {
  	std::array<double,4> endpoints{ {NomHvX,NomCalX,NomY,NomZ} };
  	return endpoints;
  };



private:

  std::vector<ROOT::Math::SVector<double,3>> StrawCenters;
  std::vector<ROOT::Math::SVector<double,3>> WireCenters;
  double HvWireEndPointY;
  double HvWireEndPointZ;
  double CalWireEndPointY;
  double CalWireEndPointZ;
  double HvStrawEndPointY;
  double HvStrawEndPointZ;
  double CalStrawEndPointY;
  double CalStrawEndPointZ;
  double NomHvX;//nominal
  double NomCalX;//nominal
  double NomY;// nominal
  double NomZ;// nominal
  int StrawNo;
  // all dimensions from doc.db 888
  const double InnerRadius = 700.;//in mm
  const double SurveyMiddleX = 689.539;
  const double Straw0Y = 380.-67.826;//in mm
  const double Straw0SurveyY = -67.826;//in mm
  const double SpacingY = 6.25/2.;// in mm
  const double StrawMidZ = 0.5112;// in mm
  //const double StrawMidZ = 0.55;// in mm
  const double SpacingZ = 0.41;// in mm
  const double StrawRadius = 2.5;// in mm

};
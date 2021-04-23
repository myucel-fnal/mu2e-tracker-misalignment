// function to determine plane angle

#include "TMath.h"
TF3 *fitFcn;

Double_t fitFunction(Double_t *x, Double_t *par){
  
  Float_t xx =x[0];
  Float_t yy =x[1];
  Float_t zz =x[2];
  return par[0]*xx + par[1]*yy + par[2]*zz + par[3];

}

struct Rotation{

	double angle;
	TVector3 axis;

}rotation;

bool DoFit(const char* fitter, TVirtualPad *pad, TH3F *g) {
	
	gRandom = new TRandom3();
	ROOT::Math::MinimizerOptions::SetDefaultMinimizer(fitter);
	//pad->SetGrid();
	fitFcn->Update();
	std::string title = std::string(fitter) + " fit bench";

	TString fitterType(fitter);

	bool ok = true;
	
  fitFcn->SetParameter(0,0.e-11);
  fitFcn->SetParameter(1,0.e-11);
  fitFcn->SetParameter(2,7.05e-07);
  fitFcn->SetParameter(3,1.);

	TFitResultPtr fr = g->Fit(fitFcn,"WMSQ");
  std::cout<<"fit result = "<<fr<<" chi2 = "<<fr->Chi2()<<std::endl;

	TVector3 n1(0,0,1);
  TVector3 n2(fitFcn->GetParameter(0),fitFcn->GetParameter(1),fitFcn->GetParameter(2));

  double angle = n1.Angle(n2);
  rotation.angle = angle;
  rotation.axis = n2.Cross(n1);
  
  //std::cout<<"angle between n1 and u = "<<TMath::RadToDeg()*n1.Angle(rotation.axis)<<std::endl;
  //std::cout<<"angle between n2 and u = "<<TMath::RadToDeg()*n2.Angle(rotation.axis)<<std::endl;
  //std::cout<<"angle between n1 and n2 = "<<TMath::RadToDeg()*n1.Angle(n2)<<std::endl;

	//std::cout<<"angle between planes(cosQ) = "<<TMath::Cos(angle)<<std::endl;
	//std::cout<<"angle between planes(radians) = "<<angle<<std::endl;
	//std::cout<<"angle between planes(degrees) = "<<TMath::RadToDeg()*angle<<std::endl;

	return true;
}


Rotation fit(TH3F *gr){

  
  std::cout<<"Finding angle between planes"<<std::endl;
  TH1::AddDirectory(kFALSE);

  fitFcn = new TF3("fitFcn",fitFunction,0.,1500.,200.,700.,-0.1,0.1,4,3);
  fitFcn->SetDrawOption("same");
  fitFcn->SetClippingBoxOn();
  fitFcn->SetLineColorAlpha(kBlue,0.35);
  fitFcn->SetFillColorAlpha(kWhite,0.35);
  fitFcn->SetNpx(100);
	

  gStyle->SetOptFit();

  DoFit("Minuit2",gPad,gr);
  return rotation;
  
}

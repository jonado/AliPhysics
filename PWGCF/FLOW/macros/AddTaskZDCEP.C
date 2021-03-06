AliAnalysisTask * AddTaskZDCEP(TString ZDCCalibFileName,
                               TString trigger="MB",
                               TString dataset="pass1",
                               TString Label="",
                               const char* suffix="")
{
  // the manager is static, so get the existing manager via the static method
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    printf("No analysis manager to connect to!\n");
    return NULL;
  }

  // just to see if all went well, check if the input event handler has been connected
  if (!mgr->GetInputEventHandler()) {
    printf("This task requires an input event handler!\n");
    return NULL;
  }

  // get the default name of the output file ("AnalysisResults.root")
  TString file = "AnalysisResults";

  // get the common input container from the analysis manager
  AliAnalysisDataContainer *cinput = mgr->GetCommonInputContainer();

  // create the flow analysis tasks
  TString AnalysisTaskName = "AnalysisTaskZDCEP";
  AnalysisTaskName += Label;
  AnalysisTaskName += suffix;
  AliAnalysisTaskZDCEP *taskZDC = new AliAnalysisTaskZDCEP(AnalysisTaskName);
  if(trigger=="MB") taskZDC->SelectCollisionCandidates(AliVEvent::kINT7);
  if(dataset=="pass1") taskZDC->SetDataSet(AliAnalysisTaskZDCEP::k2015o_pass1_pass1pidfix);
  if(trigger=="muon_single") taskZDC->SelectCollisionCandidates(AliVEvent::kMuonSingleHighPt7 | AliVEvent::kMuonSingleLowPt7);
  if(dataset=="muon_calo_pass1") taskZDC->SetDataSet(AliAnalysisTaskZDCEP::k2015o_muon_calo_pass1);

  // add list for ZDC towers gain equalization
  TString ZDCTowerEqFileName = "";
  if(dataset=="pass1") ZDCTowerEqFileName = "alien:///alice/cern.ch/user/j/jmargutt/15oHI_EZDCcalib.root";
  if(dataset=="muon_calo_pass1") ZDCTowerEqFileName = "alien:///alice/cern.ch/user/j/jmargutt/15oHI_muon_calo_pass1_EZDCcalib.root";
  TFile* ZDCTowerEqFile = TFile::Open(ZDCTowerEqFileName,"READ");
  if(!ZDCTowerEqFile) {
    cout << "ERROR: ZDC tower equalisation: file not found!" << endl;
    exit(1);
  }
  gROOT->cd();
  TList* ZDCTowerEqList = (TList*)(ZDCTowerEqFile->FindObjectAny("EZNcalib"));
  if(ZDCTowerEqList) {
    taskZDC->SetTowerEqList(ZDCTowerEqList);
    cout << "ZDC tower equalisation: set! (from " <<  ZDCTowerEqFileName.Data() << ")" << endl;
  } else {
    cout << "ERROR: ZDC tower equalisation: EZNcalib TList not found!" << endl;
    exit(1);
  }
  delete ZDCTowerEqFile;

  if(ZDCCalibFileName!="") {
    // add list for ZDC Q-vector re-centering
    TFile* ZDCCalibFile = TFile::Open(ZDCCalibFileName,"READ");
    if(!ZDCCalibFile) {
      cout << "ERROR: ZDC Q-vector calibration: file not found!" << endl;
      exit(1);
    }
    TList* ZDCCalibList = dynamic_cast<TList*>(ZDCCalibFile->FindObjectAny("Q Vectors"));
    if(ZDCCalibList) {
      taskZDC->SetZDCCalibList(ZDCCalibList);
      cout << "ZDC Q-vector calibration: set! (from " <<  ZDCCalibFileName.Data() << ")" << endl;
    } else {
      cout << "ERROR: ZDC Q-vector calibration: Q Vectors TList not found!" << endl;
      exit(1);
    }
    delete ZDCCalibFile;
  }

  // connect the task to the analysis manager
  mgr->AddTask(taskZDC);

  // create a data container for the output of the flow event task
  TString taskZDCEPname = "ZDCEPExchangeContainer";
  taskZDCEPname += Label;
  taskZDCEPname += suffix;
  AliAnalysisDataContainer *coutputFE = mgr->CreateContainer(taskZDCEPname,
    AliFlowEventSimple::Class(),
    AliAnalysisManager::kExchangeContainer);
    // connect the input data to the flow event task
    mgr->ConnectInput(taskZDC,0,cinput);
    // and connect the output to the flow event task
    mgr->ConnectOutput(taskZDC,1,coutputFE);

    // QA OUTPUT CONTAINER
    TString taskZDCQAname = "AnalysisResults.root:ZDCQA";
    taskZDCQAname += Label;
    taskZDCQAname += suffix;
    AliAnalysisDataContainer* coutputZDCQA = mgr->CreateContainer(taskZDCQAname.Data(),
    TList::Class(),
    AliAnalysisManager::kOutputContainer,
    taskZDCQAname);
    // and connect the qa output container to the flow event.
    // this container will be written to the output file
    mgr->ConnectOutput(taskZDC,2,coutputZDCQA);

    return taskZDC;
  }

let uploadCSV='';let cleanedCSV='';let originalCSV='';let comparisonData=null;let auditOperations=[];let currentColumnTypes=[];
const sampleDatasets={airbnb:'bathrooms,bedrooms,beds,locationname,price,rating\n1,1,1,atlanta,38,y\n1,1,2,san diego,150,y\n2,2,3,new orleans,175,y\n',credit:'age,debtratio,monthlyincome,seriousdlqin2yrs\n45,0.80,9120,1\n40,0.12,2600,0\n38,0.09,3042,0\n',titanic:'passengerid,survived,pclass,age,sex,fare\n1,0,3,22.0,male,7.25\n2,1,1,38.0,female,71.28\n3,1,3,26.0,female,7.93\n'};

// Wire all event listeners on DOMContentLoaded so we can remove 'unsafe-inline'
// from the Content-Security-Policy header.
document.addEventListener('DOMContentLoaded',function(){
  // --- welcome screen mode selection ---
  var btnCsv=document.getElementById('btnCsvMode');
  var btnSemi=document.getElementById('btnSemiMode');
  var btnText=document.getElementById('btnTextMode');
  if(btnCsv)btnCsv.addEventListener('click',selectStructured);
  if(btnSemi)btnSemi.addEventListener('click',selectSemiStructured);
  if(btnText)btnText.addEventListener('click',selectUnstructured);

  // --- CSV upload area ---
  var uploadArea=document.getElementById('uploadArea');
  if(uploadArea){
    uploadArea.addEventListener('dragover',handleDragOver);
    uploadArea.addEventListener('dragleave',handleDragLeave);
    uploadArea.addEventListener('drop',handleDrop);
  }
  var btnCsvFile=document.getElementById('btnCsvFile');
  if(btnCsvFile)btnCsvFile.addEventListener('click',function(){document.getElementById('fileInput').click();});
  var btnPasteCsv=document.getElementById('btnPasteCsv');
  if(btnPasteCsv)btnPasteCsv.addEventListener('click',pasteFromClipboard);
  var fileInput=document.getElementById('fileInput');
  if(fileInput)fileInput.addEventListener('change',handleFileUpload);

  // --- sample datasets ---
  var btnAirbnb=document.getElementById('btnSampleAirbnb');
  var btnCredit=document.getElementById('btnSampleCredit');
  var btnTitanic=document.getElementById('btnSampleTitanic');
  if(btnAirbnb)btnAirbnb.addEventListener('click',function(){loadSampleDataset('airbnb');});
  if(btnCredit)btnCredit.addEventListener('click',function(){loadSampleDataset('credit');});
  if(btnTitanic)btnTitanic.addEventListener('click',function(){loadSampleDataset('titanic');});

  // --- clean button ---
  var btnClean=document.getElementById('btnQuickClean');
  if(btnClean)btnClean.addEventListener('click',performQuickClean);

  // --- audit log toggle ---
  var auditToggle=document.getElementById('auditLogToggle');
  if(auditToggle){
    auditToggle.addEventListener('click',function(){
      var d=document.getElementById('auditLogDisplay');
      var c=d.style.maxHeight==='none';
      d.style.maxHeight=c?'400px':'none';
      auditToggle.setAttribute('aria-expanded',!c);
    });
    auditToggle.addEventListener('keydown',function(e){
      if(e.key==='Enter'||e.key===' '){e.preventDefault();auditToggle.click();}
    });
  }

  // --- pagination ---
  var prevBtn=document.getElementById('prevPageBtn');
  var nextBtn=document.getElementById('nextPageBtn');
  if(prevBtn)prevBtn.addEventListener('click',previousPage);
  if(nextBtn)nextBtn.addEventListener('click',nextPage);

  // --- new session ---
  var btnNewSession=document.getElementById('btnNewSession');
  if(btnNewSession)btnNewSession.addEventListener('click',startNewSession);

  // --- JSON upload area ---
  var jsonArea=document.getElementById('jsonUploadArea');
  if(jsonArea){
    jsonArea.addEventListener('dragover',handleJsonDragOver);
    jsonArea.addEventListener('dragleave',handleJsonDragLeave);
    jsonArea.addEventListener('drop',handleJsonDrop);
  }
  var btnJsonFile=document.getElementById('btnJsonFile');
  if(btnJsonFile)btnJsonFile.addEventListener('click',function(){document.getElementById('jsonFileInput').click();});
  var btnPasteJson=document.getElementById('btnPasteJson');
  if(btnPasteJson)btnPasteJson.addEventListener('click',pasteJsonFromClipboard);
  var jsonFileInput=document.getElementById('jsonFileInput');
  if(jsonFileInput)jsonFileInput.addEventListener('change',handleJsonFileUpload);

  // --- XML upload area ---
  var xmlArea=document.getElementById('xmlUploadArea');
  if(xmlArea){
    xmlArea.addEventListener('dragover',handleXmlDragOver);
    xmlArea.addEventListener('dragleave',handleXmlDragLeave);
    xmlArea.addEventListener('drop',handleXmlDrop);
  }
  var btnXmlFile=document.getElementById('btnXmlFile');
  if(btnXmlFile)btnXmlFile.addEventListener('click',function(){document.getElementById('xmlFileInput').click();});
  var btnPasteXml=document.getElementById('btnPasteXml');
  if(btnPasteXml)btnPasteXml.addEventListener('click',pasteXmlFromClipboard);
  var xmlFileInput=document.getElementById('xmlFileInput');
  if(xmlFileInput)xmlFileInput.addEventListener('change',handleXmlFileUpload);

  // --- Text upload area ---
  var textArea=document.getElementById('textUploadArea');
  if(textArea){
    textArea.addEventListener('dragover',handleTextDragOver);
    textArea.addEventListener('dragleave',handleTextDragLeave);
    textArea.addEventListener('drop',handleTextDrop);
  }
  var btnTextFile=document.getElementById('btnTextFile');
  if(btnTextFile)btnTextFile.addEventListener('click',function(){document.getElementById('textFileInput').click();});
  var btnPasteText=document.getElementById('btnPasteText');
  if(btnPasteText)btnPasteText.addEventListener('click',pasteTextFromClipboard);
  var textFileInput=document.getElementById('textFileInput');
  if(textFileInput)textFileInput.addEventListener('change',handleTextFileUpload);

  // --- download / back buttons (semi-structured results) ---
  var btnDlJson=document.getElementById('btnDlJson');
  if(btnDlJson)btnDlJson.addEventListener('click',downloadSemiStructured);
  var btnDlXml=document.getElementById('btnDlXml');
  if(btnDlXml)btnDlXml.addEventListener('click',downloadXml);
  var btnBack1=document.getElementById('btnBackSemi');
  if(btnBack1)btnBack1.addEventListener('click',backToHome);

  // --- download / back buttons (unstructured results) ---
  var btnDlText=document.getElementById('btnDlText');
  if(btnDlText)btnDlText.addEventListener('click',downloadText);
  var btnBack2=document.getElementById('btnBackText');
  if(btnBack2)btnBack2.addEventListener('click',backToHome);
});

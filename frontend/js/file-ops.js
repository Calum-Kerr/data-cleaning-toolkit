function handleDragOver(e){e.preventDefault();document.getElementById('uploadArea').classList.add('drag-over');}
function handleDragLeave(e){e.preventDefault();document.getElementById('uploadArea').classList.remove('drag-over');}
function handleDrop(e){
  e.preventDefault();document.getElementById('uploadArea').classList.remove('drag-over');
  const files=e.dataTransfer.files;
  if(files.length>0){const file=files[0];if(file.name.endsWith('.csv')){const reader=new FileReader();reader.onload=event=>{uploadCSV=event.target.result;beginProfiling();};reader.readAsText(file);}}
}
function handleFileUpload(event){const file=event.target.files[0];if(file){const reader=new FileReader();reader.onload=e=>{uploadCSV=e.target.result;beginProfiling();};reader.readAsText(file);}}
function pasteFromClipboard(){navigator.clipboard.readText().then(text=>{uploadCSV=text;beginProfiling();});}
function loadSampleDataset(datasetname){uploadCSV=sampleDatasets[datasetname];if(uploadCSV){beginProfiling();}else{alert('sample dataset not found.');}}

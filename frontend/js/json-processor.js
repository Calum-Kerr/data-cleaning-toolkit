function handleJsonDragOver(e){e.preventDefault();document.getElementById('jsonUploadArea').classList.add('drag-over');}
function handleJsonDragLeave(e){document.getElementById('jsonUploadArea').classList.remove('drag-over');}
function handleJsonDrop(e){e.preventDefault();document.getElementById('jsonUploadArea').classList.remove('drag-over');const files=e.dataTransfer.files;if(files.length>0)handleJsonFileUpload({target:{files:files}});}
function handleJsonFileUpload(e){const file=e.target.files[0];if(file){const reader=new FileReader();reader.onload=()=>processJsonData(reader.result);reader.readAsText(file);}}
function pasteJsonFromClipboard(){navigator.clipboard.readText().then(text=>processJsonData(text)).catch(()=>alert('Could not read clipboard'));}
function processJsonData(data){try{JSON.parse(data);alert('JSON is valid!');console.log('Processing:',data);}catch(e){alert('Invalid JSON: '+e.message);}}

function handleJsonDragOver(e){e.preventDefault();document.getElementById('jsonUploadArea').classList.add('drag-over');}
function handleJsonDragLeave(e){document.getElementById('jsonUploadArea').classList.remove('drag-over');}
function handleJsonDrop(e){e.preventDefault();document.getElementById('jsonUploadArea').classList.remove('drag-over');const files=e.dataTransfer.files;if(files.length>0)handleJsonFileUpload({target:{files:files}});}
function handleJsonFileUpload(e){const file=e.target.files[0];if(file){const reader=new FileReader();reader.onload=()=>processJsonData(reader.result);reader.readAsText(file);}}
function pasteJsonFromClipboard(){navigator.clipboard.readText().then(text=>processJsonData(text)).catch(()=>alert('Could not read clipboard'));}
function processJsonData(data){try{const parsed=JSON.parse(data);displayJsonResults(data,parsed);}catch(e){alert('Invalid JSON: '+e.message);}}
function displayJsonResults(raw,parsed){document.getElementById('semiStructuredOutput').innerHTML='<pre style="margin:0;font-family:monospace;font-size:12px;">'+JSON.stringify(parsed,null,2)+'</pre>';document.getElementById('semiStructuredResults').style.display='block';document.getElementById('jsonUploadArea').style.display='none';}
function downloadSemiStructured(){const output=document.getElementById('semiStructuredOutput').innerText;const blob=new Blob([output],{type:'application/json'});const url=URL.createObjectURL(blob);const a=document.createElement('a');a.href=url;a.download='cleaned-data.json';a.click();}

function handleTextDragOver(e){e.preventDefault();document.getElementById('textUploadArea').classList.add('drag-over');}
function handleTextDragLeave(e){document.getElementById('textUploadArea').classList.remove('drag-over');}
function handleTextDrop(e){e.preventDefault();document.getElementById('textUploadArea').classList.remove('drag-over');const files=e.dataTransfer.files;if(files.length>0)handleTextFileUpload({target:{files:files}});}
function handleTextFileUpload(e){const file=e.target.files[0];if(file){const reader=new FileReader();reader.onload=()=>processTextData(reader.result);reader.readAsText(file);}}
function pasteTextFromClipboard(){navigator.clipboard.readText().then(text=>processTextData(text)).catch(()=>alert('Could not read clipboard'));}
function processTextData(data){displayTextResults(data);}
function displayTextResults(data){const output=data.split('\n').map(l=>l.trim()).filter(l=>l).join('\n');document.getElementById('textOutput').innerHTML='<pre style="margin:0;font-family:monospace;font-size:12px;white-space:pre-wrap;">'+output+'</pre>';document.getElementById('unstructuredResults').style.display='block';document.getElementById('textUploadArea').style.display='none';}
function downloadText(){const output=document.getElementById('textOutput').innerText;const blob=new Blob([output],{type:'text/plain'});const url=URL.createObjectURL(blob);const a=document.createElement('a');a.href=url;a.download='cleaned-data.txt';a.click();}

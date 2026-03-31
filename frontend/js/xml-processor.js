function handleXmlDragOver(e){e.preventDefault();document.getElementById('xmlUploadArea').classList.add('drag-over');}
function handleXmlDragLeave(e){document.getElementById('xmlUploadArea').classList.remove('drag-over');}
function handleXmlDrop(e){e.preventDefault();document.getElementById('xmlUploadArea').classList.remove('drag-over');const files=e.dataTransfer.files;if(files.length>0)handleXmlFileUpload({target:{files:files}});}
function handleXmlFileUpload(e){const file=e.target.files[0];if(file){const reader=new FileReader();reader.onload=()=>processXmlData(reader.result);reader.readAsText(file);}}
function pasteXmlFromClipboard(){navigator.clipboard.readText().then(text=>processXmlData(text)).catch(()=>alert('Could not read clipboard'));}
function processXmlData(data){const parser=new DOMParser();const doc=parser.parseFromString(data,'application/xml');if(doc.getElementsByTagName('parsererror').length)alert('Invalid XML');else displayXmlResults(data,doc);}
function displayXmlResults(raw,doc){document.getElementById('semiStructuredOutput').innerHTML='<pre style="margin:0;font-family:monospace;font-size:12px;">'+escapeHtml(raw)+'</pre>';document.getElementById('semiStructuredResults').style.display='block';document.getElementById('xmlUploadArea').style.display='none';}
function escapeHtml(text){return text.replace(/[&<>"']/g,char=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":"&#039;"}[char]))}

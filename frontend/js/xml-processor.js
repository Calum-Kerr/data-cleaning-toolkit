function handleJsonDragOver(e){e.preventDefault();document.getElementById('jsonUploadArea').classList.add('drag-over');}
function handleJsonDragLeave(e){document.getElementById('jsonUploadArea').classList.remove('drag-over');}
function handleJsonDrop(e){e.preventDefault();document.getElementById('jsonUploadArea').classList.remove('drag-over');const files=e.dataTransfer.files;if(files.length>0)handleXmlFileUpload({target:{files:files}});}
function handleXmlFileUpload(e){const file=e.target.files[0];if(file){const reader=new FileReader();reader.onload=()=>processXmlData(reader.result);reader.readAsText(file);}}
function pasteJsonFromClipboard(){navigator.clipboard.readText().then(text=>processXmlData(text)).catch(()=>alert('Could not read clipboard'));}
function processXmlData(data){const parser=new DOMParser();const doc=parser.parseFromString(data,'application/xml');if(doc.getElementsByTagName('parsererror').length)alert('Invalid XML');else alert('XML is valid!');console.log('Processing:',data);}

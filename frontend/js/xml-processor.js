function handleXmlDragOver(e){e.preventDefault();document.getElementById('xmlUploadArea').classList.add('drag-over');}
function handleXmlDragLeave(e){document.getElementById('xmlUploadArea').classList.remove('drag-over');}
function handleXmlDrop(e){e.preventDefault();document.getElementById('xmlUploadArea').classList.remove('drag-over');const files=e.dataTransfer.files;if(files.length>0)handleXmlFileUpload({target:{files:files}});}
function handleXmlFileUpload(e){const file=e.target.files[0];if(file){const reader=new FileReader();reader.onload=()=>processXmlData(reader.result);reader.readAsText(file);}}
function pasteXmlFromClipboard(){navigator.clipboard.readText().then(text=>processXmlData(text)).catch(()=>alert('Could not read clipboard'));}
function processXmlData(data){const parser=new DOMParser();const doc=parser.parseFromString(data,'application/xml');if(doc.getElementsByTagName('parsererror').length)alert('Invalid XML');else alert('XML is valid!');console.log('Processing:',data);}

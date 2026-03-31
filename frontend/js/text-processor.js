function handleTextDragOver(e){e.preventDefault();document.getElementById('textUploadArea').classList.add('drag-over');}
function handleTextDragLeave(e){document.getElementById('textUploadArea').classList.remove('drag-over');}
function handleTextDrop(e){e.preventDefault();document.getElementById('textUploadArea').classList.remove('drag-over');const files=e.dataTransfer.files;if(files.length>0)handleTextFileUpload({target:{files:files}});}
function handleTextFileUpload(e){const file=e.target.files[0];if(file){const reader=new FileReader();reader.onload=()=>processTextData(reader.result);reader.readAsText(file);}}
function pasteTextFromClipboard(){navigator.clipboard.readText().then(text=>processTextData(text)).catch(()=>alert('Could not read clipboard'));}
function processTextData(data){alert('Text loaded ('+data.length+' chars)');console.log('Processing:',data.substring(0,100));}

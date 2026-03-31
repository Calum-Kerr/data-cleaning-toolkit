function logAuditOperation(name,rowsBefore,rowsAfter,cellsAffected){const timestamp=new Date().toISOString();auditOperations.push({operationName:name,rowsBefore,rowsAfter,cellsAffected,timestamp});}
function beginProfiling(){document.getElementById('welcomeSection').classList.add('hidden');document.getElementById('structuredSection').style.display='block';}
function getColumnCount(){return uploadCSV.trim().split('\n')[0].split(',').length;}
function downloadAsCSV(){triggerDownload(cleanedCSV,'cleaned_data.csv','text/csv');}
function downloadAsJSON(){const json=JSON.stringify(cleanedCSV,null,2);triggerDownload(json,'cleaned_data.json','application/json');}
function downloadAsTSV(){const tsv=cleanedCSV.replace(/,/g,'\t');triggerDownload(tsv,'cleaned_data.tsv','text/tab-separated-values');}
function triggerDownload(content,filename,type){const blob=new Blob([content],{type});const url=URL.createObjectURL(blob);const a=document.createElement('a');a.href=url;a.download=filename;a.click();URL.revokeObjectURL(url);}

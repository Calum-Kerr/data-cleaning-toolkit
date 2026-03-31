function selectStructured(){document.getElementById('welcomeSection').classList.add('hidden');document.getElementById('structuredSection').style.display='block';}
function showComingSoon(type){alert(`${type} data cleaning is coming soon! please select "structured data" for now.`);}
function startNewSession(){location.reload();}
function startQuickClean(){performQuickClean();}
function showCleaningComplete(){document.getElementById('cleaningSection').style.display='none';document.getElementById('progressSection').style.display='none';document.getElementById('completeSection').style.display='block';const headers=cleanedCSV.trim().split('\n')[0].split(',');updateTableDisplay(headers);displayAuditLog();}

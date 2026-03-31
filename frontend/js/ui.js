function selectStructured(){document.getElementById('welcomeSection').classList.add('hidden');document.getElementById('structuredSection').style.display='block';}
function selectSemiStructured(){document.getElementById('welcomeSection').classList.add('hidden');document.getElementById('semiStructuredSection').style.display='block';}
function selectUnstructured(){document.getElementById('welcomeSection').classList.add('hidden');document.getElementById('unstructuredSection').style.display='block';}
function backToHome(){location.reload();}
function showComingSoon(type){alert(`${type} data cleaning is coming soon! please select "structured data" for now.`);}
function startNewSession(){location.reload();}
function startQuickClean(){performQuickClean();}
function showCleaningComplete(){document.getElementById('cleaningSection').style.display='none';document.getElementById('progressSection').style.display='none';startMergeGame();}

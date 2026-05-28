function selectDataMode(sectionId){['structuredSection','semiStructuredSection','unstructuredSection'].forEach(id=>{document.getElementById(id).style.display=id===sectionId?'block':'none';});document.getElementById('welcomeSection').classList.add('hidden');}
function selectStructured(){selectDataMode('structuredSection');}
function selectSemiStructured(){selectDataMode('semiStructuredSection');}
function selectUnstructured(){selectDataMode('unstructuredSection');}
function backToHome(){location.reload();}
function showComingSoon(type){alert(`${type} data cleaning is coming soon! please select "structured data" for now.`);}
function startNewSession(){location.reload();}
function startQuickClean(){performQuickClean();}
function showCleaningComplete(){document.getElementById('cleaningSection').style.display='none';document.getElementById('progressSection').style.display='none';startMergeGame();}

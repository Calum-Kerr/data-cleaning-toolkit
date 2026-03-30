function addFindReplaceRule(){const rulesDiv=document.getElementById('findReplaceRules');const rule=document.createElement('div');rule.style.marginBottom='10px';rule.innerHTML='<input type=text placeholder="find" style="width:45%;margin-right:10px;padding:8px;border:1px solid #d9d9d9;"><input type=text placeholder="replace with" style="width:45%;padding:8px;border:1px solid #d9d9d9;">';rulesDiv.appendChild(rule);}
function applyFindReplace(){alert('apply rules');}
function approveClusters(){alert('approve clusters');}
function confirmDuplicateRemoval(){alert('remove dupes');}

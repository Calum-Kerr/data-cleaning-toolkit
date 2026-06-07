// merge game state
let mergeState={column:'',allColumns:[],currentColumnIdx:0,letterValues:{},currentLetter:'',pendingMerges:[],mergedCount:0,mergeHistory:[],nextFocusIdx:-1};

// keyboard navigation state
let keyboardFocusIdx=-1;
let keyboardSelectedIdx=-1;
let keyboardCards=[];
let keyboardGrid=[];

function getTextColumns(){const lines=cleanedCSV.trim().split('\n');const headers=lines[0].split(',').map(h=>h.trim());const textCols=[];for(let col=0;col<headers.length;col++){let isText=false;for(let row=1;row<Math.min(6,lines.length);row++){const cell=lines[row].split(',')[col];if(cell&&isNaN(cell)){isText=true;break;}}if(isText)textCols.push(headers[col]);}return textCols;}

function parseCSVLine(line){const cells=[];let current='';let inQuotes=false;for(let i=0;i<line.length;i++){const c=line[i];if(c=='"'){inQuotes=!inQuotes;}else if(c===','&&!inQuotes){cells.push(current.trim());current='';}else{current+=c;}}cells.push(current.trim());return cells;}

async function startMergeGame(){document.getElementById('mergeGameSection').style.display='block';mergeState.allColumns=getTextColumns();mergeState.currentColumnIdx=0;if(mergeState.allColumns.length===0){autoDownloadCSV();return;}loadNextColumn();}

async function loadNextColumn(){if(mergeState.currentColumnIdx>=mergeState.allColumns.length){showMergeSummary();return;}mergeState.column=mergeState.allColumns[mergeState.currentColumnIdx];mergeState.pendingMerges=[];mergeState.mergeHistory=[];mergeState.currentLetter='';loadUniqueValuesForColumn();advanceToNextLetterWithValues();}

function loadUniqueValuesForColumn(){const lines=cleanedCSV.trim().split('\n');const headers=parseCSVLine(lines[0]);const colIdx=headers.indexOf(mergeState.column);if(colIdx===-1)return;const freq={};for(let i=1;i<lines.length;i++){const cells=parseCSVLine(lines[i]);const val=cells[colIdx];if(val)freq[val]=(freq[val]||0)+1;}mergeState.letterValues={};for(const [val,count] of Object.entries(freq)){const letter=val[0].toUpperCase();if(!mergeState.letterValues[letter])mergeState.letterValues[letter]=[];mergeState.letterValues[letter].push({value:val,count:count,checked:false});}for(const letter in mergeState.letterValues){mergeState.letterValues[letter].sort((a,b)=>a.value.localeCompare(b.value));}}

function advanceToNextLetterWithValues(){const alphabet='ABCDEFGHIJKLMNOPQRSTUVWXYZ'.split('');const currentIdx=alphabet.indexOf(mergeState.currentLetter);const startIdx=currentIdx===-1?0:currentIdx+1;for(let i=startIdx;i<alphabet.length;i++){const letter=alphabet[i];if(mergeState.letterValues[letter]&&mergeState.letterValues[letter].length>1){mergeState.currentLetter=letter;renderMergeInterface();return;}}mergeState.currentColumnIdx++;loadNextColumn();}

// --- keyboard helpers --------------------------------------------------

function announceKb(msg){
  const live=document.getElementById('kbAnnouncer');
  if(live){live.textContent='';live.textContent=msg;}
}

function getGridLayout(cards){
  const rows=[];
  let currentRow=[];
  let lastTop=-1;
  for(let i=0;i<cards.length;i++){
    const top=cards[i].offsetTop;
    if(top!==lastTop&&currentRow.length>0){rows.push(currentRow);currentRow=[];}
    currentRow.push(i);
    lastTop=top;
  }
  if(currentRow.length>0)rows.push(currentRow);
  return rows;
}

function clearKeyboardFocus(){
  keyboardCards.forEach(c=>c.classList.remove('keyboard-focus'));
}

function focusKeyboardCard(board,idx){
  clearKeyboardFocus();
  if(idx<0||idx>=keyboardCards.length)return;
  keyboardFocusIdx=idx;
  const card=keyboardCards[idx];
  card.classList.add('keyboard-focus');
  card.scrollIntoView({block:'nearest',behavior:'auto'});
}

function handleKeyboardMerge(fromIdx,toIdx){
  const values=mergeState.letterValues[mergeState.currentLetter]||[];
  if(fromIdx<0||fromIdx>=values.length||toIdx<0||toIdx>=values.length)return;
  if(fromIdx===toIdx){keyboardSelectedIdx=-1;clearKeyboardFocus();focusKeyboardCard(null,toIdx);announceKb('selection cancelled');updateHint();return;}
  const fromValue=values[fromIdx].value;
  const toValue=values[toIdx].value;
  const fromCount=values[fromIdx].count;
  mergeState.pendingMerges.push({values:[fromValue],mergeInto:toValue});
  mergeState.mergeHistory.push({fromIndex:fromIdx,toIndex:toIdx,fromValue,toValue,fromCount});
  values[toIdx].count+=fromCount;
  values.splice(fromIdx,1);
  // find the new index of the target value after the splice
  mergeState.nextFocusIdx=values.findIndex(v=>v.value===toValue);
  keyboardSelectedIdx=-1;
  keyboardFocusIdx=-1;
  announceKb('merged '+fromValue+' into '+toValue);
  renderMergeInterface(mergeState.nextFocusIdx);
}

function setupKeyboardNav(board,listDiv,startIdx){
  keyboardCards=Array.from(listDiv.querySelectorAll('[data-index]'));
  keyboardGrid=getGridLayout(keyboardCards);
  keyboardFocusIdx=-1;
  keyboardSelectedIdx=-1;

  // remove old listener by cloning the node (simplest cleanup)
  const newBoard=board.cloneNode(true);
  board.parentNode.replaceChild(newBoard,board);

  newBoard.addEventListener('keydown',function(e){
    // ignore if inline editor is open
    if(document.querySelector('#gameBoard input[type=text]'))return;

    const values=mergeState.letterValues[mergeState.currentLetter]||[];

    // --- arrow key navigation ---
    if(e.key==='ArrowRight'||e.key==='ArrowLeft'||e.key==='ArrowUp'||e.key==='ArrowDown'){
      e.preventDefault();
      // refresh card references in case DOM changed
      keyboardCards=Array.from(newBoard.querySelectorAll('[data-index]'));
      if(keyboardCards.length===0)return;

      // find current position in grid
      let row=-1,col=-1;
      if(keyboardFocusIdx>=0){
        for(let r=0;r<keyboardGrid.length;r++){
          const c=keyboardGrid[r].indexOf(keyboardFocusIdx);
          if(c!==-1){row=r;col=c;break;}
        }
      }
      if(row===-1||col===-1){focusKeyboardCard(newBoard,0);return;}

      let newRow=row,newCol=col;
      if(e.key==='ArrowRight'){newCol=col+1;if(newCol>=keyboardGrid[row].length)newCol=0;}
      else if(e.key==='ArrowLeft'){newCol=col-1;if(newCol<0)newCol=keyboardGrid[row].length-1;}
      else if(e.key==='ArrowUp'){newRow=row-1;if(newRow<0)newRow=keyboardGrid.length-1;newCol=Math.min(col,keyboardGrid[newRow].length-1);}
      else if(e.key==='ArrowDown'){newRow=row+1;if(newRow>=keyboardGrid.length)newRow=0;newCol=Math.min(col,keyboardGrid[newRow].length-1);}

      const newIdx=keyboardGrid[newRow][newCol];
      focusKeyboardCard(newBoard,newIdx);
      return;
    }

    // --- space: select / deselect card ---
    if(e.key===' '||e.code==='Space'){
      e.preventDefault();
      if(keyboardFocusIdx<0||keyboardFocusIdx>=values.length)return;
      if(keyboardSelectedIdx===keyboardFocusIdx){
        // deselect
        if(keyboardCards[keyboardSelectedIdx])keyboardCards[keyboardSelectedIdx].classList.remove('keyboard-selected');
        keyboardSelectedIdx=-1;
        announceKb('selection cleared');
        updateHint();
        return;
      }
      // deselect previous if any
      if(keyboardSelectedIdx>=0&&keyboardCards[keyboardSelectedIdx]){
        keyboardCards[keyboardSelectedIdx].classList.remove('keyboard-selected');
      }
      keyboardSelectedIdx=keyboardFocusIdx;
      keyboardCards[keyboardFocusIdx].classList.add('keyboard-selected');
      announceKb('selected '+values[keyboardFocusIdx].value+' — navigate to target and press Enter to merge');
      updateHint();
      return;
    }

    // --- enter: merge (only when a card is selected and it is different) ---
    if(e.key==='Enter'){
      e.preventDefault();
      if(keyboardFocusIdx<0||keyboardFocusIdx>=values.length)return;
      if(keyboardSelectedIdx===-1||keyboardSelectedIdx===keyboardFocusIdx){
        announceKb('select a card first with Space, then press Enter on a different card to merge');
        return;
      }
      const fromIdx=keyboardSelectedIdx;
      const toIdx=keyboardFocusIdx;
      if(keyboardCards[fromIdx])keyboardCards[fromIdx].classList.remove('keyboard-selected');
      handleKeyboardMerge(fromIdx,toIdx);
      return;
    }

    // --- escape: cancel selection ---
    if(e.key==='Escape'){
      e.preventDefault();
      if(keyboardSelectedIdx>=0&&keyboardCards[keyboardSelectedIdx]){
        keyboardCards[keyboardSelectedIdx].classList.remove('keyboard-selected');
      }
      keyboardSelectedIdx=-1;
      announceKb('selection cancelled');
      updateHint();
      return;
    }

    // --- ctrl+arrow / page keys: navigate between letter groups ---
    if((e.ctrlKey&&e.key==='ArrowRight')||e.key==='PageDown'){
      e.preventDefault();
      if(mergeState.pendingMerges.length>0){applyMergesForLetter();}
      mergeState.mergeHistory=[];
      keyboardSelectedIdx=-1;keyboardFocusIdx=-1;
      advanceToNextLetterWithValues();
      return;
    }
    if((e.ctrlKey&&e.key==='ArrowLeft')||e.key==='PageUp'){
      e.preventDefault();
      // navigate to previous letter group
      var alphabet='ABCDEFGHIJKLMNOPQRSTUVWXYZ'.split('');
      var currentIdx=alphabet.indexOf(mergeState.currentLetter);
      if(currentIdx>0){
        for(var i=currentIdx-1;i>=0;i--){
          var prevLetter=alphabet[i];
          if(mergeState.letterValues[prevLetter]&&mergeState.letterValues[prevLetter].length>1){
            mergeState.currentLetter=prevLetter;
            mergeState.pendingMerges=[];mergeState.mergeHistory=[];
            keyboardSelectedIdx=-1;keyboardFocusIdx=-1;
            renderMergeInterface(0);
            return;
          }
        }
      }
      announceKb('no previous letter group');
      return;
    }
  });

  // focus the board so keyboard events work
  newBoard.setAttribute('tabindex','0');
  newBoard.focus();

  // restore focus to the startIdx card if specified
  if(startIdx>=0&&startIdx<keyboardCards.length){
    keyboardFocusIdx=startIdx;
    keyboardCards[startIdx].classList.add('keyboard-focus');
    keyboardCards[startIdx].scrollIntoView({block:'nearest',behavior:'auto'});
  }

  // update the global gameBoard reference — cloning replaced the DOM node
}

// update the hint text at the bottom of the board based on selection state
function updateHint(){
  var hint=document.getElementById('kbHint');
  if(!hint)return;
  if(keyboardSelectedIdx>=0){
    var values=mergeState.letterValues[mergeState.currentLetter]||[];
    if(keyboardSelectedIdx<values.length){
      hint.textContent='card \''+values[keyboardSelectedIdx].value+'\' selected — navigate to target and press Enter to merge';
      return;
    }
  }
  hint.textContent='Space to select • Enter to merge • Escape to cancel • Ctrl+←/→ for letter groups';
}

// --- end keyboard helpers -----------------------------------------------

function renderMergeInterface(startFocusIdx){
  if(typeof startFocusIdx==='undefined')startFocusIdx=mergeState.nextFocusIdx;
  const board=document.getElementById('gameBoard');
  board.innerHTML='';
  const container=document.createElement('div');
  container.style.cssText='padding:20px;';

  // aria-live region for screen reader announcements
  const announcer=document.createElement('div');
  announcer.id='kbAnnouncer';
  announcer.setAttribute('aria-live','polite');
  announcer.setAttribute('role','status');
  announcer.style.cssText='position:absolute;width:1px;height:1px;overflow:hidden;clip:rect(0,0,0,0);white-space:nowrap;';
  container.appendChild(announcer);

  const header=document.createElement('div');
  header.style.cssText='margin-bottom:20px;';
  const letterDisplay=document.createElement('div');
  letterDisplay.style.cssText='font-size:28px;font-weight:bold;color:#333;';
  letterDisplay.textContent=mergeState.currentLetter;
  const columnDisplay=document.createElement('div');
  columnDisplay.style.cssText='font-size:12px;color:#666;margin-top:4px;';
  columnDisplay.textContent='column: '+mergeState.column+' • drag to merge • arrow keys to navigate';
  header.appendChild(letterDisplay);header.appendChild(columnDisplay);
  container.appendChild(header);

  const values=mergeState.letterValues[mergeState.currentLetter]||[];
  const listDiv=document.createElement('div');
  listDiv.style.cssText='margin-bottom:20px;border:1px solid #ddd;border-radius:4px;padding:12px;background:#fafafa;display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:8px;max-height:600px;overflow-y:auto;';
  listDiv.setAttribute('role','list');
  listDiv.setAttribute('aria-label','merge cards for letter '+mergeState.currentLetter);

  for(let i=0;i<values.length;i++){
    const item=values[i];
    const itemDiv=document.createElement('div');
    itemDiv.style.cssText='display:flex;flex-direction:column;gap:4px;padding:8px;background:#fff;border:1px solid #e0e0e0;border-radius:2px;cursor:grab;user-select:none;';
    itemDiv.draggable=true;itemDiv.dataset.index=i;
    itemDiv.setAttribute('role','listitem');
    itemDiv.setAttribute('aria-label',item.value+' — count '+item.count);
    const countBadge=document.createElement('span');
    countBadge.style.cssText='font-size:11px;background:#e0e0e0;color:#333;padding:2px 6px;border-radius:2px;align-self:flex-start;';
    countBadge.textContent=item.count;itemDiv.appendChild(countBadge);
    const label=document.createElement('span');
    label.style.cssText='font-size:13px;word-break:break-word;cursor:pointer;padding:4px;border-radius:2px;transition:background 0.2s;';
    label.textContent=item.value;
    label.onmouseover=()=>{label.style.background='#f0f0f0';};
    label.onmouseout=()=>{label.style.background='transparent';};
    label.onclick=(e)=>{
      e.stopPropagation();
      const input=document.createElement('input');
      input.type='text';input.value=item.value.toUpperCase();
      input.style.cssText='width:100%;padding:4px;border:1px solid #0000ee;font-size:13px;';
      input.onblur=()=>{
        const newValue=input.value.trim();
        if(newValue&&newValue!==item.value){
          const oldValue=item.value;const newCount=item.count;
          mergeState.pendingMerges.push({values:[oldValue],mergeInto:newValue});
          values.splice(i,1);
          const existingIdx=values.findIndex(v=>v.value===newValue);
          if(existingIdx!==-1){values[existingIdx].count+=newCount;}
          else{values.push({value:newValue,count:newCount,checked:false});}
          values.sort((a,b)=>a.value.localeCompare(b.value));
          renderMergeInterface();
        }
      };
      input.onkeydown=(e)=>{if(e.key==='Enter'){input.blur();}else if(e.key==='Escape'){renderMergeInterface();}};
      itemDiv.style.display='none';
      const editDiv=document.createElement('div');
      editDiv.style.cssText='display:flex;flex-direction:column;gap:4px;padding:8px;background:#fff;border:1px solid #0000ee;border-radius:2px;';
      editDiv.appendChild(input);
      const confirmBtn=document.createElement('button');
      confirmBtn.textContent='change';
      confirmBtn.style.cssText='padding:4px 8px;background:#0000ee;color:#fff;border:none;cursor:pointer;font-size:12px;border-radius:2px;';
      confirmBtn.onclick=()=>{input.blur();};
      editDiv.appendChild(confirmBtn);
      itemDiv.parentNode.insertBefore(editDiv,itemDiv.nextSibling);
      input.focus();input.select();
    };
    itemDiv.appendChild(label);
    itemDiv.ondragstart=(e)=>{
      e.dataTransfer.effectAllowed='move';e.dataTransfer.setData('fromIndex',i);
      itemDiv.style.opacity='0.5';
    };
    itemDiv.ondragend=()=>{itemDiv.style.opacity='1';};
    itemDiv.ondragover=(e)=>{
      e.preventDefault();e.dataTransfer.dropEffect='move';
      itemDiv.style.background='#e3f2fd';itemDiv.style.borderColor='#0000ee';
    };
    itemDiv.ondragleave=()=>{itemDiv.style.background='#fff';itemDiv.style.borderColor='#e0e0e0';};
    itemDiv.ondrop=(e)=>{
      e.preventDefault();
      const fromIndex=parseInt(e.dataTransfer.getData('fromIndex'));
      if(fromIndex===i)return;
      const fromValue=values[fromIndex].value;const toValue=values[i].value;
      const fromCount=values[fromIndex].count;
      mergeState.pendingMerges.push({values:[fromValue],mergeInto:toValue});
      mergeState.mergeHistory.push({fromIndex,toIndex:i,fromValue,toValue,fromCount});
      values[i].count+=fromCount;values.splice(fromIndex,1);
      itemDiv.style.background='#fff';itemDiv.style.borderColor='#e0e0e0';
      keyboardSelectedIdx=-1;keyboardFocusIdx=-1;
      renderMergeInterface();
    };
    listDiv.appendChild(itemDiv);
  }
  container.appendChild(listDiv);

  const buttonsDiv=document.createElement('div');
  buttonsDiv.style.cssText='display:flex;gap:8px;';
  const backBtn=document.createElement('button');
  backBtn.style.cssText='padding:8px 16px;background:#f0f0f0;color:#333;border:1px solid #ddd;cursor:pointer;font-size:13px;border-radius:2px;';
  backBtn.textContent='← back';
  backBtn.onclick=()=>{
    const alphabet='ABCDEFGHIJKLMNOPQRSTUVWXYZ'.split('');
    const currentIdx=alphabet.indexOf(mergeState.currentLetter);
    if(currentIdx<=0)return;
    const prevLetter=alphabet[currentIdx-1];
    if(!mergeState.letterValues[prevLetter]||mergeState.letterValues[prevLetter].length<2)return;
    mergeState.currentLetter=prevLetter;
    mergeState.pendingMerges=[];mergeState.mergeHistory=[];
    keyboardSelectedIdx=-1;keyboardFocusIdx=-1;
    renderMergeInterface();
  };
  buttonsDiv.appendChild(backBtn);
  if(mergeState.mergeHistory.length>0){
    const undoBtn=document.createElement('button');
    undoBtn.style.cssText='padding:8px 16px;background:#f0f0f0;color:#333;border:1px solid #ddd;cursor:pointer;font-size:13px;border-radius:2px;';
    undoBtn.textContent='undo';
    undoBtn.onclick=()=>{
      const action=mergeState.mergeHistory.pop();
      mergeState.pendingMerges.pop();
      const vals=mergeState.letterValues[mergeState.currentLetter];
      const targetIdx=vals.findIndex(v=>v.value===action.toValue);
      if(targetIdx!==-1){vals[targetIdx].count-=action.fromCount;}
      vals.push({value:action.fromValue,count:action.fromCount,checked:false});
      vals.sort((a,b)=>a.value.localeCompare(b.value));
      keyboardSelectedIdx=-1;keyboardFocusIdx=-1;
      renderMergeInterface();
    };
    buttonsDiv.appendChild(undoBtn);
  }
  const nextLetterBtn=document.createElement('button');
  nextLetterBtn.style.cssText='padding:8px 16px;background:#333;color:#fff;border:none;cursor:pointer;font-size:13px;border-radius:2px;flex:1;';
  nextLetterBtn.textContent='next letter';
  nextLetterBtn.onclick=async()=>{
    if(mergeState.pendingMerges.length>0){await applyMergesForLetter();}
    mergeState.mergeHistory=[];
    keyboardSelectedIdx=-1;keyboardFocusIdx=-1;
    advanceToNextLetterWithValues();
  };
  buttonsDiv.appendChild(nextLetterBtn);
  container.appendChild(buttonsDiv);

  // keyboard instructions
  const kbHint=document.createElement('div');
  kbHint.id='kbHint';
  kbHint.style.cssText='margin-top:12px;padding:8px 12px;background:#f7f7f7;border:1px solid #d6d6d6;font-size:12px;color:#565656;text-align:center;';
  kbHint.textContent='Space to select • Enter to merge • Escape to cancel • Ctrl+←/→ for letter groups';
  container.appendChild(kbHint);

  board.appendChild(container);
  setupKeyboardNav(board,listDiv,startFocusIdx);
  mergeState.nextFocusIdx=-1;
}

async function applyMergesForLetter(){if(!checkOnlineAndNotify())return;const payload={csvData:cleanedCSV,column:mergeState.column,merges:mergeState.pendingMerges};try{const res=await fetch(window.location.origin+'/api/merge-clusters',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(payload)});if(!res.ok){console.error('Merge failed with status',res.status);alert(OFFLINE_MESSAGE);return;}const data=await res.json();if(data.csvData){cleanedCSV=data.csvData;mergeState.mergedCount+=mergeState.pendingMerges.length;mergeState.pendingMerges=[];loadUniqueValuesForColumn();}else{console.error('No csvData in response',data);}}catch(e){console.error('Merge error:',e);alert(OFFLINE_MESSAGE);}}

async function showMergeSummary(){document.getElementById('mergeGameSection').style.display='none';document.getElementById('completeSection').style.display='block';const summaryDiv=document.createElement('div');summaryDiv.style.cssText='padding:20px;background:#f0f0f0;border-radius:4px;margin-bottom:20px;';summaryDiv.innerHTML='<strong>merge complete:</strong> '+mergeState.mergedCount+' merge groups applied across '+mergeState.allColumns.length+' columns<br/><em style="font-size:12px;">removing duplicates...</em>';document.getElementById('gameBoard').appendChild(summaryDiv);await removeDuplicatesAfterMerge();updateTableDisplay(cleanedCSV.trim().split('\n')[0].split(','));displayAuditLog();autoDownloadCSV();}

async function removeDuplicatesAfterMerge(){const lines=cleanedCSV.trim().split('\n');const header=lines[0];const seen=new Set();const dedupedLines=[header];let removed=0;for(let i=1;i<lines.length;i++){if(seen.has(lines[i])){removed++;}else{seen.add(lines[i]);dedupedLines.push(lines[i]);}}if(removed>0){cleanedCSV=dedupedLines.join('\n');console.log('removed '+removed+' duplicate rows');}}

function autoDownloadCSV(){
  // clean up keyboard state before auto-download
  keyboardSelectedIdx=-1;keyboardFocusIdx=-1;keyboardCards=[];keyboardGrid=[];
  const csv=cleanedCSV;
  const blob=new Blob([csv],{type:'text/csv;charset=utf-8;'});
  const link=document.createElement('a');
  link.href=URL.createObjectURL(blob);
  link.download='cleaned_data_'+new Date().toISOString().split('T')[0]+'.csv';
  link.click();
  setTimeout(()=>{
    document.getElementById('mergeGameSection').style.display='none';
    document.getElementById('completeSection').style.display='block';
    updateTableDisplay(cleanedCSV.trim().split('\n')[0].split(','));displayAuditLog();
  },500);
}

// Phase 4 workflow: orchestrate clustering, merging, and find/replace
let workflowState={stage:'complete',clustersDetected:false,mergesApplied:false};
function showClusteringOptions(){const opt=document.createElement('div');opt.style.marginTop='20px';opt.innerHTML='<button onclick="startClusterWorkflow()" style="margin-right: 8px;">detect & merge clusters</button>';const complete=document.getElementById('completeSection');complete.appendChild(opt);}

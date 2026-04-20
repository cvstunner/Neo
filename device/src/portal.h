#pragma once
// ═══════════════════════════════════════════════════════════
//  portal.h — All web UI HTML stored in PROGMEM
// ═══════════════════════════════════════════════════════════

// ── Setup / WiFi config page ────────────────────────────────
const char HTML_SETUP[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>NEO Setup</title>
<style>
:root{--bg:#07070f;--card:#0f0f1e;--border:#252550;--accent:#5555ff;--accent2:#8888ff;--text:#d0d0f0;--sub:#6666aa;--ok:#33cc66;--err:#ff4444}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;min-height:100vh;display:flex;justify-content:center;align-items:flex-start;padding:20px}
.wrap{width:100%;max-width:440px}
.logo{text-align:center;padding:32px 0 24px}
.logo-text{font-size:3.5rem;font-weight:900;letter-spacing:10px;background:linear-gradient(135deg,#4444ff,#9999ff,#4444ff);-webkit-background-clip:text;-webkit-text-fill-color:transparent;background-size:200%;animation:shine 3s linear infinite}
@keyframes shine{0%{background-position:200%}100%{background-position:-200%}}
.logo-sub{color:var(--sub);font-size:.8rem;letter-spacing:4px;margin-top:4px}
.card{background:var(--card);border:1px solid var(--border);border-radius:16px;padding:24px;margin-bottom:16px}
.sec-title{font-size:.65rem;letter-spacing:3px;color:var(--sub);text-transform:uppercase;margin-bottom:16px;padding-bottom:6px;border-bottom:1px solid var(--border)}
label{display:block;font-size:.78rem;color:var(--sub);margin:14px 0 4px}
label:first-of-type{margin-top:0}
input,select{width:100%;padding:10px 14px;background:var(--bg);border:1px solid var(--border);border-radius:10px;color:var(--text);font-size:.95rem;outline:none;transition:all .2s}
input:focus,select:focus{border-color:var(--accent);box-shadow:0 0 0 3px rgba(85,85,255,.15)}
select option{background:var(--card)}
.hint{font-size:.7rem;color:var(--sub);margin-top:4px;opacity:.7}
.btn{display:block;width:100%;padding:14px;margin-top:8px;background:linear-gradient(135deg,var(--accent),var(--accent2));border:none;border-radius:12px;color:#fff;font-size:1rem;font-weight:700;letter-spacing:3px;cursor:pointer;transition:all .2s}
.btn:hover{opacity:.88;transform:translateY(-1px)}
.btn:active{transform:translateY(0)}
.row2{display:grid;grid-template-columns:1fr 1fr;gap:12px}
</style></head><body>
<div class="wrap">
  <div class="logo">
    <div class="logo-text">NEO</div>
    <div class="logo-sub">Here for You</div>
  </div>
  <form method="POST" action="/save">
    <div class="card">
      <div class="sec-title">📡 WiFi Network</div>
      <label>Network Name (SSID)</label>
      <input name="ssid" placeholder="Your WiFi name" required autocomplete="off">
      <label>Password</label>
      <input name="password" type="password" placeholder="WiFi password" autocomplete="off">
    </div>
    <div class="card">
      <div class="sec-title">👤 Your Profile</div>
      <label>Your Name</label>
      <input name="owner_name" placeholder="e.g. Arjun">
      <div class="row2">
        <div>
          <label>Age</label>
          <input name="owner_age" type="number" placeholder="24" min="1" max="120">
        </div>
        <div>
          <label>Gender</label>
          <select name="owner_gender">
            <option value="">--</option>
            <option>Male</option><option>Female</option><option>Other</option>
          </select>
        </div>
      </div>
      <label>Date of Birth</label>
      <input name="owner_dob" type="date">
      <label>Phone</label>
      <input name="owner_phone" type="tel" placeholder="+91 ...">
    </div>
    <div class="card">
      <div class="sec-title">📍 Location (for weather &amp; AQI)</div>
      <label>City / District</label>
      <input name="location" placeholder="e.g. Mumbai, Pune, Delhi" required>
      <p class="hint">Used with Open-Meteo (free, no API key). Enter closest major city.</p>
    </div>
    <div class="card">
      <div class="sec-title">🌿 Plant</div>
      <label>Plant Name</label>
      <input name="plant_name" placeholder="e.g. Basil, Monstera">
      <label>Plant Type / Notes</label>
      <input name="plant_note" placeholder="e.g. Needs watering every 2 days">
    </div>
    <button class="btn" type="submit">CONNECT &amp; SAVE ›</button>
  </form>
</div></body></html>
)HTML";

// ── Redirect / result pages ─────────────────────────────────
const char HTML_OK[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta http-equiv="refresh" content="5;url=/">
<title>NEO Connected</title>
<style>
body{background:#07070f;color:#d0d0f0;font-family:'Segoe UI',sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0}
.card{text-align:center;background:#0a1a0f;border:1px solid #1a4a2a;border-radius:20px;padding:48px 36px;max-width:360px;width:90%}
.icon{font-size:3.5rem;margin-bottom:16px}
h1{font-size:2rem;letter-spacing:4px;color:#44ff88;font-weight:900}
p{color:#66aa77;margin-top:12px;line-height:1.7;font-size:.9rem}
</style></head><body>
<div class="card">
  <div class="icon">✅</div>
  <h1>LINKED</h1>
  <p>Credentials saved!<br>NEO is now connecting.<br>This page will redirect shortly.</p>
</div></body></html>
)HTML";

const char HTML_FAIL[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>NEO - Failed</title>
<style>
body{background:#07070f;color:#d0d0f0;font-family:'Segoe UI',sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0}
.card{text-align:center;background:#1a0a0a;border:1px solid #4a1a1a;border-radius:20px;padding:48px 36px;max-width:360px;width:90%}
h1{font-size:1.8rem;letter-spacing:3px;color:#ff5555;font-weight:900}
p{color:#aa7777;margin-top:12px;line-height:1.7;font-size:.9rem}
a{color:#ff8888;text-decoration:none;font-weight:700}
</style></head><body>
<div class="card">
  <div style="font-size:3.5rem;margin-bottom:16px">❌</div>
  <h1>NOT CONNECTED</h1>
  <p>Could not connect to <b>%SSID%</b>.<br>Double-check SSID &amp; password.<br><br><a href="/">← Try again</a></p>
</div></body></html>
)HTML";

// ── Dashboard / management web app ──────────────────────────
const char HTML_DASHBOARD[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>NEO Dashboard</title>
<style>
:root{--bg:#07070f;--card:#0f0f1e;--border:#252550;--acc:#5555ff;--acc2:#8888ff;--text:#d0d0f0;--sub:#6666aa;--ok:#33cc66;--warn:#ffaa22;--err:#ff4444}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:'Segoe UI',system-ui,sans-serif;padding:16px;max-width:500px;margin:0 auto}
h1.logo{font-size:2rem;font-weight:900;letter-spacing:6px;background:linear-gradient(135deg,#4444ff,#9999ff);-webkit-background-clip:text;-webkit-text-fill-color:transparent;margin-bottom:4px}
.tagline{color:var(--sub);font-size:.75rem;letter-spacing:3px;margin-bottom:20px}
.tabs{display:flex;gap:8px;margin-bottom:16px;flex-wrap:wrap}
.tab{padding:7px 14px;border-radius:20px;border:1px solid var(--border);background:var(--card);color:var(--sub);font-size:.8rem;cursor:pointer;transition:all .2s}
.tab.active{background:var(--acc);color:#fff;border-color:var(--acc)}
.pane{display:none}.pane.active{display:block}
.card{background:var(--card);border:1px solid var(--border);border-radius:14px;padding:18px;margin-bottom:14px}
.card h3{font-size:.7rem;letter-spacing:2px;color:var(--sub);text-transform:uppercase;margin-bottom:12px}
.stat-row{display:flex;justify-content:space-between;align-items:center;padding:6px 0;border-bottom:1px solid var(--border)}
.stat-row:last-child{border:none}
.stat-val{font-size:1.1rem;font-weight:700;color:var(--acc2)}
label{display:block;font-size:.75rem;color:var(--sub);margin:10px 0 4px}
input,select,textarea{width:100%;padding:9px 12px;background:var(--bg);border:1px solid var(--border);border-radius:9px;color:var(--text);font-size:.9rem;outline:none}
input:focus,select:focus,textarea:focus{border-color:var(--acc)}
.btn{padding:10px 20px;background:var(--acc);border:none;border-radius:9px;color:#fff;font-size:.85rem;font-weight:700;cursor:pointer;transition:all .2s;letter-spacing:1px}
.btn:hover{opacity:.85}.btn.sm{padding:6px 12px;font-size:.75rem}
.btn.red{background:var(--err)}.btn.green{background:var(--ok)}
.row2{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.task-item{display:flex;align-items:center;gap:10px;padding:8px 0;border-bottom:1px solid var(--border)}
.task-item:last-child{border:none}
.task-check{width:18px;height:18px;border-radius:4px;border:2px solid var(--border);background:transparent;cursor:pointer;flex-shrink:0;display:flex;align-items:center;justify-content:center}
.task-check.done{background:var(--ok);border-color:var(--ok)}
.task-text{flex:1;font-size:.9rem}.task-text.done{text-decoration:line-through;opacity:.5}
.task-owner{font-size:.7rem;color:var(--sub);background:var(--bg);padding:2px 7px;border-radius:10px}
.profile-card{display:flex;align-items:center;gap:12px;padding:10px 0;border-bottom:1px solid var(--border)}
.profile-card:last-child{border:none}
.avatar{width:36px;height:36px;border-radius:50%;background:linear-gradient(135deg,var(--acc),var(--acc2));display:flex;align-items:center;justify-content:center;font-weight:900;font-size:1rem;flex-shrink:0}
.profile-info{flex:1}.profile-name{font-weight:700}.profile-meta{font-size:.75rem;color:var(--sub)}
.uid-badge{font-size:.65rem;background:var(--bg);border:1px solid var(--border);padding:2px 8px;border-radius:8px;font-family:monospace}
.badge{padding:2px 8px;border-radius:10px;font-size:.7rem;font-weight:700}
.badge.ok{background:#1a4a2a;color:var(--ok)}.badge.warn{background:#3a2a0a;color:var(--warn)}.badge.err{background:#3a0a0a;color:var(--err)}
.log-item{font-family:monospace;font-size:.75rem;padding:4px 0;border-bottom:1px solid var(--border);color:var(--sub)}
.log-item:last-child{border:none}
#toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%);background:var(--ok);color:#fff;padding:10px 20px;border-radius:12px;font-size:.85rem;font-weight:700;display:none;z-index:999}
</style></head><body>
<h1 class="logo">NEO</h1>
<div class="tagline">HERE FOR YOU</div>
<div class="tabs">
  <div class="tab active" onclick="tab('dash')">Dashboard</div>
  <div class="tab" onclick="tab('tasks')">Tasks</div>
  <div class="tab" onclick="tab('people')">People</div>
  <div class="tab" onclick="tab('plant')">Plant</div>
  <div class="tab" onclick="tab('log')">Log</div>
  <div class="tab" onclick="tab('settings')">Settings</div>
</div>

<!-- DASHBOARD -->
<div class="pane active" id="pane-dash">
  <div class="card"><h3>🌤 Weather</h3>
    <div class="stat-row"><span>Temperature</span><span class="stat-val" id="w-temp">--</span></div>
    <div class="stat-row"><span>Feels like</span><span id="w-feels">--</span></div>
    <div class="stat-row"><span>Condition</span><span id="w-cond">--</span></div>
    <div class="stat-row"><span>Humidity</span><span id="w-hum">--</span></div>
    <div class="stat-row"><span>Wind</span><span id="w-wind">--</span></div>
    <div class="stat-row"><span>AQI</span><span id="w-aqi">--</span></div>
  </div>
  <div class="card"><h3>📡 System</h3>
    <div class="stat-row"><span>WiFi</span><span id="s-wifi">--</span></div>
    <div class="stat-row"><span>IP</span><span id="s-ip">--</span></div>
    <div class="stat-row"><span>Uptime</span><span id="s-up">--</span></div>
    <div class="stat-row"><span>Free Heap</span><span id="s-heap">--</span></div>
    <div class="stat-row"><span>Time</span><span id="s-time">--</span></div>
  </div>
  <div class="card"><h3>🌿 Plant</h3>
    <div class="stat-row"><span>Moisture</span><span class="stat-val" id="p-moist">--</span></div>
    <div class="stat-row"><span>Status</span><span id="p-status">--</span></div>
  </div>
  <div class="card"><h3>🔖 Last RFID Scan</h3>
    <div class="stat-row"><span>UID</span><span id="r-uid" style="font-family:monospace">--</span></div>
    <div class="stat-row"><span>Who</span><span id="r-who">--</span></div>
    <div class="stat-row"><span>Time</span><span id="r-time">--</span></div>
  </div>
</div>

<!-- TASKS -->
<div class="pane" id="pane-tasks">
  <div class="card"><h3>➕ Add Task / Reminder</h3>
    <label>Task</label>
    <input id="t-text" placeholder="e.g. Water the plant">
    <div class="row2">
      <div><label>For</label>
        <select id="t-owner"><option value="all">Everyone</option></select>
      </div>
      <div><label>Due time</label>
        <input id="t-time" type="time">
      </div>
    </div>
    <label>Due date (optional)</label>
    <input id="t-date" type="date">
    <br><br>
    <button class="btn" onclick="addTask()">ADD TASK</button>
  </div>
  <div class="card"><h3>📋 Tasks</h3>
    <div id="task-list"><p style="color:var(--sub);font-size:.85rem">Loading...</p></div>
  </div>
</div>

<!-- PEOPLE -->
<div class="pane" id="pane-people">
  <div class="card"><h3>👤 Add Profile</h3>
    <label>Name</label><input id="pr-name" placeholder="e.g. Priya">
    <div class="row2">
      <div><label>Role</label>
        <select id="pr-role">
          <option value="family">Family</option>
          <option value="partner">Partner / GF</option>
          <option value="owner">Owner (me)</option>
          <option value="plant">Plant</option>
        </select>
      </div>
      <div><label>Age</label><input id="pr-age" type="number" placeholder="--"></div>
    </div>
    <label>Gender</label>
    <select id="pr-gender"><option value="">--</option><option>Male</option><option>Female</option><option>Other</option></select>
    <label>Note</label><input id="pr-note" placeholder="Optional note">
    <label>RFID UID (scan card to assign)</label>
    <input id="pr-uid" placeholder="e.g. A3:4C:D8:FA" style="font-family:monospace">
    <br><br><button class="btn" onclick="addProfile()">SAVE PROFILE</button>
  </div>
  <div class="card"><h3>👥 Profiles</h3>
    <div id="profile-list"><p style="color:var(--sub);font-size:.85rem">Loading...</p></div>
  </div>
</div>

<!-- PLANT -->
<div class="pane" id="pane-plant">
  <div class="card"><h3>🌿 Plant Status</h3>
    <div class="stat-row"><span>Moisture</span><span class="stat-val" id="pl-moist">--</span></div>
    <div class="stat-row"><span>Health</span><span id="pl-status">--</span></div>
    <div style="margin-top:12px;background:var(--bg);border-radius:8px;height:12px;overflow:hidden">
      <div id="pl-bar" style="height:100%;background:linear-gradient(90deg,#1a6a1a,#44cc44);transition:width .5s;width:0%"></div>
    </div>
  </div>
  <div class="card"><h3>🗓 Plant Tasks</h3>
    <div id="plant-tasks"><p style="color:var(--sub);font-size:.85rem">Loading...</p></div>
  </div>
</div>

<!-- LOG -->
<div class="pane" id="pane-log">
  <div class="card"><h3>📜 RFID Scan Log</h3>
    <div id="log-list"><p style="color:var(--sub);font-size:.85rem">Loading...</p></div>
    <br><button class="btn red sm" onclick="clearLog()">Clear Log</button>
  </div>
</div>

<!-- SETTINGS -->
<div class="pane" id="pane-settings">
  <div class="card"><h3>⚙️ System Settings</h3>
    <label>Owner Name</label><input id="cfg-name">
    <label>Location (city)</label><input id="cfg-loc">
    <label>WiFi SSID</label><input id="cfg-ssid">
    <label>WiFi Password</label><input id="cfg-pass" type="password">
    <br><br>
    <button class="btn" onclick="saveSettings()">SAVE SETTINGS</button>
    <br><br>
    <button class="btn red sm" onclick="if(confirm('Reset all data?'))fetch('/reset').then(()=>location.reload())">Factory Reset</button>
  </div>
</div>

<div id="toast"></div>

<script>
function tab(id){
  document.querySelectorAll('.tab').forEach((t,i)=>{t.classList.toggle('active',['dash','tasks','people','plant','log','settings'][i]===id)});
  document.querySelectorAll('.pane').forEach(p=>p.classList.toggle('active',p.id==='pane-'+id));
  if(id==='dash')loadDash();
  if(id==='tasks')loadTasks();
  if(id==='people')loadProfiles();
  if(id==='plant')loadPlant();
  if(id==='log')loadLog();
  if(id==='settings')loadSettings();
}
function toast(msg,ok=true){
  const t=document.getElementById('toast');
  t.textContent=msg;t.style.background=ok?'#33cc66':'#ff4444';
  t.style.display='block';setTimeout(()=>t.style.display='none',2500);
}
function loadDash(){
  fetch('/api/status').then(r=>r.json()).then(d=>{
    document.getElementById('w-temp').textContent=d.weather?.temp!=null?d.weather.temp+'°C':'--';
    document.getElementById('w-feels').textContent=d.weather?.feels!=null?d.weather.feels+'°C':'--';
    document.getElementById('w-cond').textContent=d.weather?.desc||'--';
    document.getElementById('w-hum').textContent=d.weather?.hum!=null?d.weather.hum+'%':'--';
    document.getElementById('w-wind').textContent=d.weather?.wind!=null?d.weather.wind+' km/h':'--';
    const aqi=d.weather?.aqi;
    document.getElementById('w-aqi').textContent=aqi>0?aqi+' '+(aqi<=50?'(Good)':aqi<=100?'(Moderate)':aqi<=150?'(Unhealthy)':'(Poor)'):'--';
    document.getElementById('s-wifi').textContent=d.rssi?d.rssi+' dBm':'--';
    document.getElementById('s-ip').textContent=d.ip||'--';
    document.getElementById('s-up').textContent=d.uptime!=null?Math.floor(d.uptime/60)+' min':'--';
    document.getElementById('s-heap').textContent=d.heap!=null?Math.floor(d.heap/1024)+' KB':'--';
    document.getElementById('s-time').textContent=d.time||'--';
    const m=d.moisture;
    document.getElementById('p-moist').textContent=m!=null?m+'%':'N/A';
    document.getElementById('p-status').textContent=m==null?'No sensor':m<20?'🚨 Needs water!':m<40?'⚠ Getting dry':m<70?'✅ Healthy':'💧 Well watered';
    document.getElementById('r-uid').textContent=d.lastUID||'--';
    document.getElementById('r-who').textContent=d.lastWho||'Unknown';
    document.getElementById('r-time').textContent=d.lastScanTime||'--';
  }).catch(()=>{});
}
function loadTasks(){
  fetch('/api/tasks').then(r=>r.json()).then(data=>{
    const sel=document.getElementById('t-owner');
    sel.innerHTML='<option value="all">Everyone</option>';
    (data.profiles||[]).forEach(p=>sel.innerHTML+=`<option value="${p.name}">${p.name}</option>`);
    const el=document.getElementById('task-list');
    if(!data.tasks||!data.tasks.length){el.innerHTML='<p style="color:var(--sub);font-size:.85rem">No tasks yet.</p>';return;}
    el.innerHTML=data.tasks.map((t,i)=>`
      <div class="task-item">
        <div class="task-check ${t.done?'done':''}" onclick="toggleTask(${i})">
          ${t.done?'✓':''}
        </div>
        <span class="task-text ${t.done?'done':''}">${t.text}${t.dueTime?' <small style="color:var(--sub)">@ '+t.dueTime+'</small>':''}</span>
        <span class="task-owner">${t.owner}</span>
        <button class="btn red sm" onclick="deleteTask(${i})" style="padding:3px 8px;font-size:.7rem">✕</button>
      </div>`).join('');
  });
}
function addTask(){
  const t={text:document.getElementById('t-text').value,owner:document.getElementById('t-owner').value,dueTime:document.getElementById('t-time').value,dueDate:document.getElementById('t-date').value,done:false};
  if(!t.text){toast('Enter task text',false);return;}
  fetch('/api/tasks/add',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(t)}).then(()=>{toast('Task added!');document.getElementById('t-text').value='';loadTasks();});
}
function toggleTask(i){fetch('/api/tasks/toggle',{method:'POST',body:String(i)}).then(()=>loadTasks());}
function deleteTask(i){fetch('/api/tasks/delete',{method:'POST',body:String(i)}).then(()=>{toast('Deleted');loadTasks();});}
function loadProfiles(){
  fetch('/api/profiles').then(r=>r.json()).then(data=>{
    const el=document.getElementById('profile-list');
    if(!data.length){el.innerHTML='<p style="color:var(--sub);font-size:.85rem">No profiles yet. Add one above.</p>';return;}
    el.innerHTML=data.map((p,i)=>`
      <div class="profile-card">
        <div class="avatar">${p.name?p.name[0].toUpperCase():'?'}</div>
        <div class="profile-info">
          <div class="profile-name">${p.name} <span class="badge ok">${p.role}</span></div>
          <div class="profile-meta">${p.age?'Age '+p.age:''}${p.gender?' · '+p.gender:''} ${p.note?'· '+p.note:''}</div>
          <div class="uid-badge">${p.uid||'No card assigned'}</div>
        </div>
        <button class="btn red sm" onclick="deleteProfile(${i})">✕</button>
      </div>`).join('');
  });
}
function addProfile(){
  const p={name:document.getElementById('pr-name').value,role:document.getElementById('pr-role').value,age:document.getElementById('pr-age').value,gender:document.getElementById('pr-gender').value,note:document.getElementById('pr-note').value,uid:document.getElementById('pr-uid').value.toUpperCase().trim()};
  if(!p.name){toast('Enter a name',false);return;}
  fetch('/api/profiles/add',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(p)}).then(()=>{toast('Profile saved!');loadProfiles();});
}
function deleteProfile(i){fetch('/api/profiles/delete',{method:'POST',body:String(i)}).then(()=>{toast('Removed');loadProfiles();});}
function loadPlant(){
  fetch('/api/status').then(r=>r.json()).then(d=>{
    const m=d.moisture;
    document.getElementById('pl-moist').textContent=m!=null?m+'%':'N/A';
    document.getElementById('pl-status').textContent=m==null?'No sensor':m<20?'🚨 Needs water NOW!':m<40?'⚠ Getting dry':m<70?'✅ Healthy':'💧 Well watered';
    document.getElementById('pl-bar').style.width=(m||0)+'%';
  });
  fetch('/api/tasks').then(r=>r.json()).then(d=>{
    const el=document.getElementById('plant-tasks');
    const pt=(d.tasks||[]).filter(t=>t.owner==='Plant'||t.owner==='plant'||t.owner==='all');
    el.innerHTML=pt.length?pt.map(t=>`<div class="task-item"><span class="task-text ${t.done?'done':''}">${t.text}</span><span class="badge ${t.done?'ok':'warn'}">${t.done?'Done':'Pending'}</span></div>`).join(''):'<p style="color:var(--sub);font-size:.85rem">No plant tasks.</p>';
  });
}
function loadLog(){
  fetch('/api/log').then(r=>r.text()).then(txt=>{
    const el=document.getElementById('log-list');
    const lines=txt.trim().split('\n').filter(Boolean).reverse().slice(0,50);
    el.innerHTML=lines.length?lines.map(l=>`<div class="log-item">${l}</div>`).join(''):'<p style="color:var(--sub)">No scans yet.</p>';
  });
}
function clearLog(){if(confirm('Clear scan log?'))fetch('/api/log/clear',{method:'POST'}).then(()=>{toast('Log cleared');loadLog();});}
function loadSettings(){
  fetch('/api/config').then(r=>r.json()).then(d=>{
    document.getElementById('cfg-name').value=d.ownerName||'';
    document.getElementById('cfg-loc').value=d.location||'';
    document.getElementById('cfg-ssid').value=d.wifiSSID||'';
  });
}
function saveSettings(){
  const d={ownerName:document.getElementById('cfg-name').value,location:document.getElementById('cfg-loc').value,wifiSSID:document.getElementById('cfg-ssid').value,wifiPass:document.getElementById('cfg-pass').value};
  fetch('/api/config/save',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(d)}).then(()=>toast('Settings saved!'));
}
// Auto-refresh dashboard every 10s
setInterval(()=>{if(document.getElementById('pane-dash').classList.contains('active'))loadDash();},10000);
loadDash();
</script>
</body></html>
)HTML";
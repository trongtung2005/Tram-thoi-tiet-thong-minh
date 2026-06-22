// ============================================================
//  web_ui.h  v2.2
//  Refactored: 5 Realtime Sensors (Temp, Hum, Wind, Dir, Rain)
// ============================================================
#pragma once
#include <pgmspace.h>

const char WEB_UI_HTML[] PROGMEM = R"HTMLEOF(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">
<title>AI Weather Station</title>
<style>
  :root {
    --bg:        #0d1117;
    --surface:   #161b22;
    --surface2:  #21262d;
    --border:    #30363d;
    --text:      #e6edf3;
    --text2:     #8b949e;
    --blue:      #388bfd;
    --cyan:      #39d353;
    --orange:    #f0883e;
    --red:       #f85149;
    --radius:    12px;
    --gap:       14px;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    background: var(--bg);
    color: var(--text);
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', system-ui, sans-serif;
    font-size: 14px;
    min-height: 100vh;
    padding: 12px;
  }

  /* HEADER */
  .header {
    display: flex; align-items: center; justify-content: space-between;
    padding: 14px 18px; background: var(--surface);
    border: 1px solid var(--border); border-radius: var(--radius);
    margin-bottom: var(--gap);
  }
  .header-left { display: flex; align-items: center; gap: 10px; }
  .logo { font-size: 22px; }
  .title { font-size: 17px; font-weight: 700; letter-spacing: -0.3px; }
  .subtitle { font-size: 11px; color: var(--text2); margin-top: 1px; }
  .status-dot {
    width: 8px; height: 8px; border-radius: 50%;
    background: var(--cyan); box-shadow: 0 0 6px var(--cyan);
    animation: pulse 2s infinite;
  }
  @keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.4} }
  .header-right { text-align: right; }
  .refresh-badge { font-size: 11px; color: var(--text2); display: flex; align-items: center; gap: 5px; justify-content: flex-end; }
  .countdown {
    display: inline-block; background: var(--surface2);
    border: 1px solid var(--border); border-radius: 20px;
    padding: 2px 8px; font-size: 11px; color: var(--blue);
    min-width: 28px; text-align: center;
  }
  .last-update { font-size: 10px; color: var(--text2); margin-top: 3px; }

  /* GRIDS */
  .grid-realtime { display: grid; grid-template-columns: repeat(auto-fit, minmax(110px, 1fr)); gap: var(--gap); margin-bottom: var(--gap); }
  .grid-7 { display: grid; grid-template-columns: repeat(7,1fr); gap: 8px; margin-bottom: var(--gap); }
  @media (max-width: 700px) {
    .grid-7 { grid-template-columns: repeat(4,1fr); }
  }
  @media (max-width: 420px) {
    .grid-7 { grid-template-columns: repeat(2,1fr); }
  }

  /* CARD & METRIC */
  .card {
    background: var(--surface); border: 1px solid var(--border);
    border-radius: var(--radius); padding: 14px 16px;
  }
  .card-title {
    font-size: 12px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.8px;
    color: var(--text2); margin-bottom: 12px;
  }
  .metric { display: flex; flex-direction: column; align-items: flex-start; gap: 2px; }
  .metric-icon { font-size: 20px; margin-bottom: 4px; }
  .metric-label { font-size: 10px; color: var(--text2); text-transform: uppercase; letter-spacing: 0.5px; }
  .metric-value { font-size: 22px; font-weight: 700; line-height: 1.1; }
  .metric-unit  { font-size: 11px; color: var(--text2); }

  /* 7-DAY STRIP */
  .day-card {
    background: var(--surface2); border: 1px solid var(--border);
    border-radius: 10px; padding: 12px 6px; text-align: center;
    cursor: pointer; transition: all 0.2s;
  }
  .day-card:hover { border-color: var(--blue); }
  .day-card.selected { border-color: var(--orange); background: rgba(240,136,62,0.1); }
  .day-name { font-size: 13px; color: var(--text); font-weight: 600; margin-bottom: 6px; }
  .day-temp { font-size: 12px; font-weight: 700; color: var(--text); margin-bottom: 6px; }
  .day-rain { font-size: 11px; color: var(--text2); }

  /* CANVAS CHART */
  .chart-wrap { position: relative; height: 200px; width: 100%; }
  .chart-wrap canvas { display: block; width: 100%; height: 100%; }

  /* SECTION LABEL */
  .section-label {
    font-size: 11px; color: var(--text2); font-weight: 600;
    text-transform: uppercase; letter-spacing: 1px; margin-bottom: var(--gap);
    display: flex; align-items: center; gap: 8px;
  }
  .section-label::after { content:''; flex:1; height:1px; background:var(--border); }

  /* OFFLINE BANNER */
  #offline-banner {
    display: none; background: rgba(248,81,73,0.15);
    border: 1px solid rgba(248,81,73,0.4); border-radius: 8px;
    padding: 8px 14px; font-size: 12px; color: var(--red);
    margin-bottom: var(--gap); text-align: center;
  }
</style>
</head>
<body>

<div class="header">
  <div class="header-left">
    <div class="logo">🌤️</div>
    <div>
      <div class="title">AI Weather Station</div>
      <div class="subtitle">ESP32 · Ridge ML Model</div>
    </div>
  </div>
  <div class="header-right">
    <div class="refresh-badge">
      <div class="status-dot" id="dot"></div>
      Next: <span class="countdown" id="cd">5</span>s
    </div>
    <div class="last-update" id="last-update">–</div>
  </div>
</div>

<div id="offline-banner">⚠️ Cannot reach device — showing last known data</div>

<div class="section-label">📡 Realtime Sensors</div>
<div class="grid-realtime">
  <div class="card">
    <div class="metric">
      <div class="metric-icon">🌡️</div>
      <div class="metric-label">Temperature</div>
      <div><span class="metric-value" id="s-temp">–</span><span class="metric-unit"> °C</span></div>
    </div>
  </div>
  <div class="card">
    <div class="metric">
      <div class="metric-icon">💧</div>
      <div class="metric-label">Humidity</div>
      <div><span class="metric-value" id="s-hum">–</span><span class="metric-unit"> %</span></div>
    </div>
  </div>
  <div class="card">
    <div class="metric">
      <div class="metric-icon">💨</div>
      <div class="metric-label">Wind Speed</div>
      <div><span class="metric-value" id="s-wind">–</span><span class="metric-unit"> m/s</span></div>
    </div>
  </div>
  <div class="card">
    <div class="metric">
      <div class="metric-icon">🧭</div>
      <div class="metric-label">Wind Dir</div>
      <div><span class="metric-value" id="s-dir" style="font-size:18px;">–</span></div>
    </div>
  </div>
  <div class="card">
    <div class="metric">
      <div class="metric-icon" id="rain-icon">☀️</div>
      <div class="metric-label">Status</div>
      <div><span class="metric-value" id="s-rain" style="font-size:18px;">–</span></div>
    </div>
  </div>
</div>

<div class="section-label">🔮 7-Day Outlook</div>
<div class="grid-7" id="forecast-strip">
</div>

<div id="chart-section" style="display:none; margin-bottom:var(--gap);">
  <div class="card">
    <div class="card-title" id="chart-title">Daily Temperature Forecast</div>
    <div class="chart-wrap" id="canvas-container">
      <canvas id="tempChart"></canvas>
    </div>
  </div>
</div>

<script>
let globalHourlyData = null;
let currentDayIndex = -1;

function drawChart(data) {
  let canvas = document.getElementById('tempChart');
  let container = document.getElementById('canvas-container');
  let ctx = canvas.getContext('2d');
  
  let cw = container.clientWidth;
  let ch = container.clientHeight;
  
  let dpr = window.devicePixelRatio || 1;
  canvas.width = cw * dpr;
  canvas.height = ch * dpr;
  ctx.scale(dpr, dpr);
  
  ctx.clearRect(0, 0, cw, ch);
  
  let minT = Math.min(...data) - 1;
  let maxT = Math.max(...data) + 1;
  
  let padX = 35, padY = 20;
  let w = cw - padX * 2;
  let h = ch - padY * 2;
  
  ctx.fillStyle = '#8b949e';
  ctx.font = '10px sans-serif';
  ctx.textBaseline = 'middle';
  
  ctx.textAlign = 'center';
  for(let i = 0; i <= 24; i += 3) {
    if(i === 24) continue;
    let x = padX + (i/23) * w;
    ctx.fillText(i + 'h', x, ch - padY/2 + 5);
  }
  
  ctx.textAlign = 'right';
  for(let i = 0; i <= 4; i++) {
    let t = minT + (maxT - minT) * (i/4);
    let y = ch - padY - (i/4) * h;
    ctx.fillText(t.toFixed(1), padX - 8, y);
    ctx.beginPath();
    ctx.strokeStyle = '#30363d';
    ctx.moveTo(padX, y);
    ctx.lineTo(cw - padX + 10, y);
    ctx.stroke();
  }
  
  ctx.beginPath();
  ctx.strokeStyle = '#f0883e';
  ctx.lineWidth = 2.5;
  ctx.lineJoin = 'round';
  for(let i = 0; i < 24; i++) {
    let x = padX + (i/23) * w;
    let y = ch - padY - ((data[i] - minT) / (maxT - minT)) * h;
    if(i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }
  ctx.stroke();

  ctx.fillStyle = '#f0883e';
  for(let i = 0; i < 24; i++) {
    let x = padX + (i/23) * w;
    let y = ch - padY - ((data[i] - minT) / (maxT - minT)) * h;
    ctx.beginPath();
    ctx.arc(x, y, 3.5, 0, Math.PI*2);
    ctx.fill();
    ctx.beginPath();
    ctx.fillStyle = '#161b22';
    ctx.arc(x, y, 1.5, 0, Math.PI*2);
    ctx.fill();
    ctx.fillStyle = '#f0883e';
  }
}

window.addEventListener('resize', () => {
  if (currentDayIndex >= 0 && globalHourlyData) {
    drawChart(globalHourlyData[currentDayIndex]);
  }
});

function updateUI(d) {
  // Update Realtime Sensors
  document.getElementById('s-temp').textContent = d.realtime.temperature.toFixed(1);
  document.getElementById('s-hum').textContent  = d.realtime.humidity.toFixed(1);
  document.getElementById('s-wind').textContent = d.realtime.windSpeed.toFixed(1);
  document.getElementById('s-dir').textContent  = d.realtime.windDir;
  
  // Update Rain Status visually
  if(d.realtime.rain === 1) {
      document.getElementById('s-rain').textContent = "Đang mưa!";
      document.getElementById('s-rain').style.color = "var(--cyan)";
      document.getElementById('rain-icon').textContent = "☔";
  } else {
      document.getElementById('s-rain').textContent = "Không mưa";
      document.getElementById('s-rain').style.color = "var(--text)";
      document.getElementById('rain-icon').textContent = "☀️";
  }
  
  globalHourlyData = d.hourlyForecast;

  let strip = document.getElementById('forecast-strip');
  strip.innerHTML = '';
  let days = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];
  
  d.forecast7d.forEach((fc, idx) => {
    let dt = new Date(fc.date);
    let dayName = isNaN(dt) ? fc.date : days[dt.getDay()];
    let rainIcon = fc.rain ? '🌧' : '☀';
    let rainText = fc.rain ? 'Rain' : 'No Rain';
    
    let el = document.createElement('div');
    el.className = 'day-card' + (currentDayIndex === idx ? ' selected' : '');
    el.innerHTML = `
      <div class="day-name">${dayName}</div>
      <div class="day-temp">${fc.minTemp.toFixed(1)}° - ${fc.maxTemp.toFixed(1)}°</div>
      <div class="day-rain">${rainIcon} ${rainText}</div>
    `;
    
    el.onclick = () => {
      currentDayIndex = idx;
      document.querySelectorAll('.day-card').forEach(c => c.classList.remove('selected'));
      el.classList.add('selected');
      document.getElementById('chart-section').style.display = 'block';
      document.getElementById('chart-title').textContent = `${dayName} Temperature Forecast`;
      drawChart(globalHourlyData[idx]);
    };
    strip.appendChild(el);
  });
  
  if (currentDayIndex >= 0 && currentDayIndex < 7) {
    drawChart(globalHourlyData[currentDayIndex]);
  }
  
  let t = new Date();
  document.getElementById('last-update').textContent = 'Updated ' +
    ('0'+t.getHours()).slice(-2) + ':' +
    ('0'+t.getMinutes()).slice(-2) + ':' +
    ('0'+t.getSeconds()).slice(-2);
}

let counter = 5;
function fetchDat() {
  fetch('/data')
    .then(r => {
      if(!r.ok) throw new Error();
      return r.json();
    })
    .then(d => {
      document.getElementById('offline-banner').style.display = 'none';
      document.getElementById('dot').style.background = 'var(--cyan)';
      document.getElementById('dot').style.boxShadow = '0 0 6px var(--cyan)';
      if(!d.error) updateUI(d);
    })
    .catch(() => {
      document.getElementById('offline-banner').style.display = 'block';
      document.getElementById('dot').style.background = 'var(--red)';
      document.getElementById('dot').style.boxShadow = '0 0 6px var(--red)';
    });
}

setInterval(() => {
  counter--;
  if(counter <= 0) { counter = 5; fetchDat(); }
  document.getElementById('cd').textContent = counter;
}, 1000);

fetchDat();
</script>
</body>
</html>
)HTMLEOF";
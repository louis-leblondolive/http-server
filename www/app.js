/* app.js — horloge en temps réel */

function updateClock() {
  const el = document.getElementById('clock');
  if (!el) return;
  el.textContent = new Date().toLocaleTimeString('fr-FR');
}

updateClock();
setInterval(updateClock, 1000);

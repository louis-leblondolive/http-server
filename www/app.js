/* app.js — horloge en temps réel et gestion CGI */

function updateClock() {
  const el = document.getElementById('clock');
  if (!el) return;
  el.textContent = new Date().toLocaleTimeString('fr-FR');
}

// Initialisation de l'horloge
updateClock();
setInterval(updateClock, 1000);
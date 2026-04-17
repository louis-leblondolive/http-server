/* app.js — horloge en temps réel et exécution CGI dynamique */

// ── Horloge ───────────────────────────────────────────
function updateClock() {
  const el = document.getElementById('clock');
  if (!el) return;
  el.textContent = new Date().toLocaleTimeString('fr-FR');
}
updateClock();
setInterval(updateClock, 1000);

// ── Onglets méthode ───────────────────────────────────
let currentMethod = 'GET';

document.querySelectorAll('.tab').forEach(tab => {
  tab.addEventListener('click', () => {
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    tab.classList.add('active');
    currentMethod = tab.dataset.method;
  });
});

// ── Exécution CGI ─────────────────────────────────────
async function runCgi() {
  const command = document.getElementById('command-select').value;
  const output = document.getElementById('cgi-output');
  const btn = document.getElementById('run-btn');

  btn.classList.add('loading');
  btn.textContent = '…';
  output.className = 'cgi-output';
  output.innerHTML = '<span class="cgi-placeholder">Exécution…</span>';

  const url = '/cgi-bin/exec.py';
  const start = performance.now();

  try {
    let response;

    if (currentMethod === 'GET') {
      response = await fetch(`${url}?command=${encodeURIComponent(command)}`, {
        method: 'GET',
      });
    } else {
      response = await fetch(url, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: `command=${encodeURIComponent(command)}`,
      });
    }

    const elapsed = Math.round(performance.now() - start);
    const text = await response.text();

    if (response.ok) {
      output.classList.add('has-result');
      output.innerHTML = `
        <div class="cgi-inner">
          <span class="cgi-meta">${currentMethod} /cgi-bin/exec.py · ${response.status} · ${elapsed}ms</span>
          <span>${escapeHtml(text)}</span>
        </div>`;
    } else {
      output.classList.add('error');
      output.innerHTML = `
        <div class="cgi-inner">
          <span class="cgi-meta">${currentMethod} · ${response.status}</span>
          <span>${escapeHtml(text)}</span>
        </div>`;
    }
  } catch (err) {
    output.classList.add('error');
    output.innerHTML = `<span>Erreur réseau : ${escapeHtml(err.message)}</span>`;
  }

  btn.classList.remove('loading');
  btn.textContent = 'Exécuter';
}

function escapeHtml(str) {
  return str
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

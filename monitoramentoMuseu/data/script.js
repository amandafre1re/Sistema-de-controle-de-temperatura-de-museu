async function fetchDados() {
  try {
    const res = await fetch('/dados');
    const data = await res.json();

    // Atualiza valores de sensores
    document.getElementById('temperature-value').textContent = data.temperatura.toFixed(1) + ' °C';
    document.getElementById('humidity-value').textContent = data.umidade.toFixed(1) + ' %';
    document.getElementById('conditions-status').textContent = data.situacao;

    // Atualiza tempos das funções
    document.getElementById('tSensor').textContent = data.tempoLerSensor + ' ms';
    document.getElementById('tTemp').textContent = data.tempoLerTemperatura + ' ms';
    document.getElementById('tUmid').textContent = data.tempoLerUmidade + ' ms';
    document.getElementById('tLigar').textContent = data.tempoLigarAlarme + ' ms';
    document.getElementById('tDesligar').textContent = data.tempoDesligarAlarme + ' ms';

    // Atualiza status dos componentes
    document.getElementById('statusBuzzer').textContent = data.alarme ? 'Ativo' : 'Desligado';
    document.getElementById('statusLed').textContent = data.led ? 'Ligado' : 'Desligado';
    document.getElementById('statusLedAzul').textContent = data.led_azul ? 'Ligado' : 'Desligado';

    // Atualiza status visual (ex: alerta)
    const fireAlert = document.getElementById('fire-alert');
    if (data.alarme) {
      fireAlert.classList.remove('hidden');
    } else {
      fireAlert.classList.add('hidden');
    }

    // Atualiza horário da última leitura
    const now = new Date();
    document.getElementById('update-time').textContent = now.toLocaleTimeString();

  } catch (e) {
    console.error('Erro ao buscar /dados', e);
    document.getElementById('connection-alert').classList.remove('hidden');
  }
}

async function toggle(url) {
  try {
    await fetch(url);
    setTimeout(fetchDados, 300); // pequeno atraso para atualização
  } catch (e) {
    console.error('Erro ao enviar comando', e);
  }
}

// Eventos dos botões
document.getElementById('btnToggleBuzzer').addEventListener('click', () => toggle('/toggleBuzzer'));
document.getElementById('btnToggleLed').addEventListener('click', () => toggle('/toggleLed'));
document.getElementById('btnToggleLedAzul').addEventListener('click', () => toggle('/toggleLedAzul'));

// Atualiza a cada 2 segundos
fetchDados();
setInterval(fetchDados, 2000);

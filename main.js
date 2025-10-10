 document.querySelectorAll('.nav-link').forEach(link => {
            link.addEventListener('click', function(e) {
                e.preventDefault();
                
                document.querySelectorAll('.nav-link').forEach(l => l.classList.remove('active'));
                this.classList.add('active');
                
                const tabId = this.getAttribute('data-tab');
                document.querySelectorAll('.tab-content').forEach(tab => tab.classList.remove('active'));
                document.getElementById(tabId).classList.add('active');
            });
        });
        
        function updateSensorData() {
            const temperature = (Math.random() * 40 + 15).toFixed(1);
            const humidity = (Math.random() * 40 + 30).toFixed(1);
            
            document.getElementById('temperatureValue').textContent = `${temperature} °C`;
            document.getElementById('humidityValue').textContent = `${humidity} %`;
            
            const tempStatus = document.getElementById('temperature-status');
            const humidityStatus = document.getElementById('humidity-status');
            
            if (temperature > 57) {
                tempStatus.textContent = 'CRÍTICO';
                tempStatus.className = 'sensor-status status-danger';
                document.getElementById('fire-alert').classList.remove('hidden');
            } else if (temperature > 45) {
                tempStatus.textContent = 'ALTO';
                tempStatus.className = 'sensor-status status-warning';
                document.getElementById('fire-alert').classList.add('hidden');
            } else {
                tempStatus.textContent = 'NORMAL';
                tempStatus.className = 'sensor-status status-ok';
                document.getElementById('fire-alert').classList.add('hidden');
            }
            
            if (humidity < 40 || humidity > 60) {
                humidityStatus.textContent = 'FORA DA FAIXA IDEAL';
                humidityStatus.className = 'sensor-status status-warning';
            } else {
                humidityStatus.textContent = 'IDEAL';
                humidityStatus.className = 'sensor-status status-ok';
            }
            
            const now = new Date();
            document.getElementById('updateTime').textContent = now.toLocaleTimeString();
        }
        
        document.getElementById('toggle-alarm').addEventListener('click', function() {
            const alarmStatus = document.getElementById('alarm-status');
            const alarmButton = document.getElementById('toggle-alarm');
            
            if (alarmStatus.textContent === 'ATIVADO') {
                alarmStatus.textContent = 'DESATIVADO';
                alarmStatus.className = 'alarm-status alarm-off';
                alarmButton.textContent = 'Ativar';
            } else {
                alarmStatus.textContent = 'ATIVADO';
                alarmStatus.className = 'alarm-status alarm-on';
                alarmButton.textContent = 'Desativar';
            }
        });
        
        document.querySelector('.dismiss-alert').addEventListener('click', function() {
            document.getElementById('fire-alert').classList.add('hidden');
        });
        
        updateSensorData();
        
        setInterval(updateSensorData, 5000);
        
        setInterval(() => {
            if (Math.random() < 0.1) { 
                document.getElementById('connection-alert').classList.remove('hidden');
                
                setTimeout(() => {
                    document.getElementById('connection-alert').classList.add('hidden');
                }, 5000);
            }
        }, 15000);
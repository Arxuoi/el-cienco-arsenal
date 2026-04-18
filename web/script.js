// El Cienco Dashboard Controller
class ElCiencoDashboard {
    constructor() {
        this.apiHost = 'localhost';
        this.apiPort = 6969;
        this.connected = false;
        this.attackRunning = false;
        this.attackStartTime = null;
        this.attackDuration = 0;
        this.packetsSent = 0;
        this.updateInterval = null;
        
        this.initElements();
        this.bindEvents();
        this.updateMethodInfo();
    }
    
    initElements() {
        this.elements = {
            connectBtn: document.getElementById('connectBtn'),
            attackBtn: document.getElementById('attackBtn'),
            stopBtn: document.getElementById('stopBtn'),
            targetInput: document.getElementById('targetInput'),
            portInput: document.getElementById('portInput'),
            threadsInput: document.getElementById('threadsInput'),
            durationInput: document.getElementById('durationInput'),
            apiPortInput: document.getElementById('apiPortInput'),
            methodSelect: document.getElementById('methodSelect'),
            connectionStatus: document.getElementById('connectionStatus'),
            systemStatus: document.getElementById('systemStatus'),
            activeThreads: document.getElementById('activeThreads'),
            currentTarget: document.getElementById('currentTarget'),
            packetsSent: document.getElementById('packetsSent'),
            bandwidth: document.getElementById('bandwidth'),
            timeElapsed: document.getElementById('timeElapsed'),
            progressFill: document.getElementById('progressFill'),
            consoleOutput: document.getElementById('consoleOutput'),
            consoleInput: document.getElementById('consoleInput'),
            methodName: document.getElementById('methodName'),
            methodDescription: document.getElementById('methodDescription'),
            methodLayer: document.getElementById('methodLayer'),
            methodSuccess: document.getElementById('methodSuccess'),
            methodStealth: document.getElementById('methodStealth')
        };
    }
    
    bindEvents() {
        this.elements.connectBtn.addEventListener('click', () => this.toggleConnection());
        this.elements.attackBtn.addEventListener('click', () => this.launchAttack());
        this.elements.stopBtn.addEventListener('click', () => this.stopAttack());
        this.elements.methodSelect.addEventListener('change', () => this.updateMethodInfo());
        this.elements.consoleInput.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') this.executeCommand();
        });
    }
    
    updateMethodInfo() {
        const method = parseInt(this.elements.methodSelect.value);
        const methods = [
            { name: 'UDP Flood', desc: 'Volumetric attack sending massive UDP packets to target port. High bandwidth consumption. Effective against unprotected services.', layer: 'L4', success: '85%', stealth: 'LOW' },
            { name: 'TCP SYN Flood', desc: 'Exploits TCP three-way handshake by sending SYN packets without completing connection. Exhausts connection tables.', layer: 'L4', success: '75%', stealth: 'MEDIUM' },
            { name: 'HTTP Flood', desc: 'Layer 7 attack sending valid HTTP requests. Difficult to distinguish from legitimate traffic.', layer: 'L7', success: '60%', stealth: 'HIGH' },
            { name: 'Slowloris', desc: 'Keeps connections open by sending partial HTTP headers. Slowly exhausts server connection pool.', layer: 'L7', success: '70%', stealth: 'VERY HIGH' },
            { name: 'DNS Amplification', desc: 'Spoofed DNS queries to open resolvers causing amplified responses to target.', layer: 'L4', success: '90%', stealth: 'LOW' },
            { name: 'ICMP Flood', desc: 'Ping flood attack overwhelming target with ICMP Echo Request packets.', layer: 'L3', success: '50%', stealth: 'LOW' },
            { name: 'RudyLoris', desc: 'Slow POST attack sending data at extremely low rates. Bypasses many timeout protections.', layer: 'L7', success: '65%', stealth: 'VERY HIGH' },
            { name: 'Cloudflare Bypass', desc: 'Advanced attack mimicking legitimate browsers with rotating headers. Bypasses basic Cloudflare protection.', layer: 'L7', success: '40%', stealth: 'MAXIMUM' }
        ];
        
        const info = methods[method];
        this.elements.methodName.textContent = info.name;
        this.elements.methodDescription.textContent = info.desc;
        this.elements.methodLayer.textContent = info.layer;
        this.elements.methodSuccess.textContent = info.success;
        this.elements.methodStealth.textContent = info.stealth;
    }
    
    log(message, type = 'info') {
        const line = document.createElement('div');
        line.className = `console-line ${type}`;
        line.textContent = `> ${message}`;
        this.elements.consoleOutput.appendChild(line);
        this.elements.consoleOutput.scrollTop = this.elements.consoleOutput.scrollHeight;
        
        // Limit console lines
        while (this.elements.consoleOutput.children.length > 50) {
            this.elements.consoleOutput.removeChild(this.elements.consoleOutput.firstChild);
        }
    }
    
    async toggleConnection() {
        if (!this.connected) {
            this.apiPort = parseInt(this.elements.apiPortInput.value) || 6969;
            this.apiHost = 'localhost'; // Always localhost for security
            
            this.log(`Attempting connection to API on port ${this.apiPort}...`);
            
            try {
                const response = await this.apiRequest('GET', '/status');
                if (response) {
                    this.connected = true;
                    this.elements.connectionStatus.textContent = '● CONNECTED';
                    this.elements.connectionStatus.classList.add('connected');
                    this.elements.connectBtn.textContent = 'DISCONNECT';
                    this.elements.attackBtn.disabled = false;
                    this.elements.stopBtn.disabled = false;
                    this.elements.consoleInput.disabled = false;
                    this.log('Connection established. El Cienco online.', 'success');
                    this.startTelemetry();
                }
            } catch (error) {
                this.log(`Connection failed: ${error.message}`, 'error');
                this.log('Ensure elcienco is running with --api flag', 'warning');
            }
        } else {
            this.disconnect();
        }
    }
    
    disconnect() {
        this.connected = false;
        this.stopTelemetry();
        this.elements.connectionStatus.textContent = '● DISCONNECTED';
        this.elements.connectionStatus.classList.remove('connected');
        this.elements.connectBtn.textContent = 'CONNECT API';
        this.elements.attackBtn.disabled = true;
        this.elements.stopBtn.disabled = true;
        this.elements.consoleInput.disabled = true;
        this.elements.systemStatus.textContent = 'IDLE';
        this.log('Disconnected from API');
    }
    
    async apiRequest(method, endpoint, body = null) {
        const url = `http://${this.apiHost}:${this.apiPort}${endpoint}`;
        const options = { method };
        
        if (body) {
            options.body = JSON.stringify(body);
        }
        
        const response = await fetch(url, options);
        const text = await response.text();
        
        try {
            return JSON.parse(text);
        } catch {
            return text;
        }
    }
    
    async launchAttack() {
        const config = {
            target: this.elements.targetInput.value,
            port: parseInt(this.elements.portInput.value),
            threads: parseInt(this.elements.threadsInput.value),
            duration: parseInt(this.elements.durationInput.value),
            method: parseInt(this.elements.methodSelect.value)
        };
        
        this.log(`Launching ${this.elements.methodName.textContent} against ${config.target}:${config.port}`);
        this.log(`Threads: ${config.threads} | Duration: ${config.duration}s`);
        
        try {
            const response = await this.apiRequest('POST', '/attack', config);
            if (response.status === 'ATTACK_INITIATED') {
                this.attackRunning = true;
                this.attackStartTime = Date.now();
                this.attackDuration = config.duration;
                this.packetsSent = 0;
                
                this.elements.systemStatus.textContent = 'ATTACKING';
                this.elements.currentTarget.textContent = `${config.target}:${config.port}`;
                this.elements.activeThreads.textContent = config.threads;
                this.elements.attackBtn.disabled = true;
                
                this.log('Attack initiated successfully', 'warning');
            }
        } catch (error) {
            this.log(`Attack failed: ${error.message}`, 'error');
        }
    }
    
    async stopAttack() {
        try {
            const response = await this.apiRequest('POST', '/stop');
            if (response.status === 'ATTACK_STOPPED') {
                this.attackRunning = false;
                this.elements.systemStatus.textContent = 'IDLE';
                this.elements.attackBtn.disabled = false;
                this.elements.progressFill.style.width = '0%';
                this.log('Attack stopped by user command', 'warning');
            }
        } catch (error) {
            this.log(`Stop failed: ${error.message}`, 'error');
        }
    }
    
    startTelemetry() {
        this.updateInterval = setInterval(() => this.updateTelemetry(), 1000);
    }
    
    stopTelemetry() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
            this.updateInterval = null;
        }
    }
    
    async updateTelemetry() {
        if (!this.connected) return;
        
        try {
            const status = await this.apiRequest('GET', '/status');
            
            if (this.attackRunning) {
                const elapsed = (Date.now() - this.attackStartTime) / 1000;
                const progress = (elapsed / this.attackDuration) * 100;
                
                this.elements.timeElapsed.textContent = this.formatTime(elapsed);
                this.elements.progressFill.style.width = `${Math.min(progress, 100)}%`;
                
                // Simulate packet count and bandwidth
                this.packetsSent += Math.floor(Math.random() * 1000) + 500;
                this.elements.packetsSent.textContent = this.formatNumber(this.packetsSent);
                this.elements.bandwidth.textContent = `${(Math.random() * 50 + 10).toFixed(1)} Mbps`;
                
                if (elapsed >= this.attackDuration) {
                    this.attackRunning = false;
                    this.elements.systemStatus.textContent = 'COMPLETED';
                    this.elements.attackBtn.disabled = false;
                    this.log('Attack duration completed', 'info');
                }
            }
            
            if (!status.running && this.attackRunning) {
                this.attackRunning = false;
                this.elements.systemStatus.textContent = 'IDLE';
                this.elements.attackBtn.disabled = false;
            }
        } catch (error) {
            this.log('Telemetry error - connection lost', 'error');
            this.disconnect();
        }
    }
    
    executeCommand() {
        const command = this.elements.consoleInput.value.trim();
        if (!command) return;
        
        this.log(command, 'command');
        this.elements.consoleInput.value = '';
        
        // Handle local commands
        if (command === 'clear') {
            this.elements.consoleOutput.innerHTML = '';
            this.log('Console cleared');
        } else if (command === 'status') {
            this.log(`Connected: ${this.connected}`);
            this.log(`Attack Running: ${this.attackRunning}`);
            this.log(`API Port: ${this.apiPort}`);
        } else if (command === 'help') {
            this.log('Available commands: clear, status, help, exit');
        } else if (command === 'exit') {
            this.disconnect();
        } else {
            this.log(`Unknown command: ${command}`, 'error');
        }
    }
    
    formatTime(seconds) {
        const mins = Math.floor(seconds / 60);
        const secs = Math.floor(seconds % 60);
        return `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }
    
    formatNumber(num) {
        if (num >= 1000000) return (num / 1000000).toFixed(1) + 'M';
        if (num >= 1000) return (num / 1000).toFixed(1) + 'K';
        return num.toString();
    }
}

// Initialize dashboard
document.addEventListener('DOMContentLoaded', () => {
    window.dashboard = new ElCiencoDashboard();
});

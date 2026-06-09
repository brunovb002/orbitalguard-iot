# 🛰️ OrbitalGuard — IoT

> \*\*Tecnologia Espacial Salvando Vidas na Terra\*\*

Sistema inteligente de monitoramento e prevenção de enchentes utilizando ESP32, sensores IoT, MQTT e dashboard em tempo real.

\---

## 👥 Integrantes

|Nome|RM|
|-|-|
|Bruno Vinicius Barbosa|566366|
|Raphael Gomes Mancera|562279|
|Guilherme de Andrade Martini|566087|
|Gabriel Gomes Mancera|555427|
|João Victor Rebello de Santis|555287|

**Turma:** 2TDSPX — Fevereiro 2026  
**Disciplina:** Disruptive Architectures: IoT, IoB \& Generative IA  
**Global Solution:** 2026/1

\---

## 📋 Sobre o Projeto

O **OrbitalGuard** é um sistema de monitoramento de enchentes que combina sensores IoT com dados climáticos e espaciais para prever e alertar sobre riscos em tempo real.

O componente IoT simula uma estação de monitoramento instalada às margens de um rio, com um sensor ultrassônico apontado para baixo medindo a distância até a superfície da água. Conforme o nível do rio sobe, a distância diminui e o sistema aciona alertas progressivos.

### Problema Abordado

Muitas cidades brasileiras não possuem sistemas eficientes de alerta para enchentes, resultando em perdas de vidas e danos materiais. O OrbitalGuard propõe uma solução acessível e escalável combinando IoT com monitoramento contínuo.

### Conexão com os ODS da ONU

* **ODS 9** — Indústria, inovação e infraestrutura
* **ODS 11** — Cidades e comunidades sustentáveis
* **ODS 13** — Ação contra a mudança global do clima

\---

## 🔧 Arquitetura do Sistema

```
ESP32 (Wokwi)
    │
    ├── HC-SR04 (nível da água)
    ├── Botão de pânico (alerta manual)
    ├── LED Verde (risco baixo/médio)
    ├── LED Vermelho (risco alto)
    ├── Buzzer (alarme sonoro)
    └── Display OLED (interface local)
         │
         └── Wi-Fi → MQTT (HiveMQ Cloud)
                          │
                          └── Node-RED → Dashboard
```

\---

## 🛠️ Componentes

### Hardware (Simulado no Wokwi)

|Componente|Pino ESP32|Função|
|-|-|-|
|HC-SR04 TRIG|D5|Disparo do ultrassom|
|HC-SR04 ECHO|D18|Leitura do eco|
|Botão de Pânico|D19|Alerta manual|
|LED Verde|D26|Indicador risco baixo/médio|
|LED Vermelho|D27|Indicador risco alto|
|Buzzer|D14|Alarme sonoro|
|OLED SDA|D21|Display I2C|
|OLED SCL|D22|Display I2C|

### Software

* **Firmware:** Arduino (C++) via PlatformIO
* **Broker MQTT:** HiveMQ Cloud (TLS porta 8883)
* **Dashboard:** Node-RED + node-red-dashboard
* **Simulador:** Wokwi + VS Code

\---

## 📡 Tópicos MQTT

|Tópico|Tipo|Descrição|Exemplo|
|-|-|-|-|
|`orbitalguard/nivel`|Publish|Distância em cm do sensor ao rio|`36.5`|
|`orbitalguard/risco`|Publish|Nível de risco calculado|`ALTO` / `MEDIO` / `BAIXO`|
|`orbitalguard/alerta`|Publish|Status do alerta manual|`1` (ativo) / `0` (normal)|

\---

## 🚦 Lógica de Risco

O sensor HC-SR04 é instalado a **150cm** acima do rio quando está seco. Conforme a água sobe, a distância diminui:

|Distância|Nível (%)|Risco|Saídas|
|-|-|-|-|
|> 60cm|< 60%|🟢 BAIXO|LED Verde aceso|
|30–60cm|60–80%|🟡 MÉDIO|LED Verde piscando|
|< 30cm|> 80%|🔴 ALTO|LED Vermelho + Buzzer|
|Botão pressionado|100%|🔴 ALTO|LED Vermelho + Buzzer|

\---

## 📊 Dashboard (Node-RED)

O dashboard exibe em tempo real:

* **Gauge** do nível da água em %
* **Distância** em cm ao sensor
* **Nível de Risco** com cores (verde/amarelo/vermelho)
* **Status do Alerta Manual**
* **Última atualização** (timestamp)
* **Status da conexão MQTT**
* **Gráfico histórico** do nível da água

\---

## 🚀 Como Executar

### Pré-requisitos

* [VS Code](https://code.visualstudio.com/)
* [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)
* [Wokwi Extension for VS Code](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode)
* [Node-RED](https://nodered.org/docs/getting-started/windows)

### 1\. Clonar o repositório

```bash
git clone https://github.com/brunovb002/orbitalguard-iot.git
cd orbitalguard-iot
```

### 2\. Compilar o firmware

Abra a pasta no VS Code com PlatformIO instalado e compile:

```
Ctrl+Shift+P → PlatformIO: Build
```

### 3\. Executar a simulação

Abra o arquivo `diagram.json` no VS Code e clique em **Start Simulation**.

### 4\. Importar o flow no Node-RED

1. Inicie o Node-RED: `node-red`
2. Acesse `http://127.0.0.1:1880`
3. Menu (≡) → **Import** → cole o conteúdo de `nodered-flow.json`
4. Configure o broker MQTT nos nós roxos com suas credenciais HiveMQ
5. Clique em **Deploy**

### 5\. Acessar o dashboard

```
http://127.0.0.1:1880/ui
```

\---

## 📁 Estrutura do Repositório

```
orbitalguard-iot/
├── src/
│   └── orbitalguard.ino      # Firmware do ESP32
├── diagram.json               # Circuito do Wokwi
├── wokwi.toml                 # Configuração do simulador
├── platformio.ini             # Dependências do projeto
├── nodered-flow.json          # Flow do Node-RED
└── README.md                  # Documentação
```

\---

## 📦 Dependências

```ini
lib\_deps =
    knolleary/PubSubClient @ ^2.8
    adafruit/Adafruit SSD1306 @ ^2.5.7
    adafruit/Adafruit GFX Library @ ^1.11.9
```

\---

## 🎥 Vídeo de Demonstração

> Link do vídeo no YouTube: https://youtu.be/NHOOTDonkQY

\---

## 📄 Licença

Projeto desenvolvido para fins acadêmicos — FIAP Global Solution 2026/1.


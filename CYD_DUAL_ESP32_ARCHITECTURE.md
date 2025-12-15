# 🔗 Arquitetura Dual ESP32: CYD + ESP32 Externo

## 🎯 Conceito

Usar o **ESP32-2432S028R (CYD)** exclusivamente como **display/interface** e um **ESP32 externo** para gerenciar o **MFRC522**, comunicando-se entre si através dos conectores disponíveis.

---

## 🏗️ Arquitetura do Sistema

```
┌─────────────────────────────────────────┐
│   ESP32-2432S028R (CYD)                 │
│   ┌─────────────────────────────────┐   │
│   │  Display 2.8" TFT + Touch       │   │
│   │  LVGL Interface                 │   │
│   │  - QR Code Display              │   │
│   │  - Status Messages              │   │
│   │  - Animações                    │   │
│   └─────────────────────────────────┘   │
│                                          │
│   ESP32-WROOM-32 (Interno)              │
│   - Controla Display                    │
│   - Renderiza LVGL                      │
│   - Recebe dados via UART/I2C           │
│                                          │
│   Conectores: P3, CN1, P5               │
│      │                                   │
│      │ UART/I2C                         │
│      │ Power (5V/3.3V)                  │
└──────┼───────────────────────────────────┘
       │
       │ ┌─── VIN (5V)
       │ ┌─── TX/RX ou SDA/SCL
       │ └─── GND
       │
┌──────▼───────────────────────────────────┐
│   ESP32 Externo (DevKit)                 │
│                                          │
│   ESP32-WROOM-32 / ESP32-DevKitC        │
│   - Controla MFRC522                    │
│   - Lê tags RFID/NFC                    │
│   - Parser NDEF                         │
│   - Envia dados via UART/I2C            │
│                                          │
│   ┌─────────────────────┐               │
│   │    MFRC522          │               │
│   │    SPI Interface    │               │
│   │    - Leitura NTAG   │               │
│   │    - Parser NDEF    │               │
│   └─────────────────────┘               │
└─────────────────────────────────────────┘
```

---

## ✅ Vantagens desta Arquitetura

| Vantagem | Descrição |
|----------|-----------|
| 🟢 **Sem Soldagem** | CYD mantém-se intacto, sem modificações |
| 🟢 **Modular** | Componentes independentes e testáveis |
| 🟢 **Manutenção Fácil** | Trocar qualquer parte sem afetar outra |
| 🟢 **Pinagem Livre** | ESP32 externo tem todos GPIOs disponíveis |
| 🟢 **Debugging Simples** | Cada ESP32 com seu próprio Serial Monitor |
| 🟢 **Escalável** | Fácil adicionar mais sensores no ESP32 externo |
| 🟢 **Alimentação Simples** | CYD alimenta o ESP32 externo diretamente |
| 🟢 **Custo Baixo** | ESP32-DevKitC ~$3-5 USD |

---

## 🔌 Opções de Comunicação

### 📊 Comparação dos Protocolos

| Protocolo | Velocidade | Fios | Distância | Complexidade | Recomendado |
|-----------|------------|------|-----------|--------------|-------------|
| **UART (Serial)** | 115200 baud | 3 (TX,RX,GND) | ~3m | 🟢 Baixa | ✅ **SIM** |
| **I2C** | 100-400 kHz | 4 (SDA,SCL,GND,3V3) | ~1m | 🟡 Média | ✅ **SIM** |
| **SPI** | MHz | 5+ | Curta | 🔴 Alta | ❌ Não (pinos ocupados) |

---

## 🏆 Solução 1: UART (Serial) - MAIS SIMPLES ⭐

### ⚡ Características
- ✅ **Mais fácil de implementar**
- ✅ **3 fios apenas** (TX, RX, GND)
- ✅ **Protocolo simples** (texto ou JSON)
- ✅ **Debugging independente** (Serial Monitor em cada)
- ✅ **Bidirecional** (CYD pode enviar comandos)
- ✅ **Alimentação separada** (flexível)

### 🔧 Conexões Físicas

#### **CYD ←→ ESP32 Externo**

| CYD (P5 Connector) | ESP32 Externo | Função |
|-------------------|---------------|--------|
| Pin 2 (GPIO 1 - TX) | GPIO 16 (RX2) | CYD transmite |
| Pin 3 (GPIO 3 - RX) | GPIO 17 (TX2) | CYD recebe |
| Pin 4 (GND) | GND | Ground comum |
| Pin 1 (VIN - 5V) | VIN / 5V | 🔴 Alimentação ESP32 |

**⚠️ Importante**: 
- GPIO 1 e 3 do CYD são do **Serial0** (debug)
- Precisamos configurar **Serial customizado** no CYD
- Ou usar GPIO 22 e 27 com SoftwareSerial

### 🔧 Conexões Melhoradas (GPIO alternativos)

| CYD (CN1) | ESP32 Externo | Função |
|-----------|---------------|--------|
| Pin 2 (GPIO 22) | GPIO 16 (RX2) | CYD TX |
| Pin 3 (GPIO 27) | GPIO 17 (TX2) | CYD RX |
| Pin 1 (GND) | GND | Ground |
| Pin 4 (3.3V) | 3.3V | Alimentação |

### 💡 Alimentação do ESP32 Externo

#### **Opção A: Via VIN (5V) - RECOMENDADA**
```
CYD P5 Pin 1 (VIN) → ESP32 VIN
CYD P5 Pin 4 (GND) → ESP32 GND
```
- ✅ ESP32 regula internamente para 3.3V
- ✅ Suficiente para ESP32 + MFRC522 (~200mA)
- ⚠️ Verifique se CYD suporta corrente extra

#### **Opção B: Via 3.3V (CN1)**
```
CYD CN1 Pin 4 (3.3V) → ESP32 3.3V
CYD CN1 Pin 1 (GND) → ESP32 GND
```
- ✅ Direto, sem regulador
- ⚠️ Limitado pela fonte 3.3V do CYD (~500mA)
- ✅ Suficiente para ESP32 + MFRC522

#### **Opção C: Alimentação Externa (Mais Seguro)**
```
Fonte Externa 5V → CYD VIN
Fonte Externa 5V → ESP32 VIN
GND comum entre CYD e ESP32
```
- ✅ Sem sobrecarga no CYD
- ✅ Cada módulo com alimentação própria
- ✅ Mais confiável

### 💻 Código - ESP32 Externo (Reader)

```cpp
#include <SPI.h>
#include <MFRC522.h>

// ============================================
// ESP32 Externo - MFRC522 Reader
// ============================================
#define SS_PIN    5
#define RST_PIN   22
#define SCK_PIN   18
#define MISO_PIN  19
#define MOSI_PIN  23

// UART para comunicação com CYD
#define RX2_PIN   16
#define TX2_PIN   17

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  // Serial0 para debug
  Serial.begin(115200);
  
  // Serial2 para comunicação com CYD
  Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
  
  // Inicializa SPI e MFRC522
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  
  Serial.println("ESP32 Reader: Aguardando tags...");
  sendToCYD("READY");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    
    // Lê UID
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    
    // Lê dados da tag (função já existente)
    String ndefUrl = readNDEFUrl();
    
    // Envia dados para CYD via UART
    sendTagData(uid, ndefUrl);
    
    mfrc522.PICC_HaltA();
    delay(1000);
  }
}

void sendTagData(String uid, String url) {
  // Protocolo simples: TAG|UID|URL
  String message = "TAG|" + uid + "|" + url;
  Serial2.println(message);
  
  Serial.println("Enviado para CYD: " + message);
}

void sendToCYD(String message) {
  Serial2.println(message);
}

String readNDEFUrl() {
  // Implementação do parser NDEF (código já existente)
  // Retorna URL extraída ou string vazia
  return "https://example.com";
}
```

### 💻 Código - CYD (Display)

```cpp
#include <lvgl.h>
#include <TFT_eSPI.h>

// ============================================
// CYD - Display + LVGL
// ============================================
#define RX_PIN   27  // CN1 Pin 3
#define TX_PIN   22  // CN1 Pin 2

TFT_eSPI tft = TFT_eSPI();

// LVGL objects
lv_obj_t *label_status;
lv_obj_t *label_uid;
lv_obj_t *qr_code;

void setup() {
  // Serial0 para debug
  Serial.begin(115200);
  
  // Serial2 para comunicação com Reader
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Inicializa display
  tft.begin();
  tft.setRotation(1);
  
  // Inicializa LVGL
  initLVGL();
  createUI();
  
  showStatus("Aguardando leitor...");
  Serial.println("CYD Display: Aguardando dados...");
}

void loop() {
  lv_timer_handler();
  
  // Verifica dados do Reader
  if (Serial2.available()) {
    String message = Serial2.readStringUntil('\n');
    processMessage(message);
  }
  
  delay(10);
}

void processMessage(String message) {
  Serial.println("Recebido: " + message);
  
  if (message.startsWith("TAG|")) {
    // Parse: TAG|UID|URL
    int firstSep = message.indexOf('|');
    int secondSep = message.indexOf('|', firstSep + 1);
    
    String uid = message.substring(firstSep + 1, secondSep);
    String url = message.substring(secondSep + 1);
    
    // Atualiza interface
    showTagDetected(uid, url);
    
    // Volta para standby após 5 segundos
    delay(5000);
    showStatus("Aguardando tag...");
    hideQRCode();
    
  } else if (message == "READY") {
    showStatus("Sistema pronto!");
  }
}

void showTagDetected(String uid, String url) {
  lv_label_set_text(label_status, "Tag Detectada!");
  lv_label_set_text_fmt(label_uid, "UID: %s", uid.c_str());
  
  if (url.length() > 0) {
    showQRCode(url.c_str());
  }
}

void showStatus(String message) {
  lv_label_set_text(label_status, message.c_str());
}

void createUI() {
  // Cria interface LVGL (código omitido por brevidade)
  // Labels, QR Code widget, etc.
}

void showQRCode(const char* data) {
  lv_qrcode_update(qr_code, data, strlen(data));
  lv_obj_clear_flag(qr_code, LV_OBJ_FLAG_HIDDEN);
}

void hideQRCode() {
  lv_obj_add_flag(qr_code, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(label_uid, "");
}
```

---

## 🎨 Solução 2: I2C (Master-Slave)

### ⚡ Características
- ✅ **Protocolo padrão** (biblioteca Wire)
- ✅ **4 fios** (SDA, SCL, GND, VCC)
- ✅ **Permite múltiplos devices** (escalável)
- ⚠️ **Mais complexo** que UART
- ✅ **CYD como Master**, ESP32 externo como Slave

### 🔧 Conexões Físicas

| CYD (CN1) | ESP32 Externo | Função |
|-----------|---------------|--------|
| Pin 3 (GPIO 27) | GPIO 21 (SDA) | Dados I2C |
| Pin 2 (GPIO 22) | GPIO 22 (SCL) | Clock I2C |
| Pin 4 (3.3V) | 3.3V | Alimentação |
| Pin 1 (GND) | GND | Ground |

**⚠️ Nota**: Adicione resistores pull-up de 4.7kΩ em SDA e SCL se não funcionarem nativamente.

### 💻 Código - ESP32 Externo (I2C Slave)

```cpp
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

// ============================================
// ESP32 Externo - I2C Slave
// ============================================
#define I2C_SLAVE_ADDR  0x42
#define SDA_PIN         21
#define SCL_PIN         22

#define SS_PIN    5
#define RST_PIN   4
#define SCK_PIN   18
#define MISO_PIN  19
#define MOSI_PIN  23

MFRC522 mfrc522(SS_PIN, RST_PIN);

String lastUID = "";
String lastURL = "";
bool newDataAvailable = false;

void setup() {
  Serial.begin(115200);
  
  // Configura I2C Slave
  Wire.begin(SDA_PIN, SCL_PIN, I2C_SLAVE_ADDR);
  Wire.onRequest(onI2CRequest);
  
  // Inicializa MFRC522
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  
  Serial.println("ESP32 Reader (I2C Slave): Ready");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    
    // Lê UID
    lastUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      if (mfrc522.uid.uidByte[i] < 0x10) lastUID += "0";
      lastUID += String(mfrc522.uid.uidByte[i], HEX);
    }
    lastUID.toUpperCase();
    
    // Lê URL NDEF
    lastURL = readNDEFUrl();
    
    newDataAvailable = true;
    Serial.println("Tag lida: " + lastUID);
    
    mfrc522.PICC_HaltA();
    delay(1000);
  }
}

// Callback quando CYD (Master) solicita dados
void onI2CRequest() {
  if (newDataAvailable) {
    String message = "TAG|" + lastUID + "|" + lastURL;
    Wire.write(message.c_str(), message.length());
    newDataAvailable = false;
  } else {
    Wire.write("IDLE", 4);
  }
}

String readNDEFUrl() {
  // Parser NDEF (código já existente)
  return "https://example.com";
}
```

### 💻 Código - CYD (I2C Master)

```cpp
#include <Wire.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// ============================================
// CYD - I2C Master
// ============================================
#define I2C_SLAVE_ADDR  0x42
#define SDA_PIN         27
#define SCL_PIN         22

void setup() {
  Serial.begin(115200);
  
  // Configura I2C Master
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // Inicializa display e LVGL
  initDisplay();
  
  Serial.println("CYD Display (I2C Master): Ready");
}

void loop() {
  lv_timer_handler();
  
  // Poll Reader a cada 500ms
  static unsigned long lastPoll = 0;
  if (millis() - lastPoll > 500) {
    lastPoll = millis();
    
    String message = requestDataFromReader();
    if (message.startsWith("TAG|")) {
      processTagData(message);
    }
  }
  
  delay(10);
}

String requestDataFromReader() {
  Wire.requestFrom(I2C_SLAVE_ADDR, 128);
  
  String data = "";
  while (Wire.available()) {
    char c = Wire.read();
    data += c;
  }
  
  return data;
}

void processTagData(String message) {
  // Parse e exibe (mesmo código da Solução 1)
  Serial.println("Tag recebida: " + message);
  // ... atualiza UI ...
}
```

---

## 🔋 Análise de Alimentação

### 📊 Consumo de Corrente

| Componente | Corrente Típica | Pico |
|------------|----------------|------|
| CYD completo | ~150-200mA | ~300mA |
| ESP32 externo | ~80-160mA | ~240mA |
| MFRC522 | ~13-26mA | ~50mA |
| **Total** | ~243-386mA | ~590mA |

### 🔌 Capacidade dos Conectores CYD

#### **VIN (5V) no P5**
- ✅ Conectado diretamente ao USB (até 500mA)
- ✅ Pode alimentar ESP32 externo via VIN
- ⚠️ Verifique fonte USB suficiente

#### **3.3V no CN1**
- ⚠️ Vem do regulador AMS1117 do CYD
- ⚠️ Corrente máxima: ~800mA (teórico)
- ⚠️ Em uso: CYD já consome ~300mA
- 🟡 Sobra ~500mA (suficiente, mas justo)

### ✅ Recomendação de Alimentação

```
Fonte USB 5V/1A
    │
    ├─→ CYD VIN (P5 Pin 1)
    │   ├─ CYD interno (~200mA)
    │   └─ CN1 3.3V out → ESP32 externo 3.3V
    │
    └─→ GND comum
```

**Ou melhor ainda:**

```
Fonte USB 5V/2A (adaptador de parede)
    │
    ├─→ CYD VIN
    │
    └─→ ESP32 Externo VIN
         └─ GND comum
```

---

## 🛠️ Configuração PlatformIO

### platformio.ini

```ini
; ============================================
; Projeto Dual ESP32: CYD Display + Reader
; ============================================

[platformio]
default_envs = cyd-display, reader-rfid

; ============================================
; CYD - Display Controller
; ============================================
[env:cyd-display]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_port = COM3  ; Ajuste conforme necessário

lib_deps = 
    bodmer/TFT_eSPI @ ^2.5.43
    lvgl/lvgl @ ^8.4.0

build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_ESP32_CYD=1
    -DUSER_SETUP_LOADED=1
    
    ; Display config
    -DILI9341_DRIVER=1
    -DTFT_WIDTH=240
    -DTFT_HEIGHT=320
    -DTFT_MOSI=13
    -DTFT_MISO=12
    -DTFT_SCLK=14
    -DTFT_CS=15
    -DTFT_DC=2
    -DTFT_RST=-1
    -DTFT_BL=21
    -DTOUCH_CS=33
    
    ; UART para Reader
    -DUART_RX_PIN=27
    -DUART_TX_PIN=22

; ============================================
; ESP32 Externo - RFID Reader
; ============================================
[env:reader-rfid]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_port = COM4  ; Ajuste conforme necessário

lib_deps = 
    miguelbalboa/MFRC522 @ ^1.4.12

build_flags = 
    -DCORE_DEBUG_LEVEL=3
    
    ; MFRC522 SPI pins
    -DSS_PIN=5
    -DRST_PIN=22
    -DSCK_PIN=18
    -DMISO_PIN=19
    -DMOSI_PIN=23
    
    ; UART para CYD
    -DUART_RX_PIN=16
    -DUART_TX_PIN=17
```

---

## 📦 Organização do Projeto

### Estrutura Recomendada

```
RFID-Reader-Dual-ESP32/
├── cyd-display/
│   ├── src/
│   │   └── main.cpp           # CYD controller
│   ├── include/
│   │   ├── ui.h               # LVGL interface
│   │   └── comm.h             # UART/I2C handler
│   └── platformio.ini
│
├── reader-rfid/
│   ├── src/
│   │   └── main.cpp           # RFID reader
│   ├── include/
│   │   ├── ndef_parser.h      # Parser NDEF
│   │   └── comm.h             # UART/I2C handler
│   └── platformio.ini
│
├── common/
│   ├── protocol.h             # Protocolo de comunicação
│   └── messages.h             # Estruturas de mensagens
│
└── README.md
```

---

## 🔄 Protocolo de Comunicação

### Formato de Mensagens (UART)

```
[TIPO]|[CAMPO1]|[CAMPO2]|...|[CAMPON]\n

Exemplos:
TAG|04A1B2C3D4E5F6|https://example.com\n
STATUS|READY\n
ERROR|TIMEOUT\n
CMD|BEEP\n
```

### Mensagens Definidas

| Tipo | Direção | Formato | Descrição |
|------|---------|---------|-----------|
| TAG | Reader→CYD | TAG\|UID\|URL | Tag detectada |
| STATUS | Reader→CYD | STATUS\|texto | Status do reader |
| ERROR | Reader→CYD | ERROR\|codigo | Erro no reader |
| CMD | CYD→Reader | CMD\|comando | Comando do CYD |
| ACK | Ambos | ACK | Confirmação |

### 💻 Biblioteca de Protocolo

```cpp
// protocol.h - Compartilhado entre ambos ESP32s

#define MSG_TAG     "TAG"
#define MSG_STATUS  "STATUS"
#define MSG_ERROR   "ERROR"
#define MSG_CMD     "CMD"
#define MSG_ACK     "ACK"

struct TagMessage {
  String uid;
  String url;
  String text;
  int type;  // 0=raw, 1=URL, 2=Text
};

class CommProtocol {
public:
  static String encodeTag(TagMessage tag) {
    return String(MSG_TAG) + "|" + 
           tag.uid + "|" + 
           tag.url + "|" + 
           tag.text + "|" + 
           String(tag.type);
  }
  
  static TagMessage decodeTag(String message) {
    TagMessage tag;
    int sep1 = message.indexOf('|');
    int sep2 = message.indexOf('|', sep1 + 1);
    int sep3 = message.indexOf('|', sep2 + 1);
    int sep4 = message.indexOf('|', sep3 + 1);
    
    tag.uid = message.substring(sep1 + 1, sep2);
    tag.url = message.substring(sep2 + 1, sep3);
    tag.text = message.substring(sep3 + 1, sep4);
    tag.type = message.substring(sep4 + 1).toInt();
    
    return tag;
  }
};
```

---

## 🎯 Vantagens vs. Solução Única

| Aspecto | Dual ESP32 | Single ESP32 (CYD) |
|---------|------------|-------------------|
| **Soldagem** | ❌ Não necessária | ⚠️ 4 pinos no CYD |
| **Modularidade** | ✅ Alta | ❌ Tudo acoplado |
| **Manutenção** | ✅ Fácil | ⚠️ Difícil |
| **Debug** | ✅ Independente | ⚠️ Compartilhado |
| **Custo** | 🟡 +$5 (ESP32 extra) | ✅ Apenas CYD |
| **Complexidade** | 🟡 Protocolo extra | ✅ Código único |
| **Confiabilidade** | ✅ Alta | ✅ Alta |
| **GPIOs Livres** | ✅ Muitos (ESP32 ext) | ❌ Poucos (CYD) |
| **Performance** | ✅ Processamento paralelo | 🟡 Single-thread |

---

## 🚀 Plano de Implementação

### Fase 1: Prototipagem (1-2 dias)
1. ✅ Conectar hardware (UART)
2. ✅ Testar comunicação básica (echo)
3. ✅ Validar alimentação do ESP32 externo
4. ✅ MFRC522 lendo tags (standalone)

### Fase 2: Integração (2-3 dias)
1. ✅ Implementar protocolo de mensagens
2. ✅ ESP32 externo enviando dados de tags
3. ✅ CYD recebendo e processando
4. ✅ Display mostrando informações

### Fase 3: Interface LVGL (2-3 dias)
1. ✅ Design da UI no CYD
2. ✅ QR Code widget
3. ✅ Animações e transições
4. ✅ Status e feedback visual

### Fase 4: Refinamento (1-2 dias)
1. ✅ Tratamento de erros
2. ✅ Timeout e reconexão
3. ✅ Otimização de performance
4. ✅ Testes extensivos

**Total estimado**: 6-10 dias de desenvolvimento

---

## 🛒 Lista de Materiais

| Item | Quantidade | Preço (USD) | Link |
|------|-----------|-------------|------|
| ESP32-2432S028R (CYD) | 1 | ~$12 | AliExpress |
| ESP32-DevKitC v4 | 1 | ~$5 | AliExpress |
| MFRC522 Module | 1 | ~$2 | AliExpress |
| Jumpers fêmea-fêmea | 10 | ~$1 | AliExpress |
| Fonte USB 5V/2A | 1 | ~$3 | AliExpress |
| Protoboard 400 pontos | 1 | ~$2 | AliExpress |
| **Total** | - | **~$25** | - |

---

## ⚠️ Considerações Importantes

### 1. **Isolamento de Grounds**
- ✅ GND comum entre CYD e ESP32 externo
- ⚠️ Evite ground loops com múltiplas fontes

### 2. **Níveis Lógicos**
- ✅ ESP32 e CYD operam em 3.3V (compatíveis)
- ✅ Não precisa level shifters

### 3. **Comprimento dos Fios**
- UART: até ~3m
- I2C: até ~1m (adicione pull-ups se >30cm)

### 4. **Interferência**
- Display pode gerar ruído no UART
- Use fios blindados se houver problemas
- Adicione capacitores de desacoplamento (100nF)

### 5. **Serial Debug**
- CYD: Serial0 livre (GPIO 1, 3 não usados)
- Reader: Serial0 livre para debug

---

## 🎊 Conclusão

### **RECOMENDAÇÃO: Arquitetura Dual ESP32 com UART** ✅

**Razões:**
1. ✅ **Sem soldagem** no CYD
2. ✅ **Modular e manutenível**
3. ✅ **Debug independente**
4. ✅ **Escalável** (adicionar sensores)
5. ✅ **Custo baixo** (+$5 apenas)
6. ✅ **Implementação simples** (UART)
7. ✅ **Performance excelente**

### Configuração Recomendada:

```
CYD (Display)
  ├─ CN1 Pin 2 (GPIO 22) → ESP32 RX
  ├─ CN1 Pin 3 (GPIO 27) → ESP32 TX
  ├─ CN1 Pin 4 (3.3V) → ESP32 3.3V
  └─ CN1 Pin 1 (GND) → ESP32 GND

ESP32 Externo (Reader)
  ├─ GPIO 16 (RX2) ← CYD TX
  ├─ GPIO 17 (TX2) → CYD RX
  ├─ SPI (5,18,19,23) → MFRC522
  └─ GPIO 22 (RST) → MFRC522
```

**Esta é a solução IDEAL para seu projeto!** 🏆

---

**Versão**: 1.0  
**Data**: Dezembro 2024  
**Status**: ✅ ARQUITETURA RECOMENDADA

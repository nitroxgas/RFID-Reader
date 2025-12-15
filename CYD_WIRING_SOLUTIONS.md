# 🔌 Soluções de Conexão: MFRC522 + CYD ESP32-2432S028R

## 🎯 Três Soluções Completas

Este documento apresenta **3 soluções práticas** para conectar o MFRC522 ao ESP32-2432S028R (CYD), da mais simples para a mais avançada.

---

## 📋 Conectores Disponíveis no CYD

### P3 (4 pinos - na lateral)
```
1. GND
2. GPIO 35  (Input-only, ADC)
3. GPIO 22  (I2C SCL)
4. GPIO 21  (⚠️ Backlight - sempre HIGH)
```

### CN1 (4 pinos - na lateral)
```
1. GND
2. GPIO 22  (repetido)
3. GPIO 27  (pode ser I2C SDA)
4. 3.3V
```

### P5 (4 pinos - TX/RX)
```
1. VIN (5V input)
2. GPIO 1  (TX)
3. GPIO 3  (RX)
4. GND
```

---

## 🏆 Solução 1: Hardware SPI (VSPI) - Melhor Performance

### ⚡ Características
- ✅ **SPI nativo** (hardware)
- ✅ **Performance máxima**
- ⚠️ **Requer soldagem** em 4 pinos
- ✅ SD Card desabilitado
- ✅ Display funciona no HSPI

### 📌 Pinagem

```cpp
#define SS_PIN    5    // ⚠️ Solda
#define RST_PIN   27   // ✅ CN1 Pin 3
#define SCK_PIN   18   // ⚠️ Solda
#define MISO_PIN  19   // ⚠️ Solda
#define MOSI_PIN  23   // ⚠️ Solda
```

### 🔧 Tabela de Conexões

| MFRC522 Pin | CYD GPIO | Localização | Método |
|-------------|----------|-------------|--------|
| SDA/SS      | GPIO 5   | ESP32 chip pad | 🔴 Soldagem |
| SCK         | GPIO 18  | ESP32 chip pad | 🔴 Soldagem |
| MOSI        | GPIO 23  | ESP32 chip pad | 🔴 Soldagem |
| MISO        | GPIO 19  | ESP32 chip pad | 🔴 Soldagem |
| RST         | GPIO 27  | CN1 Pin 3 | ✅ Conector |
| 3.3V        | 3.3V     | CN1 Pin 4 | ✅ Conector |
| GND         | GND      | CN1 Pin 1 | ✅ Conector |

### 🛠️ Onde Soldar

#### **Opção A: Pads do ESP32-WROOM-32**

Localize os pads no módulo ESP32:
```
Pad 8:  GPIO 5  (SS)
Pad 22: GPIO 18 (SCK)
Pad 25: GPIO 19 (MISO)
Pad 27: GPIO 23 (MOSI)
```

#### **Opção B: Traces da PCB**

Solde fios finos nos traces que vão para o slot SD Card:
- SD_CS trace → GPIO 5
- SD_SCK trace → GPIO 18
- SD_MISO trace → GPIO 19
- SD_MOSI trace → GPIO 23

### 💻 Código Exemplo

```cpp
#include <SPI.h>
#include <MFRC522.h>

// ============================================
// ESP32-2432S028R (CYD) - Hardware SPI (VSPI)
// ============================================
#define SS_PIN    5    // VSPI CS (era SD_CS)
#define RST_PIN   27   // CN1 disponível
#define SCK_PIN   18   // VSPI_SCK
#define MISO_PIN  19   // VSPI_MISO
#define MOSI_PIN  23   // VSPI_MOSI

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  
  // Inicializa VSPI com pinos customizados
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  
  // Inicializa MFRC522
  mfrc522.PCD_Init();
  
  Serial.println("MFRC522 pronto! (Hardware SPI)");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    
    mfrc522.PICC_HaltA();
    delay(1000);
  }
}
```

### ⚠️ Atenção

1. **Desabilite SD Card** no código TFT_eSPI
2. **Solda SMD** requer habilidade
3. Use fios **finos** (30 AWG)
4. Teste continuidade com multímetro

---

## 🎨 Solução 2: I2C + Conversor SC18IS602B - Sem Soldagem!

### ⚡ Características
- ✅ **Sem soldagem** no CYD
- ✅ Usa apenas **CN1** (I2C)
- ⚠️ Requer módulo **SC18IS602B** (~$5-8)
- ✅ Display + Touch + SD funcionam normalmente
- ⚠️ Latência adicional (I2C → SPI)

### 📌 Pinagem

```cpp
// CYD I2C → SC18IS602B
#define I2C_SDA   27   // CN1 Pin 3
#define I2C_SCL   22   // CN1 Pin 2

// SC18IS602B → MFRC522 (automático)
// SS, SCK, MOSI, MISO → SC18IS602B cuida
```

### 🔧 Conexões

#### **CYD ←→ SC18IS602B**

| CYD | SC18IS602B |
|-----|------------|
| GPIO 22 (CN1 Pin 2) | SCL |
| GPIO 27 (CN1 Pin 3) | SDA |
| 3.3V (CN1 Pin 4) | VDD |
| GND (CN1 Pin 1) | GND |

#### **SC18IS602B ←→ MFRC522**

| SC18IS602B | MFRC522 |
|------------|---------|
| SS0 | SDA/SS |
| SCLK | SCK |
| MOSI | MOSI |
| MISO | MISO |
| GPIO (livre) | RST |
| VDD | 3.3V |
| GND | GND |

### 💻 Código Exemplo

```cpp
#include <Wire.h>
#include <MFRC522_I2C.h>  // Biblioteca modificada

// ============================================
// ESP32-2432S028R (CYD) - I2C Bridge
// ============================================
#define I2C_SDA   27
#define I2C_SCL   22
#define RST_PIN   -1  // Controlado via I2C

// SC18IS602B endereço I2C padrão: 0x28
MFRC522_I2C mfrc522(0x28, RST_PIN);

void setup() {
  Serial.begin(115200);
  
  // Inicializa I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Inicializa MFRC522 via I2C
  mfrc522.PCD_Init();
  
  Serial.println("MFRC522 pronto! (I2C Bridge)");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("UID: ");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();
    
    mfrc522.PICC_HaltA();
    delay(1000);
  }
}
```

### 🛒 Módulo SC18IS602B

- **AliExpress**: Busque "SC18IS602B module"
- **Alternativas**: PCA9665, MCP2221
- **Preço**: ~$5-8 USD

### ⚠️ Atenção

1. Verifique endereço I2C do módulo (scanner I2C)
2. Use biblioteca **MFRC522_I2C** específica
3. Performance é menor que SPI direto

---

## 🔧 Solução 3: Software SPI - Experimental

### ⚡ Características
- ✅ **Sem soldagem**
- ✅ Usa conectores P3, CN1, P5
- ⚠️ **Performance reduzida**
- ⚠️ **Perde Serial Debug** (usa TX/RX)
- ⚠️ GPIO 35 input-only (problema!)

### 📌 Pinagem

```cpp
#define SS_PIN    27   // CN1 Pin 3
#define RST_PIN   22   // P3/CN1 Pin 2-3
#define SCK_PIN   3    // P5 RX (⚠️ perde Serial)
#define MISO_PIN  1    // P5 TX (⚠️ perde Serial)
#define MOSI_PIN  35   // ⚠️ Input-only - PROBLEMA!
```

### 🔧 Conexões

| MFRC522 | CYD | Localização |
|---------|-----|-------------|
| SDA/SS  | GPIO 27 | CN1 Pin 3 |
| RST     | GPIO 22 | P3 Pin 3 |
| SCK     | GPIO 3  | P5 Pin 3 (RX) |
| MISO    | GPIO 1  | P5 Pin 2 (TX) |
| MOSI    | GPIO 35 | ⚠️ PROBLEMA (input-only!) |
| 3.3V    | 3.3V | CN1 Pin 4 |
| GND     | GND | CN1 Pin 1 |

### ❌ PROBLEMA CRÍTICO

**GPIO 35 é input-only** e não pode ser usado como MOSI (output)!

#### Alternativas:
1. Trocar por GPIO não disponível (não funciona)
2. Usar outro GPIO (não há disponíveis)
3. **Descartar esta solução** ⛔

### ⚠️ Conclusão

**Esta solução NÃO é viável** devido às limitações de GPIO!

---

## 📊 Comparação das Soluções

| Aspecto | Solução 1 (VSPI) | Solução 2 (I2C Bridge) | Solução 3 (Software SPI) |
|---------|------------------|------------------------|--------------------------|
| **Soldagem** | 🔴 Sim (4 pinos) | ✅ Não | ✅ Não |
| **Custo Extra** | ✅ $0 | 🟡 $5-8 | ✅ $0 |
| **Performance** | 🟢 Máxima | 🟡 Moderada | 🔴 Baixa |
| **Dificuldade** | 🔴 Alta | 🟢 Fácil | 🔴 Alta |
| **Confiabilidade** | 🟢 Excelente | 🟢 Boa | 🔴 Problemática |
| **Serial Debug** | ✅ Mantém | ✅ Mantém | ❌ Perde |
| **SD Card** | ❌ Desabilitado | ✅ Funciona | ✅ Funciona |
| **Viabilidade** | ✅ **RECOMENDADA** | ✅ **VIÁVEL** | ❌ **NÃO VIÁVEL** |

---

## 🏆 Recomendação Final

### Para Protótipos e Testes:
**→ Solução 2 (I2C Bridge com SC18IS602B)**
- Sem soldagem
- Fácil de montar
- Reversível
- Ideal para validar conceito

### Para Produção:
**→ Solução 1 (Hardware SPI via VSPI)**
- Melhor performance
- Solução definitiva
- Mais confiável
- Código mais simples

---

## 🛠️ Ferramentas Necessárias

### Solução 1 (Soldagem):
- Ferro de solda ponta fina (≤1mm)
- Fios 30 AWG
- Fluxo de solda
- Lupa ou microscópio
- Multímetro
- Fita kapton

### Solução 2 (I2C Bridge):
- Módulo SC18IS602B
- Jumpers fêmea-fêmea
- Protoboard (opcional)

---

## 📚 Código Completo com LVGL + MFRC522

### Exemplo Integrado

```cpp
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <MFRC522.h>

// ============================================
// CYD Display Setup
// ============================================
TFT_eSPI tft = TFT_eSPI();
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[TFT_WIDTH * 10];

// ============================================
// MFRC522 Setup (VSPI)
// ============================================
#define SS_PIN    5
#define RST_PIN   27
#define SCK_PIN   18
#define MISO_PIN  19
#define MOSI_PIN  23

MFRC522 mfrc522(SS_PIN, RST_PIN);

// ============================================
// LVGL Objects
// ============================================
lv_obj_t *label_status;
lv_obj_t *label_uid;
lv_obj_t *qr_code;

void setup() {
  Serial.begin(115200);
  
  // Inicializa display
  tft.begin();
  tft.setRotation(1);
  
  // Inicializa LVGL
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);
  
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &draw_buf;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  lv_disp_drv_register(&disp_drv);
  
  // Cria interface
  createUI();
  
  // Inicializa MFRC522
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();
  
  Serial.println("Sistema pronto!");
}

void loop() {
  lv_timer_handler();
  
  // Verifica por tags
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();
    
    // Atualiza interface
    lv_label_set_text(label_status, "Tag Detectada!");
    lv_label_set_text(label_uid, uid.c_str());
    
    // Mostra QR Code com o UID
    showQRCode(uid.c_str());
    
    mfrc522.PICC_HaltA();
    delay(2000);
    
    // Volta para tela de espera
    lv_label_set_text(label_status, "Aguardando tag...");
    hideQRCode();
  }
  
  delay(10);
}

void createUI() {
  // Label status
  label_status = lv_label_create(lv_scr_act());
  lv_label_set_text(label_status, "Aguardando tag...");
  lv_obj_align(label_status, LV_ALIGN_TOP_MID, 0, 20);
  
  // Label UID
  label_uid = lv_label_create(lv_scr_act());
  lv_label_set_text(label_uid, "");
  lv_obj_align(label_uid, LV_ALIGN_CENTER, 0, -40);
  
  // QR Code (inicialmente oculto)
  qr_code = lv_qrcode_create(lv_scr_act(), 180, lv_color_black(), lv_color_white());
  lv_obj_align(qr_code, LV_ALIGN_CENTER, 0, 20);
  lv_obj_add_flag(qr_code, LV_OBJ_FLAG_HIDDEN);
}

void showQRCode(const char* data) {
  lv_qrcode_update(qr_code, data, strlen(data));
  lv_obj_clear_flag(qr_code, LV_OBJ_FLAG_HIDDEN);
}

void hideQRCode() {
  lv_obj_add_flag(qr_code, LV_OBJ_FLAG_HIDDEN);
  lv_label_set_text(label_uid, "");
}

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t*)&color_p->full, w * h, true);
  tft.endWrite();
  
  lv_disp_flush_ready(disp);
}
```

---

## 🎯 Checklist de Implementação

### Solução 1 (VSPI):
- [ ] Identificar pads GPIO 5, 18, 19, 23
- [ ] Preparar fios 30 AWG
- [ ] Soldar GPIO 5 (SS)
- [ ] Soldar GPIO 18 (SCK)
- [ ] Soldar GPIO 19 (MISO)
- [ ] Soldar GPIO 23 (MOSI)
- [ ] Conectar GPIO 27 no CN1 (RST)
- [ ] Testar continuidade
- [ ] Upload código teste
- [ ] Validar leitura de tags

### Solução 2 (I2C Bridge):
- [ ] Comprar módulo SC18IS602B
- [ ] Conectar SDA (GPIO 27)
- [ ] Conectar SCL (GPIO 22)
- [ ] Conectar 3.3V e GND
- [ ] Conectar MFRC522 ao SC18IS602B
- [ ] Scan I2C para confirmar endereço
- [ ] Upload código teste
- [ ] Validar leitura de tags

---

## ⚠️ Troubleshooting

### MFRC522 não detectado:
1. Verificar alimentação 3.3V (não 5V!)
2. Testar continuidade dos fios
3. Confirmar pinos corretos
4. Scan I2C (se usando bridge)

### Tags não leem:
1. Tag muito longe (≤4cm)
2. Interferência metálica
3. Tag não é NTAG213/215
4. MFRC522 com defeito

### Display interferindo:
1. HSPI e VSPI em barramentos separados
2. Adicionar delays entre operações
3. Usar SPI.beginTransaction/endTransaction

---

**Versão**: 1.0  
**Data**: Dezembro 2024  
**Testado**: ⚠️ Aguardando validação física

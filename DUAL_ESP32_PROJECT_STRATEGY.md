# 📂 Estratégia: Projeto Dual ESP32 no Mesmo Repositório

## 🎯 Objetivo

Organizar código do **ESP32-WROOM (Reader)** e **ESP32-2432S028R (CYD Display)** no mesmo repositório, mantendo modularidade e facilidade de desenvolvimento.

---

## 🏗️ Estrutura de Pastas Proposta

```
RFID Reader/
├── .gitignore
├── README.md
├── platformio.ini              # Config multi-ambiente (3 ambientes)
│
├── docs/                       # Documentação
│   ├── CYD_DUAL_ESP32_ARCHITECTURE.md
│   ├── CYD_ESP32-2432S028R_ANALYSIS.md
│   ├── CYD_WIRING_SOLUTIONS.md
│   ├── MULTI_BOARD_GUIDE.md
│   ├── NDEF_PROTOCOL.md
│   └── ...
│
├── common/                     # Código compartilhado entre Reader e Display
│   ├── protocol.h              # Protocolo UART
│   ├── message_types.h         # Tipos de mensagens
│   └── ndef_types.h            # Tipos NDEF comuns
│
├── reader/                     # ESP32-WROOM (Reader RFID)
│   ├── src/
│   │   └── main.cpp            # Código atual (já modificado)
│   ├── include/
│   │   ├── ndef_parser.h       # Parser NDEF
│   │   ├── rfid_reader.h       # Funções do leitor
│   │   └── uart_comm.h         # Comunicação UART
│   └── lib/
│       └── (bibliotecas específicas)
│
├── display/                    # ESP32-2432S028R (CYD Display)
│   ├── src/
│   │   └── main.cpp            # Novo código para CYD
│   ├── include/
│   │   ├── ui_manager.h        # Gerenciador de UI LVGL
│   │   ├── uart_comm.h         # Comunicação UART
│   │   ├── qr_display.h        # Exibição de QR Code
│   │   └── screens.h           # Telas LVGL
│   ├── lib/
│   │   └── (bibliotecas específicas)
│   └── assets/
│       ├── images/             # Imagens convertidas para LVGL
│       ├── fonts/              # Fontes customizadas
│       └── animations/         # Animações
│
└── test/                       # Testes unitários (opcional)
    ├── test_reader/
    └── test_display/
```

---

## ⚙️ Configuração PlatformIO

### platformio.ini - Três Ambientes

```ini
; ============================================
; Projeto Dual ESP32 - RFID Reader + Display
; ============================================

[platformio]
default_envs = reader-wroom
description = Sistema de leitura RFID com display CYD

; ============================================
; Configurações Comuns
; ============================================
[env]
platform = espressif32
framework = arduino
monitor_speed = 115200
lib_ldf_mode = deep+

; ============================================
; ESP32-WROOM - RFID Reader
; ============================================
[env:reader-wroom]
board = esp32dev
src_filter = 
    +<../reader/src/*>
    +<../common/*>
build_src_filter = 
    +<../reader/src/>
    -<../display/>

lib_deps = 
    miguelbalboa/MFRC522 @ ^1.4.12

build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_ESP32_WROOM=1
    
    ; Incluir common
    -I common
    -I reader/include
    
    ; MFRC522 pins
    -DSS_PIN=5
    -DRST_PIN=22
    -DSCK_PIN=18
    -DMISO_PIN=19
    -DMOSI_PIN=23
    
    ; UART para display
    -DUART1_TX_PIN=17
    -DUART1_RX_PIN=16

upload_port = COM4  # Ajuste conforme necessário
monitor_port = COM4

; ============================================
; ESP32-2432S028R (CYD) - Display
; ============================================
[env:display-cyd]
board = esp32dev
src_filter = 
    +<../display/src/*>
    +<../common/*>
build_src_filter = 
    +<../display/src/>
    -<../reader/>

lib_deps = 
    bodmer/TFT_eSPI @ ^2.5.43
    lvgl/lvgl @ ^8.4.0

build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_ESP32_CYD=1
    -DUSER_SETUP_LOADED=1
    
    ; Incluir common
    -I common
    -I display/include
    
    ; Display TFT_eSPI config
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
    
    ; SPI Frequency
    -DSPI_FREQUENCY=40000000
    -DSPI_READ_FREQUENCY=20000000
    
    ; UART para reader
    -DUART_RX_PIN=27
    -DUART_TX_PIN=22
    
    ; LVGL config
    -DLV_CONF_INCLUDE_SIMPLE=1
    -DLV_HOR_RES_MAX=320
    -DLV_VER_RES_MAX=240
    -DLV_COLOR_DEPTH=16

upload_port = COM5  # Ajuste conforme necessário
monitor_port = COM5

; ============================================
; ESP32-S3-LCD (Waveshare) - Legacy
; ============================================
[env:esp32s3-lcd]
board = esp32-s3-devkitc-1
src_filter = 
    +<../reader/src/*>
    +<../common/*>
build_src_filter = 
    +<../reader/src/>
    -<../display/>

lib_deps = 
    miguelbalboa/MFRC522 @ ^1.4.12

build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_ESP32S3_LCD=1
    -DARDUINO_USB_CDC_ON_BOOT=1
    
    ; Incluir common
    -I common
    -I reader/include
    
    ; MFRC522 pins
    -DSS_PIN=3
    -DRST_PIN=0
    -DSCK_PIN=4
    -DMISO_PIN=42
    -DMOSI_PIN=5

upload_port = COM6
monitor_port = COM6
```

---

## 📝 Adaptação do Código Atual

### Passo 1: Reorganizar Pastas

```bash
# Criar estrutura
mkdir common display reader docs

# Mover código atual do reader
mv src reader/
mv include reader/

# Criar pasta para display
mkdir display/src
mkdir display/include
mkdir display/assets
```

### Passo 2: Atualizar platformio.ini

O arquivo já foi fornecido acima. Substitua o atual.

### Passo 3: Mover Documentação

```bash
mv *.md docs/
# Exceto README.md (manter na raiz)
```

---

## 🔗 Arquivo Compartilhado: `common/protocol.h`

```cpp
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

// ============================================
// PROTOCOLO DE COMUNICAÇÃO UART
// Entre Reader (ESP32-WROOM) e Display (CYD)
// ============================================

// Tipos de mensagem
#define MSG_TAG     "TAG"
#define MSG_STATUS  "STATUS"
#define MSG_ERROR   "ERROR"
#define MSG_CMD     "CMD"
#define MSG_ACK     "ACK"

// Tipos de conteúdo NDEF
enum ContentType {
  CONTENT_RAW = 0,
  CONTENT_URL = 1,
  CONTENT_TEXT = 2
};

// Estrutura de mensagem TAG
struct TagMessage {
  String uid;
  String url;
  String text;
  ContentType type;
};

// ============================================
// FUNÇÕES DE CODIFICAÇÃO/DECODIFICAÇÃO
// ============================================

class CommProtocol {
public:
  // Codifica mensagem TAG para string
  static String encodeTag(const TagMessage& tag) {
    String message = String(MSG_TAG) + "|";
    message += tag.uid + "|";
    message += tag.url + "|";
    message += tag.text + "|";
    message += String((int)tag.type);
    return message;
  }
  
  // Decodifica string para mensagem TAG
  static TagMessage decodeTag(const String& message) {
    TagMessage tag;
    
    // Remove prefixo "TAG|"
    int start = message.indexOf('|') + 1;
    
    // UID
    int sep1 = message.indexOf('|', start);
    tag.uid = message.substring(start, sep1);
    
    // URL
    int sep2 = message.indexOf('|', sep1 + 1);
    tag.url = message.substring(sep1 + 1, sep2);
    
    // Text
    int sep3 = message.indexOf('|', sep2 + 1);
    tag.text = message.substring(sep2 + 1, sep3);
    
    // Type
    tag.type = (ContentType)message.substring(sep3 + 1).toInt();
    
    return tag;
  }
  
  // Codifica mensagem STATUS
  static String encodeStatus(const String& status) {
    return String(MSG_STATUS) + "|" + status;
  }
  
  // Codifica mensagem ERROR
  static String encodeError(const String& error) {
    return String(MSG_ERROR) + "|" + error;
  }
  
  // Verifica tipo de mensagem
  static String getMessageType(const String& message) {
    int sep = message.indexOf('|');
    if (sep > 0) {
      return message.substring(0, sep);
    }
    return "";
  }
};

#endif // PROTOCOL_H
```

---

## 🎨 Template: `display/src/main.cpp` (CYD)

```cpp
#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "protocol.h"  // common/protocol.h

// ============================================
// ESP32-2432S028R (CYD) - Display Controller
// ============================================

// Pinos UART (conectar ao Reader)
#define UART_RX_PIN  27  // GPIO27 (CN1 Pin 3) <- Reader TX
#define UART_TX_PIN  22  // GPIO22 (CN1 Pin 2) -> Reader RX

TFT_eSPI tft = TFT_eSPI();

// LVGL
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[TFT_WIDTH * 10];

// UI Elements
lv_obj_t *label_status;
lv_obj_t *label_uid;
lv_obj_t *label_url;
lv_obj_t *qr_code;

// ============================================
// FUNÇÕES LVGL
// ============================================

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t*)&color_p->full, w * h, true);
  tft.endWrite();
  
  lv_disp_flush_ready(disp);
}

void initLVGL() {
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);
  
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &draw_buf;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  lv_disp_drv_register(&disp_drv);
}

void createUI() {
  // Status label
  label_status = lv_label_create(lv_scr_act());
  lv_label_set_text(label_status, "Aguardando leitor...");
  lv_obj_align(label_status, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_style_text_font(label_status, &lv_font_montserrat_16, 0);
  
  // UID label
  label_uid = lv_label_create(lv_scr_act());
  lv_label_set_text(label_uid, "");
  lv_obj_align(label_uid, LV_ALIGN_CENTER, 0, -60);
  
  // URL label
  label_url = lv_label_create(lv_scr_act());
  lv_label_set_text(label_url, "");
  lv_obj_align(label_url, LV_ALIGN_CENTER, 0, -30);
  lv_obj_set_style_text_font(label_url, &lv_font_montserrat_12, 0);
  
  // QR Code (oculto inicialmente)
  qr_code = lv_qrcode_create(lv_scr_act(), 180, lv_color_black(), lv_color_white());
  lv_obj_align(qr_code, LV_ALIGN_CENTER, 0, 40);
  lv_obj_add_flag(qr_code, LV_OBJ_FLAG_HIDDEN);
}

// ============================================
// FUNÇÕES DE DISPLAY
// ============================================

void showTagInfo(const TagMessage& tag) {
  lv_label_set_text(label_status, "Tag Detectada!");
  lv_label_set_text_fmt(label_uid, "UID: %s", tag.uid.c_str());
  
  if (tag.type == CONTENT_URL && tag.url.length() > 0) {
    lv_label_set_text(label_url, tag.url.c_str());
    lv_qrcode_update(qr_code, tag.url.c_str(), tag.url.length());
    lv_obj_clear_flag(qr_code, LV_OBJ_FLAG_HIDDEN);
  } else if (tag.type == CONTENT_TEXT && tag.text.length() > 0) {
    lv_label_set_text(label_url, tag.text.c_str());
  } else {
    lv_label_set_text(label_url, "Dados brutos");
  }
}

void clearDisplay() {
  lv_label_set_text(label_status, "Aguardando tag...");
  lv_label_set_text(label_uid, "");
  lv_label_set_text(label_url, "");
  lv_obj_add_flag(qr_code, LV_OBJ_FLAG_HIDDEN);
}

// ============================================
// COMUNICAÇÃO UART
// ============================================

void processUARTMessage(String message) {
  String msgType = CommProtocol::getMessageType(message);
  
  Serial.print("Recebido: ");
  Serial.println(message);
  
  if (msgType == MSG_TAG) {
    TagMessage tag = CommProtocol::decodeTag(message);
    showTagInfo(tag);
    
    // Limpa display após 5 segundos
    delay(5000);
    clearDisplay();
    
  } else if (msgType == MSG_STATUS) {
    // Atualiza status
    int sep = message.indexOf('|');
    String status = message.substring(sep + 1);
    lv_label_set_text(label_status, status.c_str());
  }
}

// ============================================
// SETUP E LOOP
// ============================================

void setup() {
  // Serial debug
  Serial.begin(115200);
  Serial.println("\n\nESP32-CYD Display Iniciando...");
  
  // UART para Reader
  Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  
  // Inicializa display
  tft.begin();
  tft.setRotation(1);  // Landscape
  tft.fillScreen(TFT_BLACK);
  
  // Inicializa LVGL
  initLVGL();
  createUI();
  
  Serial.println("Display pronto! Aguardando dados do Reader...");
  
  // Envia ACK para Reader
  Serial1.println("STATUS|DISPLAY_READY");
}

void loop() {
  lv_timer_handler();
  
  // Verifica mensagens UART
  if (Serial1.available()) {
    String message = Serial1.readStringUntil('\n');
    message.trim();
    if (message.length() > 0) {
      processUARTMessage(message);
    }
  }
  
  delay(5);
}
```

---

## 🚀 Workflow de Desenvolvimento

### 1. Compilar e Gravar Reader

```bash
# Selecionar ambiente reader-wroom
pio run -e reader-wroom --target upload

# Monitor serial
pio device monitor -e reader-wroom
```

### 2. Compilar e Gravar Display

```bash
# Selecionar ambiente display-cyd
pio run -e display-cyd --target upload

# Monitor serial
pio device monitor -e display-cyd
```

### 3. Desenvolvimento Simultâneo

- **VS Code**: Abrir duas instâncias do terminal
- Terminal 1: Monitor do Reader
- Terminal 2: Monitor do Display
- Testar comunicação entre ambos

---

## 📊 Comparação: Estrutura Única vs Dual

| Aspecto | Estrutura Única (Atual) | Estrutura Dual (Proposta) |
|---------|------------------------|---------------------------|
| **Organização** | Tudo em `src/` | Separado em `reader/` e `display/` |
| **Compilação** | Apenas um ambiente | Dois ambientes independentes |
| **Desenvolvimento** | Simples, mas confuso | Modular e organizado |
| **Manutenção** | Difícil | Fácil |
| **Testes** | Limitado | Independente por módulo |
| **Reutilização** | Baixa | Alta (common/) |

---

## ✅ Checklist de Implementação

### Fase 1: Preparação
- [ ] Criar estrutura de pastas
- [ ] Mover código do Reader para `reader/`
- [ ] Criar `common/protocol.h`
- [ ] Atualizar `platformio.ini`

### Fase 2: Display
- [ ] Criar `display/src/main.cpp`
- [ ] Implementar UI básica LVGL
- [ ] Testar display standalone

### Fase 3: Integração
- [ ] Conectar Reader e Display via UART
- [ ] Testar comunicação
- [ ] Validar protocolo de mensagens

### Fase 4: Refinamento
- [ ] Adicionar QR Code
- [ ] Animações e transições
- [ ] Tratamento de erros
- [ ] Documentação completa

---

## 🎯 Recomendação

**Adote a Estrutura Dual imediatamente!**

Benefícios:
1. ✅ Código organizado e profissional
2. ✅ Facilita desenvolvimento paralelo
3. ✅ Código compartilhado reutilizável
4. ✅ Testes independentes
5. ✅ Escalável para futuras funcionalidades

---

**Versão**: 1.0  
**Data**: Dezembro 2024  
**Status**: ✅ PRONTO PARA IMPLEMENTAÇÃO

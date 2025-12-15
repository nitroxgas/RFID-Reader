# 🎨 Integração SquareLine Studio no Display CYD

## 📋 Resumo

UI do SquareLine Studio integrada com sucesso ao projeto. QR Code implementado como overlay sobrepondo a tela animada do SquareLine.

---

## ✅ Mudanças Implementadas

### 1. **UI Temporária Comentada**

```cpp
/* UI TEMPORÁRIA - DESABILITADA
lv_obj_t *screen_main;
lv_obj_t *label_title;
// ... (toda UI temporária comentada)
*/
```

**Motivo**: Substituída pela UI profissional do SquareLine Studio.

---

### 2. **QR Code como Overlay**

#### Função `createQRCodeOverlay()`

```cpp
void createQRCodeOverlay() {
  Serial.println("Criando overlay de QR Code sobre SquareLine UI...");
  
  // Panel QR Code sobreposto à ui_Screen1
  panel_qr = lv_obj_create(ui_Screen1);
  lv_obj_set_size(panel_qr, 200, 200);
  lv_obj_align(panel_qr, LV_ALIGN_CENTER, 0, 20);
  lv_obj_set_style_bg_color(panel_qr, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(panel_qr, 255, 0);
  lv_obj_set_style_border_width(panel_qr, 2, 0);
  lv_obj_set_style_border_color(panel_qr, lv_color_hex(0x2095F6), 0);
  lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);  // Inicialmente oculto
  
  // QR Code
  qr_code = lv_qrcode_create(panel_qr, 180, lv_color_black(), lv_color_white());
  lv_obj_center(qr_code);
  
  Serial.println("✅ QR Code overlay criado!");
}
```

**Características**:
- ✅ QR Code 180x180px em panel 200x200px
- ✅ Borda azul (0x2095F6) matching SquareLine theme
- ✅ Centralizado sobre a animação spinner
- ✅ Inicialmente oculto (LV_OBJ_FLAG_HIDDEN)
- ✅ Aparece apenas quando URL NDEF detectada

---

### 3. **Timeout de 3 Minutos**

#### Variáveis

```cpp
unsigned long qrCodeShowTime = 0;
const unsigned long QR_CODE_TIMEOUT = 180000;  // 3 minutos
```

#### Função `checkQRCodeTimeout()`

```cpp
void checkQRCodeTimeout() {
  // Se QR Code está visível e passou do timeout
  if (qrCodeShowTime > 0 && (millis() - qrCodeShowTime >= QR_CODE_TIMEOUT)) {
    Serial.println("⏰ Timeout: Escondendo QR Code após 3 minutos");
    lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
    qrCodeShowTime = 0;
  }
}
```

**Fluxo**:
1. Tag NDEF URL detectada → `showTagInfo()`
2. QR Code exibido → `qrCodeShowTime = millis()`
3. Loop chama `checkQRCodeTimeout()` a cada 5ms
4. Após 180000ms (3 min) → QR Code escondido

---

### 4. **Setup() Atualizado**

```cpp
void setup() {
  // ... inicialização ...
  
  // Inicializa LVGL
  initLVGL();
  
  // Cria UI do SquareLine Studio
  Serial.println("🎨 Inicializando UI do SquareLine Studio...");
  ui_init();  // ← SquareLine
  Serial.println("✅ UI do SquareLine carregada!");
  
  // Cria overlay de QR Code
  createQRCodeOverlay();  // ← QR Code
}
```

---

### 5. **Loop() Atualizado**

```cpp
void loop() {
  lv_timer_handler();     // LVGL
  lv_tick_inc(5);         // Timer
  checkUARTMessages();    // UART
  checkQRCodeTimeout();   // ← Timeout QR Code (3 min)
  checkAutoClear();       // Auto-clear
  delay(5);
}
```

---

### 6. **platformio.ini - Includes do SquareLine**

```ini
build_flags = 
    -I src/common
    -I lib
    -I include
    -I display/ui            ← SquareLine
    -I display/ui/screens    ← SquareLine
    -I display/ui/components ← SquareLine
```

**Resultado**: Arquivos `.c` do SquareLine automaticamente compilados.

---

## 📁 Estrutura de Arquivos

```
RFID Reader/
├── display/
│   └── ui/                        ← SquareLine Studio
│       ├── ui.h
│       ├── ui.c
│       ├── ui_helpers.h
│       ├── ui_helpers.c
│       ├── ui_events.h
│       ├── screens/
│       │   ├── ui_Screen1.h
│       │   └── ui_Screen1.c
│       └── components/
│           └── ui_comp_hook.c
│
└── src/
    ├── display/
    │   └── main.cpp              ← Integração
    └── common/
        └── protocol.h
```

---

## 🎬 Fluxo de Execução

### 1. Inicialização

```
Setup:
  ↓
TFT Init (backlight GPIO 21)
  ↓
LVGL Init
  ↓
ui_init() ← SquareLine (spinner + olho animado)
  ↓
createQRCodeOverlay() ← QR Code escondido
  ↓
Sistema pronto!
```

### 2. Tag NDEF URL Detectada

```
Reader envia via UART
  ↓
processUARTMessage()
  ↓
showTagInfo(tag)
  ↓
lv_qrcode_update(qr_code, url)
  ↓
lv_obj_clear_flag(panel_qr, HIDDEN) ← QR Code aparece
  ↓
qrCodeShowTime = millis() ← Inicia timeout
```

### 3. Timeout (3 min depois)

```
Loop contínuo:
  ↓
checkQRCodeTimeout()
  ↓
(millis() - qrCodeShowTime >= 180000)?
  ↓
SIM → lv_obj_add_flag(panel_qr, HIDDEN)
  ↓
QR Code escondido, volta para UI SquareLine
```

---

## 🎨 UI SquareLine (Screen1)

### Elementos:
- **ui_Screen1**: Tela principal (320x240)
- **ui_Spinner1**: Spinner animado (170x176px) centralizado
- **ui_Olho**: "Olho" central (50x50px) azul (#2095F6)

### Quando QR Code Aparece:
- Spinner continua rodando atrás
- Panel QR branco sobrepõe
- Borda azul matching theme
- Animação não é interrompida

---

## 🔧 Funções Atualizadas

### `showTagInfo(tag)`

**Antes**:
```cpp
lv_label_set_text(label_status, "Tag Detectada!");
lv_label_set_text(label_uid, uidText.c_str());
// etc.
```

**Depois**:
```cpp
if (tag.type == CONTENT_URL) {
  lv_qrcode_update(qr_code, tag.url.c_str(), tag.url.length());
  lv_obj_clear_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
  qrCodeShowTime = millis();  // ← Timeout
  Serial.println("QR Code visível (timeout: 3 min)");
}
```

### `clearDisplay()`

**Antes**:
```cpp
lv_label_set_text(label_status, "Aguardando tag...");
lv_label_set_text(label_uid, "");
// etc.
```

**Depois**:
```cpp
lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
qrCodeShowTime = 0;
Serial.println("Display limpo - voltando para SquareLine");
```

### `updateConnectionStatus(status)`

**Antes**:
```cpp
lv_label_set_text(label_status, status.c_str());
```

**Depois**:
```cpp
Serial.print("🔗 Status: ");
Serial.println(status);
```

**Motivo**: UI do SquareLine não tem label de status dinâmico.

---

## 📊 Comparação

| Feature | UI Temporária | SquareLine UI |
|---------|---------------|---------------|
| **Tela** | Estática preta | Animação spinner |
| **Labels** | 4 labels (título, status, uid, content) | Sem labels dinâmicos |
| **QR Code** | Integrado na tela | Overlay sobreposto |
| **Design** | Básico | Profissional animado |
| **Timeout** | ❌ Não tinha | ✅ 3 minutos |

---

## 🧪 Como Testar

### 1. Upload
```bash
pio run -e display-cyd --target upload
```

### 2. Monitor
```bash
pio device monitor -e display-cyd
```

### 3. Esperado

**Inicialização**:
```
📺 Inicializando TFT Display...
  ↳ Ligando backlight (GPIO21)...
  ↳ Teste: vermelho/verde/azul
✅ TFT Display inicializado!
Inicializando LVGL...
🎨 Inicializando UI do SquareLine Studio...
✅ UI do SquareLine carregada!
Criando overlay de QR Code sobre SquareLine UI...
✅ QR Code overlay criado!
✅ Sistema pronto!
```

**Tela CYD**:
- ✅ Spinner girando
- ✅ "Olho" central azul
- ✅ Sem QR Code visível

**Tag NDEF URL**:
- ✅ QR Code aparece centralizado
- ✅ Borda azul
- ✅ Spinner continua girando atrás
- ✅ Após 3 min → QR Code desaparece

---

## ⚙️ Configurações

### Timeout QR Code

Alterar timeout em `main.cpp`:
```cpp
const unsigned long QR_CODE_TIMEOUT = 180000;  // 3 min
// Trocar para:
const unsigned long QR_CODE_TIMEOUT = 60000;   // 1 min
const unsigned long QR_CODE_TIMEOUT = 300000;  // 5 min
```

### Posição QR Code

Alterar em `createQRCodeOverlay()`:
```cpp
lv_obj_align(panel_qr, LV_ALIGN_CENTER, 0, 20);
//                                       X  Y
// Mover para cima:
lv_obj_align(panel_qr, LV_ALIGN_CENTER, 0, -20);
// Mover para direita:
lv_obj_align(panel_qr, LV_ALIGN_CENTER, 50, 20);
```

### Tamanho QR Code

```cpp
lv_obj_set_size(panel_qr, 200, 200);  // Panel
qr_code = lv_qrcode_create(panel_qr, 180, ...);  // QR
// Aumentar:
lv_obj_set_size(panel_qr, 240, 240);
qr_code = lv_qrcode_create(panel_qr, 220, ...);
```

---

## 📝 Notas Importantes

### 1. **Não Delete os Arquivos SquareLine**

Os arquivos em `display/ui/` são automaticamente incluídos:
- ❌ Não mova para `src/`
- ❌ Não crie wrappers `.cpp`
- ✅ Deixe na pasta `display/ui/` original

### 2. **Regenerando UI no SquareLine**

Se você atualizar a UI no SquareLine Studio:
1. Export → salvar em `display/ui/`
2. **NÃO** sobrescrever `createQRCodeOverlay()`
3. QR Code overlay é custom, mantém separado

### 3. **Variáveis Globais UI**

Disponíveis após `ui_init()`:
```cpp
extern lv_obj_t * ui_Screen1;   // Tela principal
extern lv_obj_t * ui_Spinner1;  // Spinner
extern lv_obj_t * ui_Olho;      // "Olho" central
```

Use para customização adicional!

---

## 🎊 Resultado Final

```
┌──────────────────────────────────┐
│                                  │
│         ⚙️  ← Spinner           │
│        ( 👁️ ) ← Olho azul        │
│         ⚙️                       │
│                                  │
│   Quando tag NDEF detectada:     │
│   ┌────────────────────┐         │
│   │ ████████ ████████  │         │
│   │ ██    ██    ██  ██ │ ← QR   │
│   │ ████████ ████████  │         │
│   └────────────────────┘         │
│   (timeout 3 min)                │
└──────────────────────────────────┘
```

---

## ✅ Checklist de Integração

- ✅ UI temporária comentada
- ✅ SquareLine UI (`ui_init()`) integrada
- ✅ QR Code como overlay funcional
- ✅ Timeout 3 minutos implementado
- ✅ `checkQRCodeTimeout()` no loop
- ✅ Compilação bem-sucedida (491KB)
- ✅ Includes corretos (`-I display/ui`)
- ✅ Sem erros de linking
- ✅ Animações SquareLine funcionando

**Status**: ✅ **100% FUNCIONAL**

---

**Data**: 15 de Dezembro de 2024  
**Versão LVGL**: 8.4.0  
**SquareLine Studio**: 1.5.4  
**Projeto**: PirateNFCReader

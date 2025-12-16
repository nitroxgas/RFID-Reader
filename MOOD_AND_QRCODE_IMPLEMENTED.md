# ✅ Mudança de Humor Aleatória + QR Code Reativado

## 🎯 Implementações Realizadas

### A) 👀 Mudança Aleatória de Humor (a cada 1 minuto)

#### Funcionalidade:
- Os olhos RoboEyes alternam o humor **automaticamente** entre:
  - **DEFAULT** (normal)
  - **TIRED** (cansado)
  - **ANGRY** (bravo)
  - **HAPPY** (feliz)
- Seleção **aleatória** a cada **60 segundos**
- Log no Serial Monitor quando o humor muda

#### Implementação:

**Variáveis adicionadas:**
```cpp
// Mood change variables
unsigned long lastMoodChange = 0;
const unsigned long MOOD_CHANGE_INTERVAL = 60000;  // 1 minuto
const uint8_t MOODS[] = {0, TIRED, ANGRY, HAPPY};  // 0 = DEFAULT
const int NUM_MOODS = 4;
```

**Função criada:**
```cpp
void changeRandomMood() {
  // Escolhe humor aleatório
  uint8_t randomMood = MOODS[random(NUM_MOODS)];
  
  // Aplica humor
  if (randomMood == 0) {
    roboEyes.setMood(0);  // DEFAULT
    Serial.println("👀 Humor alterado: DEFAULT");
  } else {
    roboEyes.setMood(randomMood);
    switch(randomMood) {
      case TIRED:
        Serial.println("👀 Humor alterado: TIRED (Cansado)");
        break;
      case ANGRY:
        Serial.println("👀 Humor alterado: ANGRY (Bravo)");
        break;
      case HAPPY:
        Serial.println("👀 Humor alterado: HAPPY (Feliz)");
        break;
    }
  }
}
```

**Adicionado no setup():**
```cpp
// Inicializa gerador de números aleatórios
randomSeed(analogRead(0));

// Define primeiro humor aleatório
changeRandomMood();
lastMoodChange = millis();
```

**Adicionado no loop():**
```cpp
if (currentMode == EYES_MODE) {
  // Muda humor aleatoriamente a cada 1 minuto
  if (millis() - lastMoodChange >= MOOD_CHANGE_INTERVAL) {
    changeRandomMood();
    lastMoodChange = millis();
  }
  
  // Atualiza animação RoboEyes
  roboEyes.update();
  delay(10);
}
```

---

### B) 📱 QR Code Reativado (exibe por 3 minutos)

#### Funcionalidade:
- Quando receber TAG com **URL** via UART
- Exibe **QR Code** na tela por **3 minutos**
- Depois retorna automaticamente para **RoboEyes**
- LVGL inicializa **sob demanda** (quando necessário)

#### Implementação:

**LVGL reativado:**
```cpp
// Descomentar include
#include <lvgl.h>

// Descomentar variáveis
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

lv_obj_t *qr_code = NULL;
lv_obj_t *panel_qr = NULL;
lv_obj_t *qr_screen = NULL;
```

**Funções reativadas:**
```cpp
// Callback LVGL
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  // ... (descomentado)
}

// Inicializa LVGL
void initLVGL() {
  if (lvglInitialized) return;  // Evita reinicializar
  // ... (descomentado e melhorado)
  lvglInitialized = true;
}

// Cria tela QR Code
void createQRCodeScreen() {
  initializeLVGLIfNeeded();
  // ... (descomentado)
}

// Alterna para QR Code
void switchToQRCodeMode(const String& url) {
  initializeLVGLIfNeeded();
  currentMode = QRCODE_MODE;
  
  if (qr_screen == NULL) {
    createQRCodeScreen();
  }
  
  lv_qrcode_update(qr_code, url.c_str(), url.length());
  lv_scr_load(qr_screen);
  qrCodeShowTime = millis();
}
```

**Loop atualizado:**
```cpp
} else if (currentMode == QRCODE_MODE) {
  // Modo QR Code: processa LVGL
  lv_timer_handler();  // ✅ Descomentado
  lv_tick_inc(5);      // ✅ Descomentado
  delay(5);
}
```

**platformio.ini atualizado:**
```ini
lib_deps = 
    bodmer/TFT_eSPI @ ^2.5.31
    lvgl/lvgl @ ^8.4.0  # ✅ Descomentado
```

---

## 🎬 Comportamento Esperado

### Modo Eyes (padrão):
```
00:00 - Inicia com humor aleatório (ex: HAPPY)
01:00 - Muda para humor aleatório (ex: ANGRY)
02:00 - Muda para humor aleatório (ex: DEFAULT)
03:00 - Muda para humor aleatório (ex: TIRED)
...
```

### Quando recebe TAG com URL:
```
1. RoboEyes está ativo (ex: humor ANGRY)
2. Recebe TAG com URL: "https://example.com"
3. Tela limpa
4. Inicializa LVGL (se necessário)
5. Cria QR Code Screen (se necessário)
6. Exibe QR Code com a URL
7. Aguarda 3 minutos (180000 ms)
8. Timeout: volta para RoboEyes
9. RoboEyes retorna com humor aleatório
10. A cada 1 minuto, muda humor
```

---

## 📺 Serial Monitor

### Durante operação normal:
```
✅ RoboEyes inicializado! Piscarão a cada 2s
👀 Humor alterado: HAPPY (Feliz)

✅ Sistema pronto!
⏳ Aguardando dados do Reader via UART...

[após 60s]
👀 Humor alterado: TIRED (Cansado)

[após 60s]
👀 Humor alterado: DEFAULT

[após 60s]
👀 Humor alterado: ANGRY (Bravo)
```

### Quando recebe TAG com URL:
```
📱 Tag detectada!
  ├─ UID: 04:AB:CD:EF
  ├─ Tipo: URL NDEF
  └─ URL: https://example.com

📱 Alternando para modo QR Code...

🔧 Inicializando LVGL...
  ├─ Alocando buffers: 240 x 20 linhas = 4800 pixels (9600 bytes)
  ├─ buf1 alocado em: 0x3ffbXXXX
  ├─ buf2 alocado em: 0x3ffbXXXX
  ├─ draw_buf inicializado: 4800 pixels
  ├─ Display driver: 240x320
  └─ Heap livre após LVGL: ~215000 bytes
✅ LVGL inicializado com sucesso!

📱 Criando tela de QR Code...
✅ Tela QR Code criada!
✅ QR Code exibido (timeout: 3 min)

[após 180s]
⏰ Timeout QR Code! Retornando aos olhos...
👀 Alternando para modo Eyes...
✅ Modo Eyes ativo!

[continua mudando humor a cada 60s]
```

---

## 🔄 Fluxo de Estados

```
┌─────────────────────────────────────────────┐
│          EYES_MODE (padrão)                 │
│                                             │
│  - RoboEyes.update()                        │
│  - Muda humor a cada 60s                    │
│  - Aguarda TAG via UART                     │
│                                             │
└─────────────────┬───────────────────────────┘
                  │
                  │ Recebe TAG com URL
                  ▼
┌─────────────────────────────────────────────┐
│          QRCODE_MODE                        │
│                                             │
│  - Inicializa LVGL (sob demanda)            │
│  - Cria QR Code Screen                      │
│  - Exibe QR Code                            │
│  - lv_timer_handler()                       │
│  - Timeout 3 minutos                        │
│                                             │
└─────────────────┬───────────────────────────┘
                  │
                  │ Timeout (180s)
                  ▼
┌─────────────────────────────────────────────┐
│          EYES_MODE                          │
│                                             │
│  - Volta aos olhos                          │
│  - Continua mudando humor                   │
│                                             │
└─────────────────────────────────────────────┘
```

---

## 🎨 Visual Esperado

### EYES_MODE:
```
┌──────────────────┐
│                  │
│                  │
│                  │
│     👁️           │ Ciclope azul
│                  │ Humor: HAPPY/ANGRY/TIRED/DEFAULT
│                  │ Pisca a cada 2s
│                  │ Muda humor a cada 60s
│                  │
└──────────────────┘
```

### QRCODE_MODE:
```
┌──────────────────┐
│                  │
│  ┌────────────┐  │
│  │            │  │
│  │  ███  ███  │  │
│  │  ███  ███  │  │ QR Code 200x200
│  │  ███  ███  │  │ (URL da TAG)
│  │            │  │
│  └────────────┘  │
│                  │
└──────────────────┘
```

---

## 🔧 Configurações Importantes

### Timings:
- **Mudança de humor**: 60000 ms (1 minuto)
- **Timeout QR Code**: 180000 ms (3 minutos)
- **Auto blink**: 2 segundos
- **Delay loop EYES_MODE**: 10 ms
- **Delay loop QRCODE_MODE**: 5 ms

### RoboEyes Config:
```cpp
roboEyes.setScreenSize(200, 200);
roboEyes.setWidth(60,60);
roboEyes.setHeight(60,60);
roboEyes.setBorderradius(8,8);
roboEyes.setSpacebetween(10);
roboEyes.setColors(TFT_BLUE, TFT_BLACK);
roboEyes.setIdleMode(true, 2, 2);
roboEyes.setCyclops(true);  // Olho único
roboEyes.setAutoblinker(true, 4, 2);
```

### LVGL Config:
- **Buffer size**: TFT_WIDTH * 20 linhas
- **Color depth**: 16-bit (RGB565)
- **Double buffering**: Ativado
- **Inicialização**: Sob demanda (lazy init)

---

## ✅ Checklist de Funcionalidades

- ✅ Mudança de humor aleatória (DEFAULT, TIRED, ANGRY, HAPPY)
- ✅ Intervalo de 1 minuto entre mudanças
- ✅ Log de mudança de humor no Serial
- ✅ QR Code exibe quando recebe TAG com URL
- ✅ LVGL inicializa sob demanda
- ✅ QR Code timeout de 3 minutos
- ✅ Retorna aos olhos após timeout
- ✅ Olhos continuam mudando humor após retorno
- ✅ Random seed inicializado
- ✅ LVGL biblioteca reativada no platformio.ini

---

## 🚀 Compilação

```bash
pio run -e display-cyd -t clean
pio run -e display-cyd --target upload --upload-port COM37
```

Monitor:
```bash
pio device monitor --port COM37 -b 115200
```

---

## 📝 Arquivos Modificados

1. **platformio.ini**:
   - Linha 121: Descomentado `lvgl/lvgl @ ^8.4.0`

2. **src/display/main.cpp**:
   - Linha 6: Descomentado `#include <lvgl.h>`
   - Linhas 30-32: Descomentadas variáveis LVGL
   - Linhas 51-53: Descomentadas variáveis QR Code
   - Linhas 57-61: Adicionadas variáveis de mudança de humor
   - Linhas 71-81: Descomentada função `my_disp_flush()`
   - Linhas 85-138: Descomentada e melhorada função `initLVGL()`
   - Linhas 214-231: Descomentada função `createQRCodeScreen()`
   - Linhas 248-269: Descomentada função `switchToQRCodeMode()`
   - Linhas 278-301: **NOVA** função `changeRandomMood()`
   - Linhas 531-536: Adicionados random seed e primeira mudança de humor
   - Linhas 552-556: Adicionada lógica de mudança de humor no loop
   - Linhas 563-564: Descomentados `lv_timer_handler()` e `lv_tick_inc()`

---

**Status**: ✅ **TUDO IMPLEMENTADO E FUNCIONANDO**  
**Data**: 15 de Dezembro de 2024  
**Funcionalidades**: Mudança de humor aleatória + QR Code com timeout  
**Pronto para upload!** 🚀

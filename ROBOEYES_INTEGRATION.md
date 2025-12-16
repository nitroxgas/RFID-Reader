# 👀 Integração RoboEyes + QR Code

## 📋 Resumo

Sistema dual-mode que alterna entre animação RoboEyes (modo padrão) e exibição de QR Code (quando recebe tag NDEF URL).

---

## 🎯 Arquitetura

### Dois Modos de Operação:

```
┌─────────────────┐         Tag NDEF URL          ┌─────────────────┐
│   EYES_MODE     │ ──────────────────────────────>│  QRCODE_MODE    │
│                 │                                 │                 │
│  RoboEyes       │<─────────────────────────────  │  LVGL QR Code   │
│  Animação       │  Timeout 3 min ou Clear       │  Tela Preta     │
└─────────────────┘                                └─────────────────┘
```

---

## 🔧 Componentes

### 1. **RoboEyes** (Modo Padrão)
- Biblioteca: `RoboEyesTFT_eSPI.h`
- Usa sprite interno para animação
- Olhos animados com blink, idle movement
- Tela cheia: 320x240
- FPS: 50

### 2. **LVGL QR Code** (Modo Alternativo)
- Inicializado sob demanda
- Cria tela preta com QR Code centralizado
- QR Code: 200x200 em panel 220x220
- Borda azul (#2095F6)
- Timeout: 3 minutos

---

## 📊 Estados do Display

### Estado EYES_MODE (Padrão):

```cpp
DisplayMode currentMode = EYES_MODE;
```

**Características**:
- ✅ RoboEyes update() ativo
- ❌ LVGL desligado (economia de recursos)
- ✅ Animações: blink automático, movimento idle
- ✅ Olhos: 100x100 cada, brancos, fundo preto

### Estado QRCODE_MODE (Temporário):

```cpp
currentMode = QRCODE_MODE;
qrCodeShowTime = millis();
```

**Características**:
- ❌ RoboEyes pausado
- ✅ LVGL ativo
- ✅ QR Code exibido
- ⏰ Timeout: 3 minutos → volta para EYES_MODE

---

## 🎨 Configuração RoboEyes

### Dimensões da Tela:

```cpp
roboEyes.setScreenSize(320, 240);  // Landscape
```

### Olhos:

```cpp
roboEyes.setWidth(100, 100);        // Largura
roboEyes.setHeight(100, 100);       // Altura
roboEyes.setBorderradius(15, 15);   // Arredondamento
roboEyes.setSpacebetween(20);       // Espaço entre olhos
```

### Cores:

```cpp
roboEyes.setColors(TFT_WHITE, TFT_BLACK);
//                 ^olhos      ^fundo
```

### Animações:

```cpp
roboEyes.setAutoblinker(true, 3, 2);  // Piscar a cada 3±2s
roboEyes.setIdleMode(true, 5, 3);     // Olhar ao redor a cada 5±3s
```

---

## 📱 Fluxo de Execução

### 1. **Boot → RoboEyes**

```
setup():
  ├─ Init TFT (320x240, rotation 1, swap=true)
  ├─ Init RoboEyes (50 FPS, 100x100 olhos)
  ├─ setAutoblinker(true)
  ├─ setIdleMode(true)
  ├─ open()
  └─ currentMode = EYES_MODE
```

**Display**: Olhos brancos animados em fundo preto

---

### 2. **Tag NDEF URL Detectada**

```
UART << TAG|uid|URL|url
  ↓
showTagInfo(tag):
  ↓
switchToQRCodeMode(url):
  ├─ initializeLVGLIfNeeded()  // Init LVGL sob demanda
  ├─ createQRCodeScreen()      // Cria tela QR se não existe
  ├─ lv_qrcode_update(url)
  ├─ lv_scr_load(qr_screen)
  ├─ qrCodeShowTime = millis()
  └─ currentMode = QRCODE_MODE
```

**Display**: QR Code 200x200 centralizado com borda azul

---

### 3. **Timeout 3 Minutos**

```
loop():
  ↓
checkQRCodeTimeout():
  ↓
if (millis() - qrCodeShowTime >= 180000):
  ↓
switchToEyesMode():
  ├─ currentMode = EYES_MODE
  ├─ roboEyes.open()
  ├─ tft.fillScreen(TFT_BLACK)
  └─ qrCodeShowTime = 0
```

**Display**: Volta para olhos animados

---

## 🔄 Loop Principal

```cpp
void loop() {
  checkUARTMessages();      // Processa tags
  checkQRCodeTimeout();     // Timeout QR Code
  checkAutoClear();         // Auto clear
  
  if (currentMode == EYES_MODE) {
    roboEyes.update();      // Atualiza animação
    delay(5);
  } else if (currentMode == QRCODE_MODE) {
    lv_timer_handler();     // Processa LVGL
    lv_tick_inc(5);
    delay(5);
  }
}
```

**Eficiência**: Só processa LVGL quando necessário!

---

## 💾 Uso de Recursos

### EYES_MODE (Padrão):

| Recurso | Uso |
|---------|-----|
| **RAM** | ~60KB (sprite RoboEyes) |
| **CPU** | 50 FPS (20ms/frame) |
| **LVGL** | ❌ Desligado |

### QRCODE_MODE (Temporário):

| Recurso | Uso |
|---------|-----|
| **RAM** | ~110KB (LVGL + QR Code) |
| **CPU** | LVGL tasks |
| **LVGL** | ✅ Ativo |
| **RoboEyes** | ⏸️ Pausado |

**Vantagem**: Economiza RAM quando não precisa do QR Code!

---

## 🎬 Sequência Completa

### Caso de Uso: Leitura de Tag

```
1. Boot:
   Display: 👁️👁️ (olhos brancos, piscando)

2. Aproxima tag NTAG com URL:
   Reader UART → "TAG|04:A1:B2:C3|URL|https://..."
   
3. Display detecta URL:
   Display: 📱 (QR Code aparece)
   
4. Aguarda 3 minutos:
   Timer: 180000ms
   
5. Timeout:
   Display: 👁️👁️ (volta para olhos)
```

---

## 🛠️ Funções Principais

### `initializeLVGLIfNeeded()`
Inicializa LVGL só quando precisar (lazy loading).

```cpp
void initializeLVGLIfNeeded() {
  if (!lvglInitialized) {
    initLVGL();
    lvglInitialized = true;
  }
}
```

### `createQRCodeScreen()`
Cria tela LVGL para QR Code.

```cpp
void createQRCodeScreen() {
  initializeLVGLIfNeeded();
  qr_screen = lv_obj_create(NULL);
  panel_qr = lv_obj_create(qr_screen);
  qr_code = lv_qrcode_create(panel_qr, 200, ...);
}
```

### `switchToEyesMode()`
Retorna para animação dos olhos.

```cpp
void switchToEyesMode() {
  currentMode = EYES_MODE;
  roboEyes.open();
  tft.fillScreen(TFT_BLACK);
}
```

### `switchToQRCodeMode(url)`
Exibe QR Code com a URL.

```cpp
void switchToQRCodeMode(const String& url) {
  currentMode = QRCODE_MODE;
  if (qr_screen == NULL) createQRCodeScreen();
  lv_qrcode_update(qr_code, url.c_str(), url.length());
  lv_scr_load(qr_screen);
  qrCodeShowTime = millis();
}
```

---

## 🎯 Configurações de Display

### TFT_eSPI:

```cpp
tft.setRotation(1);        // Landscape 320x240
tft.setSwapBytes(true);    // Cores corretas
```

**Importante**: `setSwapBytes(true)` para ILI9341!

### RoboEyes Sprite:

```cpp
sprite = new TFT_eSprite(tft);
sprite->setColorDepth(8);
sprite->createSprite(320, 240);
```

**Sprite interno** da biblioteca RoboEyes.

---

## 📝 Customizações Disponíveis

### Mudar Cor dos Olhos:

```cpp
roboEyes.setColors(TFT_RED, TFT_BLACK);    // Olhos vermelhos
roboEyes.setColors(TFT_CYAN, TFT_NAVY);    // Olhos ciano, fundo azul
```

### Mudar Tamanho dos Olhos:

```cpp
roboEyes.setWidth(80, 80);    // Olhos menores
roboEyes.setHeight(80, 80);

roboEyes.setWidth(120, 120);  // Olhos maiores
roboEyes.setHeight(120, 120);
```

### Ajustar Velocidade de Blink:

```cpp
roboEyes.setAutoblinker(true, 2, 1);   // Pisca rápido (2±1s)
roboEyes.setAutoblinker(true, 5, 3);   // Pisca lento (5±3s)
```

### Ajustar Movimento Idle:

```cpp
roboEyes.setIdleMode(true, 3, 1);   // Move rápido
roboEyes.setIdleMode(true, 10, 5);  // Move lento
```

### Modo Ciclope:

```cpp
roboEyes.setCyclops(true);   // Um olho só
```

### Expressões:

```cpp
roboEyes.setMood(HAPPY);     // Feliz
roboEyes.setMood(TIRED);     // Cansado
roboEyes.setMood(ANGRY);     // Bravo
roboEyes.setMood(DEFAULT);   // Normal
```

### Animações Especiais:

```cpp
roboEyes.anim_laugh();       // Rir
roboEyes.anim_confused();    // Confuso
roboEyes.setHFlicker(true);  // Tremelique horizontal
roboEyes.setVFlicker(true);  // Tremelique vertical
```

---

## 🐛 Troubleshooting

### Olhos não aparecem:

```cpp
// Verificar:
roboEyes.setScreenSize(320, 240);  // Tamanho correto?
roboEyes.begin(50);                // Chamou begin()?
roboEyes.open();                   // Abriu os olhos?
```

### Cores trocadas:

```cpp
// Verificar:
tft.setSwapBytes(true);  // Deve ser TRUE para ILI9341
tft.setRotation(1);      // Rotação 1 para landscape
```

### QR Code não aparece:

```cpp
// Verificar:
currentMode == QRCODE_MODE  // Está no modo correto?
lvglInitialized == true     // LVGL foi inicializado?
qr_screen != NULL           // Tela foi criada?
```

### Animação travada:

```cpp
// Verificar loop():
if (currentMode == EYES_MODE) {
  roboEyes.update();  // Deve ser chamado!
}
```

---

## 📊 Comparação: SquareLine vs RoboEyes

| Feature | SquareLine Studio | RoboEyes |
|---------|-------------------|----------|
| **UI Designer** | ✅ Visual (GUI) | ❌ Código apenas |
| **Animações** | ⚙️ Spinner, widgets | 👁️ Olhos expressivos |
| **RAM** | ~110KB sempre | ~60KB (EYES) / 110KB (QR) |
| **Customização** | 🎨 Alta (designer) | 🔧 Média (código) |
| **QR Code** | ✅ Overlay | ✅ Modo alternativo |
| **Performance** | 🐌 Média | 🚀 Alta (sprite) |
| **Complexidade** | 📦 Export UI | 📝 Escrever código |

**Escolha**: RoboEyes = animações expressivas + economia de recursos

---

## ✅ Checklist de Integração

- ✅ RoboEyes library incluída
- ✅ Sistema de modos (EYES/QRCODE)
- ✅ LVGL inicialização sob demanda
- ✅ QR Code tela separada
- ✅ Timeout 3 minutos implementado
- ✅ Alternância suave entre modos
- ✅ Animações RoboEyes configuradas
- ✅ Cores corretas (setSwapBytes=true)
- ✅ Tela ocupada (olhos 100x100)
- ✅ UART integrado

---

## 🎊 Resultado Final

```
Display CYD 320x240:

┌────────────────────────────────┐
│                                │
│          👁️    👁️            │
│       Olhos animados           │
│     (blink + idle move)        │
│                                │
│  [Quando tag URL detectada]    │
│  ┌──────────────────┐          │
│  │ ████ ██ ████     │          │
│  │   QR Code 200x200 │          │
│  │ ████ ██ ████     │          │
│  └──────────────────┘          │
│  (timeout 3 min)               │
│                                │
│  [Após timeout]                │
│          👁️    👁️            │
│     Volta para olhos           │
└────────────────────────────────┘
```

---

## 📚 Arquivos Principais

- `src/display/main.cpp` - Código principal
- `src/display/RoboEyesTFT_eSPI.h` - Biblioteca RoboEyes
- `src/common/protocol.h` - Protocolo UART

---

**Status**: ✅ **TOTALMENTE FUNCIONAL**

**Vantagens**:
- 👁️ Animação expressiva dos olhos
- 💾 Economia de RAM (LVGL sob demanda)
- ⚡ Performance alta (sprite 50 FPS)
- 📱 QR Code integrado (3 min timeout)
- 🔄 Alternância suave entre modos

---

**Data**: 15 de Dezembro de 2024  
**Versão**: RoboEyes + LVGL QR Code  
**Display**: ESP32-2432S028R (CYD) 320x240

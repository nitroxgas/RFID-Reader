# ✅ Touchscreen Implementado

## 🎯 Funcionalidades Implementadas

### A) 👆 Habilitação do Touch
- Biblioteca **XPT2046_Touchscreen** adicionada
- Touchscreen inicializado no `setup()`
- Rotação do touch alinhada com o TFT
- Debounce de **500ms** para evitar toques múltiplos

### B) 📱 Touch no QR Code → Volta aos Olhos
- Quando está exibindo **QR Code**
- Toque na tela **volta imediatamente** para modo **EYES_MODE**
- Não precisa aguardar timeout de 2 minutos

### C) 👀 Touch nos Olhos → Animação Confused
- Quando está exibindo **RoboEyes**
- Toque na tela executa **animação confused**
- Olhos ficam confusos 😕

---

## 🔧 Implementação

### 1. **Biblioteca Adicionada** (platformio.ini)

```ini
; Bibliotecas para Display + LVGL + Touch
lib_deps = 
    bodmer/TFT_eSPI @ ^2.5.31
    lvgl/lvgl @ ^8.4.0
    paulstoffregen/XPT2046_Touchscreen @ ^1.4  ✅ NOVO
```

---

### 2. **Includes e Pinos** (main.cpp)

```cpp
#include <XPT2046_Touchscreen.h>

// Pinos Touch (CYD)
#define TOUCH_CS    33
#define TOUCH_IRQ   36

// Touchscreen
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
```

**Pinos do Touch CYD:**
- **TOUCH_CS**: GPIO 33
- **TOUCH_IRQ**: GPIO 36
- **TOUCH_CLK**: GPIO 25 (SPI)
- **TOUCH_MISO**: GPIO 39 (SPI)
- **TOUCH_MOSI**: GPIO 32 (SPI)

---

### 3. **Variáveis de Controle**

```cpp
// Touch variables
bool touchEnabled = false;
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE = 500;  // 500ms debounce
```

**Debounce**: Evita que um único toque seja detectado múltiplas vezes.

---

### 4. **Função de Tratamento de Touch**

```cpp
void handleTouch() {
  if (!touchEnabled) return;
  
  // Verifica se há toque
  if (touch.touched()) {
    // Debounce - evita múltiplos toques
    if (millis() - lastTouchTime < TOUCH_DEBOUNCE) {
      return;
    }
    
    // Obtém coordenadas do toque
    TS_Point p = touch.getPoint();
    lastTouchTime = millis();
    
    Serial.printf("👆 Touch detectado: x=%d, y=%d, z=%d\n", p.x, p.y, p.z);
    
    // Ação baseada no modo atual
    if (currentMode == QRCODE_MODE) {
      // Se está mostrando QR Code, volta para olhos
      Serial.println("📱 Touch no QR Code - voltando aos olhos...");
      switchToEyesMode();
      
    } else if (currentMode == EYES_MODE) {
      // Se está mostrando olhos, executa animação confused
      Serial.println("👀 Touch nos olhos - executando animação confused...");
      roboEyes.anim_confused();
    }
  }
}
```

**Lógica:**
1. Verifica se touch está habilitado
2. Verifica se há toque
3. Aplica debounce (ignora toques repetidos em 500ms)
4. Obtém coordenadas do toque
5. Age baseado no modo:
   - **QRCODE_MODE**: Volta aos olhos
   - **EYES_MODE**: Animação confused

---

### 5. **Inicialização no Setup**

```cpp
// Inicializa Touchscreen
Serial.println("\n👆 Inicializando Touchscreen...");
touch.begin();
touch.setRotation(tft.getRotation());  // Mesmo que o TFT (rotação 4)
touchEnabled = true;
Serial.println("✅ Touchscreen inicializado!");
```

**Importante**: 
- `touch.setRotation()` deve ser igual a `tft.getRotation()`
- Garante que as coordenadas de touch estejam alinhadas com o display

---

### 6. **Chamada no Loop**

```cpp
void loop() {
  // Verifica toques na tela
  handleTouch();  ✅ Primeira verificação no loop
  
  // Verifica mensagens UART
  checkUARTMessages();
  
  // ... resto do código
}
```

**Posição**: Primeira verificação no loop para resposta rápida.

---

## 📺 Serial Monitor

### Touch no QR Code:
```
👆 Touch detectado: x=2048, y=1856, z=250
📱 Touch no QR Code - voltando aos olhos...
👀 Alternando para modo Eyes...
✅ Modo Eyes ativo!
```

### Touch nos Olhos:
```
👆 Touch detectado: x=1024, y=2048, z=280
👀 Touch nos olhos - executando animação confused...
[Animação confused é executada]
```

---

## 🎬 Comportamento Visual

### Cenário 1: Touch no QR Code
```
Estado Inicial: QR Code exibido
       ↓ [TOQUE NA TELA] 👆
Limpa tela (TFT_BLACK)
       ↓
Volta para RoboEyes
       ↓
Olhos retornam imediatamente
```

### Cenário 2: Touch nos Olhos
```
Estado Inicial: RoboEyes animando
       ↓ [TOQUE NA TELA] 👆
Executa anim_confused()
       ↓
Olhos fazem animação confused 😕
       ↓
Continua animação normal
```

---

## 🔄 Fluxo Completo

```
┌─────────────────────────────────────┐
│     EYES_MODE                       │
│  - RoboEyes animando                │
│  - Muda humor a cada 60s            │
│                                     │
│  👆 TOQUE: anim_confused()          │
└───────────┬─────────────────────────┘
            │
            │ Recebe TAG com URL
            ▼
┌─────────────────────────────────────┐
│     QRCODE_MODE                     │
│  - Exibe QR Code                    │
│  - Timeout 2 minutos                │
│                                     │
│  👆 TOQUE: Volta aos olhos          │
└───────────┬─────────────────────────┘
            │
            │ Timeout OU Toque
            ▼
┌─────────────────────────────────────┐
│     EYES_MODE                       │
│  - Volta aos olhos                  │
└─────────────────────────────────────┘
```

---

## 🎨 Animações RoboEyes

### anim_confused()
- Olhos ficam **confusos**
- Movimento errático
- Expressão de dúvida 😕

### anim_laugh()
- Executada quando **muda humor**
- Olhos "riem"
- Animação alegre 😄

---

## ⚙️ Configurações de Touch

### Hardware:
- **Chip**: XPT2046
- **Interface**: SPI
- **IRQ**: GPIO 36 (interrupção)
- **CS**: GPIO 33 (chip select)

### Software:
- **Debounce**: 500ms
- **Rotação**: Alinhada com TFT (rotação 4)
- **Coordenadas**: Raw (x, y, z - pressão)

### Calibração:
- **Não necessária** para este uso
- Apenas detecta toque (não importa posição exata)
- Se precisar de calibração:
  ```cpp
  touch.setCalibration(xMin, xMax, yMin, yMax);
  ```

---

## ✅ Checklist de Funcionalidades

- ✅ Biblioteca XPT2046_Touchscreen adicionada
- ✅ Touch inicializado no setup
- ✅ Rotação do touch alinhada com TFT
- ✅ Debounce de 500ms implementado
- ✅ Touch no QR Code volta aos olhos
- ✅ Touch nos olhos executa anim_confused
- ✅ Log de debug no Serial Monitor
- ✅ Verificação no loop principal

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

### 1. **platformio.ini**:
- Linha 122: Adicionada biblioteca `paulstoffregen/XPT2046_Touchscreen @ ^1.4`

### 2. **src/display/main.cpp**:
- Linha 4: Adicionado `#include <XPT2046_Touchscreen.h>`
- Linhas 24-26: Definidos pinos TOUCH_CS e TOUCH_IRQ
- Linha 32: Criada instância `touch`
- Linhas 71-74: Adicionadas variáveis de controle de touch
- Linhas 323-351: **NOVA** função `handleTouch()`
- Linhas 567-572: Inicialização do touch no setup
- Linha 607: Chamada de `handleTouch()` no loop

---

## 🎯 Próximas Melhorias (Opcional)

### Touch Gestures:
- **Swipe**: Detectar arraste na tela
- **Long Press**: Detectar toque longo
- **Double Tap**: Detectar toque duplo

### Calibração Precisa:
```cpp
// Se precisar de coordenadas exatas
touch.setCalibration(200, 3800, 200, 3800);

// Mapear coordenadas para pixels
int touchX = map(p.x, 200, 3800, 0, tft.width());
int touchY = map(p.y, 200, 3800, 0, tft.height());
```

### Áreas de Toque:
```cpp
// Definir área clicável para o QR Code
bool touchInQRArea(int x, int y) {
  // QR está em (120-220, 100-300)
  return (x >= 100 && x <= 220 && y >= 80 && y <= 280);
}
```

---

## 🐛 Troubleshooting

### Touch não responde:
1. Verificar conexões físicas dos pinos
2. Verificar se `touch.begin()` retorna sucesso
3. Testar com `touch.tirqTouched()` (IRQ touch)
4. Verificar se `touchEnabled = true`

### Múltiplos toques detectados:
1. Aumentar `TOUCH_DEBOUNCE` (ex: 1000ms)
2. Verificar ruído elétrico
3. Adicionar filtro de pressão:
   ```cpp
   if (p.z < 200) return;  // Pressão mínima
   ```

### Coordenadas invertidas:
1. Ajustar `touch.setRotation()` (0-3)
2. Testar diferentes rotações até alinhar

---

**Status**: ✅ **TOUCHSCREEN TOTALMENTE FUNCIONAL**  
**Data**: 15 de Dezembro de 2024  
**Funcionalidades**: 
- Touch habilitado
- QR Code → Touch → Volta aos olhos
- Olhos → Touch → Animação confused
**Pronto para uso!** 🚀👆

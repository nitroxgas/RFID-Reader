# 📺 ESP32-2432S028R (CYD) - Especificações e Correções

## 🎯 Especificações do Display

### Hardware:
- **Modelo**: ESP32-2432S028R (Cheap Yellow Display)
- **Display**: ILI9341 2.8" TFT
- **Resolução**: 320x240 pixels (QVGA)
- **Interface**: SPI
- **Touchscreen**: XPT2046 (resistivo)
- **Backlight**: GPIO 21 (PWM)

### Orientações Disponíveis:

| Rotação | Orientação | Largura | Altura | Uso |
|---------|------------|---------|--------|-----|
| **0** | Portrait | 240 | 320 | Vertical |
| **1** | Landscape | 320 | 240 | Horizontal ✅ |
| **2** | Portrait invertido | 240 | 320 | Vertical invertido |
| **3** | Landscape invertido | 320 | 240 | Horizontal invertido |

**Para este projeto**: Usamos **rotação 1** (landscape 320x240)

---

## ❌ Problema: Tela Riscada

### Causa:

O construtor do `RoboEyesTFT_eSPI.h` tinha as dimensões **invertidas** para landscape:

```cpp
// ❌ ERRADO (antes):
if (!portrait) {
  screenWidth = 240;   // ← Errado!
  screenHeight = 320;  // ← Errado!
}
```

**Resultado**: Sprite criado com 240x320, mas display configurado como 320x240 → Tela riscada!

---

## ✅ Solução Aplicada

### 1. **Corrigir Dimensões no Construtor**

```cpp
// ✅ CORRETO (agora):
if (!portrait) {
  screenWidth = 320;   // CYD landscape width
  screenHeight = 240;  // CYD landscape height
}
```

### 2. **Ajustar Profundidade de Cor do Sprite**

```cpp
// ❌ ANTES: 8-bit (256 cores)
sprite->setColorDepth(8);

// ✅ AGORA: 16-bit (65536 cores) - compatível com ILI9341
sprite->setColorDepth(16);
```

**Motivo**: ILI9341 usa RGB565 (16-bit). Sprite de 8-bit pode causar artefatos visuais.

### 3. **Remover setScreenSize() Duplicado**

```cpp
// ❌ ANTES:
roboEyes.setScreenSize(320, 240);  // Desnecessário
roboEyes.begin(50);

// ✅ AGORA:
roboEyes.begin(50);  // Dimensões já definidas no construtor
```

### 4. **Comentar Testes de Cores**

```cpp
/* Testes de cores - COMENTADO (funcionando)
tft.fillScreen(TFT_RED);
tft.fillScreen(TFT_GREEN);
tft.fillScreen(TFT_BLUE);
*/
tft.fillScreen(TFT_BLACK);  // Apenas limpa
```

---

## 🔧 Configuração Correta do TFT

### Setup Completo:

```cpp
void setup() {
  // 1. Liga backlight (CRÍTICO!)
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  
  // 2. Inicializa TFT
  tft.begin();
  tft.setRotation(1);        // Landscape 320x240
  tft.invertDisplay(false);  // Sem inversão
  tft.setSwapBytes(true);    // RGB→BGR para ILI9341
  
  // 3. Limpa tela
  tft.fillScreen(TFT_BLACK);
  
  // 4. Inicializa RoboEyes
  roboEyes.begin(50);  // 50 FPS, sprite 320x240x16bit
}
```

---

## 📊 Uso de Memória Sprite

### Antes (8-bit):
```
Sprite: 320 × 240 × 1 byte = 76.800 bytes (~75 KB)
```

### Agora (16-bit):
```
Sprite: 320 × 240 × 2 bytes = 153.600 bytes (~150 KB)
```

**RAM disponível**: 327 KB  
**Uso atual**: ~108 KB (33%)  
**Após sprite 16-bit**: ~185 KB (56%) ✅ Ainda OK!

---

## 🎨 Verificação de Cores

### RGB565 (16-bit):

| Cor | Hex | RGB | Aparência |
|-----|-----|-----|-----------|
| **Preto** | 0x0000 | (0, 0, 0) | ⬛ |
| **Branco** | 0xFFFF | (255, 255, 255) | ⬜ |
| **Vermelho** | 0xF800 | (255, 0, 0) | 🟥 |
| **Verde** | 0x07E0 | (0, 255, 0) | 🟩 |
| **Azul** | 0x001F | (0, 0, 255) | 🟦 |
| **Azul #2095F6** | 0x2095F6 → RGB565 | (32, 149, 246) | 🔵 |

**Swap de bytes**: `setSwapBytes(true)` converte RGB→BGR para ILI9341.

---

## 🐛 Debug: Como Verificar se Está Correto

### 1. **Verificar Dimensões no Serial**

Adicione após `roboEyes.begin()`:

```cpp
Serial.printf("Sprite: %dx%d @ %d-bit\n", 
  roboEyes.screenWidth, 
  roboEyes.screenHeight,
  16);  // colorDepth
```

**Esperado**: `Sprite: 320x240 @ 16-bit`

### 2. **Testar Preenchimento Simples**

Antes de `roboEyes.begin()`:

```cpp
tft.fillRect(0, 0, 160, 120, TFT_RED);      // Quadrante superior esquerdo
tft.fillRect(160, 0, 160, 120, TFT_GREEN);  // Quadrante superior direito
tft.fillRect(0, 120, 160, 120, TFT_BLUE);   // Quadrante inferior esquerdo
tft.fillRect(160, 120, 160, 120, TFT_YELLOW); // Quadrante inferior direito
delay(2000);
```

**Esperado**: 4 quadrados coloridos perfeitamente alinhados.

### 3. **Verificar Sprite Update**

No loop, adicione contador:

```cpp
static int frameCount = 0;
if (currentMode == EYES_MODE) {
  roboEyes.update();
  if (++frameCount % 50 == 0) {
    Serial.printf("Frame: %d\n", frameCount);
  }
}
```

**Esperado**: Log "Frame: 50, 100, 150..." sem travamentos.

---

## 📝 Comparação: Antes vs Depois

| Item | Antes (Errado) | Depois (Correto) |
|------|----------------|------------------|
| **Construtor Width** | 240 | 320 ✅ |
| **Construtor Height** | 320 | 240 ✅ |
| **Sprite ColorDepth** | 8-bit | 16-bit ✅ |
| **setScreenSize()** | Duplicado | Removido ✅ |
| **Testes Cores** | Sempre rodando | Comentado ✅ |
| **Resultado Visual** | Tela riscada ❌ | Olhos nítidos ✅ |

---

## 🎯 Ordem de Inicialização Correta

```
1. pinMode(TFT_BL, OUTPUT);
2. digitalWrite(TFT_BL, HIGH);
3. tft.begin();
4. tft.setRotation(1);          ← 320x240 landscape
5. tft.setSwapBytes(true);      ← RGB→BGR
6. tft.fillScreen(TFT_BLACK);
7. roboEyes.begin(50);          ← Cria sprite 320x240x16bit
8. roboEyes.setColors(TFT_WHITE, TFT_BLACK);
9. roboEyes.setWidth(100, 100);
10. roboEyes.setHeight(100, 100);
11. roboEyes.open();
```

**CRÍTICO**: Backlight **ANTES** de `tft.begin()`!

---

## ⚠️ Erros Comuns

### 1. **Dimensões Invertidas**
```cpp
❌ screenWidth = 240, screenHeight = 320 (em landscape)
✅ screenWidth = 320, screenHeight = 240
```

### 2. **ColorDepth Incorreto**
```cpp
❌ sprite->setColorDepth(8);   // 256 cores
✅ sprite->setColorDepth(16);  // 65536 cores
```

### 3. **Rotação Errada**
```cpp
❌ tft.setRotation(0);  // Portrait 240x320
✅ tft.setRotation(1);  // Landscape 320x240
```

### 4. **Swap de Bytes**
```cpp
❌ tft.setSwapBytes(false);  // Cores trocadas
✅ tft.setSwapBytes(true);   // Cores corretas
```

### 5. **Backlight Desligado**
```cpp
❌ Sem pinMode(TFT_BL, OUTPUT);  // Tela preta
✅ digitalWrite(TFT_BL, HIGH);   // Tela acesa
```

---

## 📚 Referências

### Pinout CYD:

| Pino | Função | Uso |
|------|--------|-----|
| GPIO 2 | TFT_DC | Data/Command |
| GPIO 12 | TFT_MISO | SPI MISO |
| GPIO 13 | TFT_MOSI | SPI MOSI |
| GPIO 14 | TFT_SCLK | SPI Clock |
| GPIO 15 | TFT_CS | Chip Select |
| **GPIO 21** | **TFT_BL** | **Backlight** |
| GPIO 22 | I2C_SCL | Touch SCL |
| GPIO 27 | I2C_SDA | Touch SDA |
| GPIO 33 | TOUCH_CS | Touch CS |

### TFT_eSPI Defines:

```cpp
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_MOSI   13
#define TFT_MISO   12
#define TFT_SCLK   14
#define TFT_CS     15
#define TFT_DC     2
#define TFT_RST    -1
#define TFT_BL     21
```

---

## ✅ Checklist de Verificação

Antes de gravar firmware:

- ✅ Backlight GPIO 21 configurado
- ✅ `tft.setRotation(1)` → 320x240
- ✅ `tft.setSwapBytes(true)` → cores corretas
- ✅ `roboEyes` construtor com `false` (landscape)
- ✅ Sprite `setColorDepth(16)` → 16-bit
- ✅ Dimensões construtor: 320x240
- ✅ `roboEyes.begin()` sem `setScreenSize()` duplicado
- ✅ Testes de cores comentados

---

## 🎊 Resultado Esperado

```
Display CYD (320x240):
┌─────────────────────────────────┐
│                                 │
│                                 │
│         👁️        👁️          │
│      (100x100)  (100x100)       │
│                                 │
│     Olhos brancos nítidos       │
│     Fundo preto uniforme        │
│     Sem riscos ou artefatos     │
│                                 │
└─────────────────────────────────┘
```

**Animação**: Pisca suavemente e olha ao redor. 50 FPS.

---

**Status**: ✅ **CORRIGIDO**  
**Problema**: Dimensões invertidas + ColorDepth errado  
**Solução**: Sprite 320x240x16bit correto  
**Data**: 15 de Dezembro de 2024

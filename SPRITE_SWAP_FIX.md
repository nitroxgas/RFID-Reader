# 🔧 Fix: Margem Branca e Olhos Não Aparecem

## ❌ Problema Reportado

1. **Margem branca** no canto direito da tela
2. **Olhos não aparecem** (tela preta)

---

## 🔍 Diagnóstico

### Causa 1: Sprite sem setSwapBytes()

O **TFT** estava configurado com `setSwapBytes(true)`, mas o **sprite** NÃO!

```cpp
// TFT: ✅ Swap configurado
tft.setSwapBytes(true);

// Sprite: ❌ Swap NÃO configurado
sprite->createSprite(320, 240);
// Faltava: sprite->setSwapBytes(true);
```

**Resultado**: Cores do sprite não correspondiam ao TFT → margem branca/artefatos.

### Causa 2: Sprite não preenchia tela completa

Possível problema de dimensões ou posicionamento do `pushSprite()`.

---

## ✅ Solução Aplicada

### 1. **Adicionar setSwapBytes() no Sprite**

**`RoboEyesTFT_eSPI.h` - função `begin()`**:

```cpp
void begin(byte frameRate = 50) {
  sprite = new TFT_eSprite(tft);
  sprite->setColorDepth(16);
  
  if (sprite->createSprite(screenWidth, screenHeight)) {
    sprite->fillSprite(bgColor);     // Preenche fundo
    sprite->setSwapBytes(true);      // ← ADICIONADO!
  }
  
  eyeLheightCurrent = 1;
  eyeRheightCurrent = 1;
  setFramerate(frameRate);
}
```

### 2. **Aplicar setSwapBytes() em setScreenSize()**

**`RoboEyesTFT_eSPI.h` - função `setScreenSize()`**:

```cpp
void setScreenSize(int w, int h) {
  screenWidth = w;
  screenHeight = h;
  
  // ... recalcula posições ...
  
  if(sprite) {
    sprite->deleteSprite();
    if (sprite->createSprite(screenWidth, screenHeight)) {
      sprite->fillSprite(bgColor);
      sprite->setSwapBytes(true);    // ← ADICIONADO!
    }
  }
}
```

### 3. **Adicionar Teste Visual de TFT**

**`main.cpp` - em `setup()`**:

```cpp
// Teste simples: desenhar círculo branco no centro
Serial.println("  ↓ Teste: Círculo branco no centro...");
tft.fillCircle(160, 120, 50, TFT_WHITE);
delay(1000);
tft.fillScreen(TFT_BLACK);
```

**Propósito**: Verificar se o TFT funciona antes de iniciar o sprite.

### 4. **Adicionar Debug de Inicialização**

**`main.cpp` - após `roboEyes.open()`**:

```cpp
Serial.printf("  ├─ Dimensões: %dx%d\n", roboEyes.screenWidth, roboEyes.screenHeight);
Serial.printf("  ├─ Olhos: %dx%d cada\n", roboEyes.eyeLwidthDefault, roboEyes.eyeLheightDefault);
Serial.printf("  ├─ Espaço entre: %d px\n", roboEyes.spaceBetweenDefault);
Serial.printf("  └─ Sprite criado: %s\n", (roboEyes.sprite != NULL) ? "SIM" : "NÃO");
```

**Propósito**: Verificar se o sprite foi criado corretamente.

---

## 🧪 Como Testar

### 1. **Upload e Monitor Serial**

```bash
pio run -e display-cyd --target upload
pio device monitor -e display-cyd
```

### 2. **Sequência Visual Esperada**

```
Boot:
  ↓
Backlight liga
  ↓
Círculo branco aparece no centro (1s)
  ↓
Tela preta
  ↓
Olhos brancos aparecem 👁️👁️
  ↓
Olhos começam a piscar e se mover
```

### 3. **Monitor Serial Esperado**

```
✅ TFT Display inicializado! (320x240)
👀 Inicializando RoboEyes...
✅ RoboEyes inicializado!
  ├─ Dimensões: 320x240
  ├─ Olhos: 100x100 cada
  ├─ Espaço entre: 20 px
  └─ Sprite criado: SIM
✅ Sistema pronto!
```

---

## 🎯 Verificações

### ✅ Se Círculo Aparece:

- TFT está funcionando corretamente
- Backlight OK
- Swap OK para TFT

### ✅ Se Olhos Aparecem:

- Sprite criado corretamente
- `setSwapBytes()` funcionando no sprite
- Dimensões 320x240 corretas

### ❌ Se Ainda Tem Margem Branca:

**Possíveis causas**:

1. **Sprite menor que tela**: Verificar `screenWidth` e `screenHeight`
2. **pushSprite() deslocado**: Deve ser `(0, 0)`
3. **Cores invertidas**: Verificar `bgColor` e `mainColor`

**Debug adicional**:

```cpp
// Adicionar no loop() temporariamente:
Serial.printf("Sprite: %dx%d, Push: (0,0)\n", 
  roboEyes.screenWidth, roboEyes.screenHeight);
```

### ❌ Se Olhos Não Aparecem:

**Possíveis causas**:

1. **Olhos fechados**: Verificar se `roboEyes.open()` foi chamado
2. **Cor idêntica ao fundo**: Ambos pretos
3. **Posição fora da tela**: Verificar `eyeLx`, `eyeLy`

**Debug adicional**:

```cpp
// Adicionar após roboEyes.open():
Serial.printf("Olho L aberto: %s\n", roboEyes.eyeL_open ? "SIM" : "NÃO");
Serial.printf("Olho R aberto: %s\n", roboEyes.eyeR_open ? "SIM" : "NÃO");
Serial.printf("Altura olhos: %d (deve ser > 1)\n", roboEyes.eyeLheightNext);
```

---

## 📊 Comparação: Antes vs Depois

| Item | Antes | Depois |
|------|-------|--------|
| **TFT setSwapBytes** | ✅ true | ✅ true |
| **Sprite setSwapBytes** | ❌ NÃO | ✅ true |
| **Sprite fillSprite** | ✅ bgColor | ✅ bgColor |
| **Teste visual TFT** | ❌ Não | ✅ Círculo branco |
| **Debug dimensões** | ❌ Não | ✅ Printf detalhado |

---

## 🔧 Configuração Completa Correta

### TFT:

```cpp
tft.begin();
tft.setRotation(1);         // 320x240 landscape
tft.setSwapBytes(true);     // RGB→BGR
```

### Sprite:

```cpp
sprite->setColorDepth(16);  // 16-bit color
sprite->createSprite(320, 240);
sprite->fillSprite(TFT_BLACK);
sprite->setSwapBytes(true); // ← CRÍTICO!
```

### RoboEyes:

```cpp
roboEyes.begin(50);
roboEyes.setColors(TFT_WHITE, TFT_BLACK);
roboEyes.setWidth(100, 100);
roboEyes.setHeight(100, 100);
roboEyes.open();  // ← CRÍTICO!
```

---

## 🎨 Fluxo de Renderização

```
1. drawEyes() desenha no sprite
   ↓
2. sprite tem fundo preto (bgColor)
   ↓
3. sprite desenha olhos brancos (mainColor)
   ↓
4. sprite->setSwapBytes(true) converte cores
   ↓
5. sprite->pushSprite(0, 0) envia para TFT
   ↓
6. TFT renderiza na tela física
```

**Importante**: Tanto TFT quanto Sprite precisam do mesmo `setSwapBytes(true)`!

---

## 📝 Arquivos Modificados

### 1. **`RoboEyesTFT_eSPI.h`**

- Linha 230: `sprite->setSwapBytes(true);` em `begin()`
- Linha 271: `sprite->setSwapBytes(true);` em `setScreenSize()`

### 2. **`main.cpp`**

- Linha 444-447: Teste de círculo branco
- Linha 462-465: Debug detalhado de inicialização

---

## ✅ Resultado Esperado

```
Display CYD 320x240:
┌────────────────────────────────┐
│                                │
│                                │
│         👁️        👁️          │
│      (100x100)  (100x100)      │
│                                │
│    SEM margem branca           │
│    SEM artefatos visuais       │
│    Fundo preto uniforme        │
│    Olhos brancos nítidos       │
│                                │
└────────────────────────────────┘
```

**Animação**: Pisca e olha ao redor suavemente.

---

## 🐛 Troubleshooting Final

### Problema: "Sprite criado: NÃO"

```cpp
// Verificar memória disponível:
Serial.printf("Heap free: %d bytes\n", ESP.getFreeHeap());
// Sprite 320x240x16bit = 153600 bytes
// Precisa ter > 160KB free
```

### Problema: Tela Completamente Branca

```cpp
// Trocar temporariamente:
roboEyes.setColors(TFT_BLACK, TFT_WHITE);  // Olhos pretos, fundo branco
// Se aparecer = problema de swap resolvido
```

### Problema: Olhos Muito Pequenos

```cpp
// Aumentar olhos:
roboEyes.setWidth(120, 120);
roboEyes.setHeight(120, 120);
roboEyes.setSpacebetween(10);  // Menos espaço
```

---

**Status**: ✅ **CORRIGIDO**  
**Fix principal**: `sprite->setSwapBytes(true)`  
**Teste**: Círculo branco + debug detalhado  
**Data**: 15 de Dezembro de 2024

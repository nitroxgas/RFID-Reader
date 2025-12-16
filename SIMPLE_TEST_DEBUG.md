# 🔍 Debug: Versão Simplificada SEM Sprite

## 🎯 Objetivo

Isolar o problema da "faixa branca lateral" testando o TFT **DIRETAMENTE** sem sprite ou RoboEyes.

---

## ❌ Problema Original

- **Sintoma**: Tela preta com faixa branca na lateral direita
- **Suspeita**: Conflito entre sprite do RoboEyes e LVGL, ou dimensões incorretas

---

## ✅ Solução de Debug

### Mudanças Implementadas:

1. **RoboEyes DESABILITADO** temporariamente
2. **Sprite REMOVIDO** completamente
3. **Desenho DIRETO** no TFT usando primitivas
4. **LVGL** alocado sob demanda (apenas para QR Code)
5. **Animação simples** de olhos piscando

---

## 📊 Código Simplificado

### Configuração TFT:

```cpp
tft.begin();
tft.setRotation(1);        // LANDSCAPE 320x240
tft.invertDisplay(false);
tft.setSwapBytes(true);    // Para ILI9341
```

**Verificações**:
```cpp
Serial.printf("Resolução: %dx%d\n", tft.width(), tft.height());
Serial.printf("Rotação: %d\n", tft.getRotation());
```

**Esperado**:
```
Resolução: 320x240
Rotação: 1
```

---

### Animação Simples (SEM Sprite):

```cpp
void drawSimpleEyes() {
  // Limpa área dos olhos
  tft.fillCircle(eyeLeftX, eyeY, eyeSize/2 + 5, TFT_BLACK);
  tft.fillCircle(eyeRightX, eyeY, eyeSize/2 + 5, TFT_BLACK);
  
  if (eyesOpen) {
    // Olhos abertos = círculos brancos
    tft.fillCircle(eyeLeftX, eyeY, eyeSize/2, TFT_WHITE);
    tft.fillCircle(eyeRightX, eyeY, eyeSize/2, TFT_WHITE);
  } else {
    // Olhos fechados = linhas horizontais
    tft.fillRect(eyeLeftX - eyeSize/2, eyeY - 2, eyeSize, 4, TFT_WHITE);
    tft.fillRect(eyeRightX - eyeSize/2, eyeY - 2, eyeSize, 4, TFT_WHITE);
  }
}
```

**Parâmetros**:
- `eyeSize = 80` (diâmetro dos olhos)
- `eyeY = 120` (centro vertical da tela)
- `eyeLeftX = 80` (olho esquerdo)
- `eyeRightX = 240` (olho direito)

---

### Loop de Animação:

```cpp
void loop() {
  // ... UART, timeouts ...
  
  if (currentMode == EYES_MODE) {
    // Pisca a cada 3 segundos
    if (millis() - lastBlink > 3000) {
      eyesOpen = !eyesOpen;
      drawSimpleEyes();      // Fecha
      delay(200);
      eyesOpen = !eyesOpen;
      drawSimpleEyes();      // Abre
      lastBlink = millis();
    }
    delay(10);
  }
}
```

---

## 🧪 Sequência de Teste Visual

### 1. **Boot** (5 segundos):

```
Backlight liga
  ↓
Vermelho (500ms)
  ↓
Verde (500ms)
  ↓
Azul (500ms)
  ↓
Preto (500ms)
  ↓
Olhos aparecem 👁️👁️
```

**Se alguma cor preenche SÓ PARTE da tela**: rotação ou dimensão errada!

---

### 2. **Olhos Estáticos** (após boot):

```
┌─────────────────────────────────┐
│                                 │
│                                 │
│      ⚪           ⚪             │
│   (x=80)       (x=240)          │
│    y=120        y=120           │
│                                 │
│   SEM faixa branca              │
│   SEM riscos                    │
└─────────────────────────────────┘
```

**O que verificar**:
- ✅ Dois círculos brancos perfeitos
- ✅ Fundo totalmente preto
- ✅ Sem margem branca em NENHUM lado
- ✅ Olhos centralizados verticalmente

---

### 3. **Animação de Piscar** (a cada 3s):

```
Olhos abertos ⚪⚪
   ↓ (200ms)
Olhos fechados ━━
   ↓ (200ms)
Olhos abertos ⚪⚪
   ↓ (aguarda 3s)
Repete...
```

---

## 📊 Monitor Serial Esperado

```
╔══════════════════════════════════════════╗
║   ESP32-2432S028R (CYD) Display         ║
║   RFID Reader System                     ║
╚══════════════════════════════════════════╝

🔗 Inicializando UART (TX: GPIO22, RX: GPIO27)...
📤 UART >> STATUS|DISPLAY_READY
📺 Inicializando TFT Display...
  ↳ Ligando backlight (GPIO21)...
  ↓ Teste 1: Vermelho...
  ↓ Teste 2: Verde...
  ↓ Teste 3: Azul...
  ↓ Teste 4: Tela preta...
✅ TFT Display inicializado! (320x240 landscape)
  ├─ Resolução: 320x240
  ├─ Rotação: 1
  └─ Heap livre: XXXXX bytes
👀 Iniciando animação de teste (sem sprite)...
  └─ Desenhando olhos direto no TFT...
  └─ Olhos desenhados! Piscarão a cada 3s

✅ Sistema pronto!
⏳ Aguardando dados do Reader via UART...
```

---

## 🔍 Análise de Resultados

### ✅ Se Olhos Aparecem Corretamente:

**Conclusão**: O problema estava no sprite ou RoboEyes!

**Próximos passos**:
1. Revisar configuração do sprite (16-bit vs 8-bit)
2. Verificar se `setSwapBytes()` estava faltando no sprite
3. Testar sprite simples sem RoboEyes

---

### ❌ Se AINDA Tem Faixa Branca:

**Possíveis causas**:

1. **Rotação errada**:
   ```cpp
   // Verificar no Serial:
   // Se aparecer "Resolução: 240x320" = ROTAÇÃO ERRADA!
   // Deve ser "Resolução: 320x240"
   ```

2. **Dimensões hardcoded erradas**:
   ```cpp
   // TFT_eSPI User_Setup.h pode ter dimensões erradas
   // Verificar #define TFT_WIDTH e TFT_HEIGHT
   ```

3. **Problema de hardware**:
   ```cpp
   // Testar em outro display ou sketch simples
   ```

---

### ❌ Se Olhos NÃO Aparecem (tela preta):

**Possíveis causas**:

1. **Backlight desligado**:
   ```cpp
   // Verificar GPIO 21 = HIGH
   pinMode(TFT_BL, OUTPUT);
   digitalWrite(TFT_BL, HIGH);
   ```

2. **Cores erradas**:
   ```cpp
   // Tentar inverter: olhos pretos, fundo branco
   tft.fillScreen(TFT_WHITE);
   tft.fillCircle(..., TFT_BLACK);
   ```

3. **setSwapBytes() errado**:
   ```cpp
   // Tentar trocar:
   tft.setSwapBytes(false);  // Ou true
   ```

---

## 💾 Uso de Memória

### Antes (com RoboEyes + Sprite):

```
RAM:   [===       ]  33.1% (used 108KB from 327KB)
Flash: [====      ]  38.5% (used 504KB from 1310KB)
```

### Agora (SEM Sprite):

```
RAM:   [===       ]  27.1% (used 88KB from 327KB)
Flash: [====      ]  37.6% (used 493KB from 1310KB)
```

**Economia**:
- **RAM**: -20KB (sprite removido)
- **Flash**: -11KB (RoboEyes library removida)

---

## 🔧 Próximos Passos (Após Debug)

### Se funcionar:

1. **Criar sprite simples**:
   ```cpp
   TFT_eSprite sprite = TFT_eSprite(&tft);
   sprite.setColorDepth(16);
   sprite.createSprite(320, 240);
   sprite.setSwapBytes(true);  // CRÍTICO!
   sprite.fillSprite(TFT_BLACK);
   ```

2. **Testar sprite com olhos**:
   ```cpp
   // Desenhar no sprite em vez de no tft
   sprite.fillCircle(80, 120, 40, TFT_WHITE);
   sprite.fillCircle(240, 120, 40, TFT_WHITE);
   sprite.pushSprite(0, 0);
   ```

3. **Reativar RoboEyes** (se sprite funcionar)

4. **Integrar LVGL** para QR Code

---

### Se NÃO funcionar:

1. **Teste ultra-básico**:
   ```cpp
   // Apenas preencher tela inteira
   tft.fillScreen(TFT_WHITE);
   delay(2000);
   tft.fillScreen(TFT_BLACK);
   ```

2. **Verificar TFT_eSPI setup**:
   - `User_Setup.h` ou `User_Setup_Select.h`
   - Pinos, driver (ILI9341), dimensões

3. **Testar exemplo TFT_eSPI**:
   ```bash
   # Carregar exemplo básico da biblioteca
   ```

---

## 📝 Comandos de Teste

### Upload:

```bash
pio run -e display-cyd --target upload
```

### Monitor Serial:

```bash
pio device monitor -e display-cyd
```

### Verificar Heap:

```cpp
Serial.printf("Heap: %d\n", ESP.getFreeHeap());
```

---

## 🎯 Checklist de Verificação

- ✅ Testes de cores (RGB) preenchem TELA TODA?
- ✅ Olhos aparecem na tela?
- ✅ Olhos estão centralizados?
- ✅ Fundo é totalmente preto (sem margem branca)?
- ✅ Olhos piscam a cada 3 segundos?
- ✅ Resolução Serial mostra "320x240"?
- ✅ Rotação Serial mostra "1"?

---

## 🔍 Debug Avançado

### Se PARTE do desenho aparece:

```cpp
// Adicionar grid de teste:
for (int x = 0; x < 320; x += 40) {
  tft.drawFastVLine(x, 0, 240, TFT_RED);
}
for (int y = 0; y < 240; y += 40) {
  tft.drawFastHLine(0, y, 320, TFT_GREEN);
}
// Deve formar grade 8x6 perfeita
```

### Se cores estranhas:

```cpp
// Testar todas as combinações:
tft.setSwapBytes(true);
tft.invertDisplay(false);

// vs

tft.setSwapBytes(false);
tft.invertDisplay(true);

// vs

tft.setSwapBytes(true);
tft.invertDisplay(true);
```

---

## 📚 Arquivos Modificados

- `src/display/main.cpp`: Código simplificado
- RoboEyes: Comentado temporariamente
- LVGL buffers: Alocação dinâmica sob demanda

---

**Status**: ✅ **PRONTO PARA TESTE**  
**Compilação**: SUCCESS  
**RAM**: 27.1% (88KB)  
**Objetivo**: Identificar se problema é no sprite ou no TFT  
**Data**: 15 de Dezembro de 2024

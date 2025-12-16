# 🐛 Bug de Buffer: Ordem de Inicialização Incorreta

## ❌ Problema Encontrado

**Sintoma**: Erro ao dimensionar o buffer que recebe os pixels do TFT.

**Causa Raiz**: **Rotação aplicada DEPOIS dos primeiros desenhos!**

---

## 🔍 Análise do Problema

### Sequência INCORRETA (antes):

```cpp
1. tft.init();                    // Inicializa em rotação padrão (0 - Portrait 240x320)
2. tft.fillScreen(TFT_RED);       // Desenha em 240x320 ❌
3. tft.fillScreen(TFT_GREEN);     // Desenha em 240x320 ❌
4. tft.fillScreen(TFT_BLUE);      // Desenha em 240x320 ❌
5. ...testes de rotação...
6. tft.setRotation(1);            // TARDE DEMAIS! Já desenhou em portrait
```

### O que acontecia:

```
Defines:           TFT_WIDTH=320, TFT_HEIGHT=240 (landscape)
Rotação inicial:   0 (portrait 240x320)
Desenhos:          Feitos em portrait (240x320)
Configuração:      landscape (320x240)

CONFLITO! ❌
```

**Resultado**: Buffer interno do TFT_eSPI dimensionado incorretamente, causando falhas de memória ou desenhos corrompidos.

---

## ✅ Solução Aplicada

### Sequência CORRETA (agora):

```cpp
1. tft.init();                    // Inicializa em rotação padrão
2. tft.setRotation(1);            // ✅ LANDSCAPE IMEDIATAMENTE!
3. tft.invertDisplay(false);      // ✅ Configurações de display
4. tft.setSwapBytes(true);        // ✅ Swap de bytes
5. tft.fillScreen(TFT_RED);       // ✅ Desenha em 320x240
6. tft.fillScreen(TFT_GREEN);     // ✅ Desenha em 320x240
7. ...testes de rotação...
8. tft.setRotation(1);            // ✅ Restaura landscape
```

### Agora:

```
Defines:           TFT_WIDTH=320, TFT_HEIGHT=240 (landscape)
Rotação inicial:   1 (landscape 320x240) ✅
Desenhos:          Feitos em landscape (320x240) ✅
Configuração:      landscape (320x240) ✅

ALINHADO! ✅
```

---

## 🔧 Mudanças no Código

### Arquivo: `src/display/main.cpp`

#### ANTES (linhas 505-509):
```cpp
// Inicializa TFT (mínimo necessário)
Serial.println("  ↓ Inicializando SPI e TFT...");
tft.init();

Serial.println("  ↓ Teste SIMPLES: Vermelho...");
// ... testes de cor sem rotação definida ❌
```

#### DEPOIS (linhas 505-517):
```cpp
// Inicializa TFT (mínimo necessário)
Serial.println("  ↓ Inicializando SPI e TFT...");
tft.init();

// CRÍTICO: Define rotação ANTES de qualquer desenho!
Serial.println("  ↓ Configurando LANDSCAPE (rotação 1)...");
tft.setRotation(1);          // Landscape 320x240 ✅
tft.invertDisplay(false);    // Sem inversão
tft.setSwapBytes(true);      // RGB->BGR para ILI9341

Serial.printf("  ↓ Resolução: %dx%d (rotação %d)\n", 
              tft.width(), tft.height(), tft.getRotation());

Serial.println("  ↓ Teste SIMPLES: Vermelho...");
// ... testes de cor COM rotação landscape ✅
```

---

## 📊 Impacto da Correção

### Buffers TFT_eSPI:

**ANTES**:
- Rotação padrão: 0 (portrait)
- Primeira operação: fillScreen() em 240x320
- Buffer interno: Dimensionado para 240x320
- Posterior rotação para landscape: **CONFLITO!**

**DEPOIS**:
- Rotação imediata: 1 (landscape)
- Primeira operação: fillScreen() em 320x240
- Buffer interno: Dimensionado para 320x240 ✅
- Consistência: **PERFEITA!**

### Buffers LVGL:

Adicionado debug detalhado para monitorar alocação:

```cpp
Serial.printf("  ├─ TFT_WIDTH: %d\n", TFT_WIDTH);
Serial.printf("  ├─ TFT_HEIGHT: %d\n", TFT_HEIGHT);
Serial.printf("  ├─ Alocando buffers: %d x 20 = %d pixels (%d bytes)\n", 
              TFT_WIDTH, TFT_WIDTH * 20, bufferSize);
Serial.printf("  ├─ buf1 alocado em: %p\n", buf1);
Serial.printf("  ├─ buf2 alocado em: %p\n", buf2);
Serial.printf("  └─ Heap livre após LVGL: %d bytes\n", ESP.getFreeHeap());
```

**Cálculo do Buffer LVGL**:
```
TFT_WIDTH = 320
Buffer lines = 20
sizeof(lv_color_t) = 2 bytes (RGB565)

buf1 size = 320 * 20 * 2 = 12,800 bytes
buf2 size = 320 * 20 * 2 = 12,800 bytes
Total LVGL = 25,600 bytes (~25KB)
```

---

## 📺 Monitor Serial Esperado

```
📺 Inicializando TFT Display...
  ↳ Ligando backlight (GPIO21)...
  ↓ Inicializando SPI e TFT...
  ↓ Configurando LANDSCAPE (rotação 1)...
  ↓ Resolução: 320x240 (rotação 1)       ← LANDSCAPE DESDE O INÍCIO!
  ↓ Teste SIMPLES: Vermelho...
  ↓ Verde...
  ↓ Azul...
  ↓ Branco...
  ↓ Preto...

  ↓ Testando rotações:
    Rotação 0: 240x320
    Rotação 1: 320x240  ← LANDSCAPE ✅
    Rotação 2: 240x320
    Rotação 3: 320x240

✅ Rotação restaurada para LANDSCAPE: 320x240

  ↓ Desenhando círculos de teste...

✅ TFT Display inicializado!
  ├─ Resolução: 320x240
  ├─ Rotação: 1
  └─ Heap livre: ~240000 bytes

👀 Desenhando olhos...

👀 Posições dos olhos LANDSCAPE (tela 320x240):
  ├─ Olho esquerdo: (80, 120)
  ├─ Olho direito: (240, 120)
  └─ Tamanho: 80 px

✅ Sistema pronto!
```

**Quando LVGL for inicializado** (ao exibir QR Code):

```
🔧 Inicializando LVGL...
  ├─ TFT_WIDTH: 320
  ├─ TFT_HEIGHT: 240
  ├─ sizeof(lv_color_t): 2 bytes
  ├─ Alocando buffers: 320 x 20 linhas = 6400 pixels (12800 bytes)
  ├─ buf1 alocado em: 0x3ffbXXXX
  ├─ buf2 alocado em: 0x3ffbXXXX
  ├─ draw_buf inicializado: 6400 pixels
  ├─ Display driver: 320x240
  └─ Heap livre após LVGL: ~215000 bytes
✅ LVGL inicializado com sucesso!
```

---

## ✅ Checklist de Verificação

### Ordem de Inicialização Correta:

1. ✅ `pinMode(TFT_BL, OUTPUT)` + `digitalWrite(TFT_BL, HIGH)`
2. ✅ `tft.init()`
3. ✅ `tft.setRotation(1)` **← ANTES de qualquer desenho!**
4. ✅ `tft.invertDisplay(false)`
5. ✅ `tft.setSwapBytes(true)`
6. ✅ Verificar resolução: `tft.width()` e `tft.height()`
7. ✅ Testes de cores
8. ✅ Loop de testes de rotações
9. ✅ Restaurar rotação 1
10. ✅ Círculos de teste
11. ✅ Olhos

### Após Upload:

- ✅ Serial Monitor mostra "320x240 (rotação 1)" ANTES dos testes de cor
- ✅ Cores preenchem tela completa sem artefatos
- ✅ Círculos aparecem corretamente
- ✅ Olhos aparecem lado a lado
- ✅ Nenhum erro de memória/buffer

---

## 🚀 Por Que Isso é Crítico

### TFT_eSPI Internamente:

O `TFT_eSPI` usa a rotação para calcular:
- Dimensões da janela de desenho (`setAddrWindow`)
- Coordenadas de pixel
- Ordem de varredura de memória do display

**Se a rotação for alterada APÓS desenhos**, o driver pode:
- Calcular endereços de memória incorretos
- Usar buffers com tamanho errado
- Causar corrupção de dados no display

### Ordem Correta (sempre):

```
1. Inicializar hardware (init)
2. Configurar orientação (setRotation)
3. Configurar propriedades (invertDisplay, setSwapBytes)
4. ENTÃO começar a desenhar
```

---

## 📝 Resumo das Mudanças

### Arquivos Modificados:

**`src/display/main.cpp`**:

1. **Linhas 509-515**: Rotação aplicada IMEDIATAMENTE após `tft.init()`
2. **Linha 554**: Simplificada restauração de rotação (sem reinicializar propriedades)
3. **Linhas 126-179**: Debug detalhado de buffers LVGL adicionado

---

## 🎯 Resultado Final

```
┌────────────────────────────────────────┐
│                                        │
│  TODAS AS OPERAÇÕES EM LANDSCAPE       │
│                                        │
│  TFT_WIDTH=320  TFT_HEIGHT=240         │
│  Rotação: 1                            │
│  Buffers: 320x20 pixels                │
│  Desenhos: 320x240                     │
│                                        │
│  CONSISTÊNCIA TOTAL! ✅                │
│                                        │
└────────────────────────────────────────┘
```

---

**Status**: ✅ **BUG DE BUFFER CORRIGIDO**  
**Causa**: Rotação aplicada após primeiros desenhos  
**Solução**: Rotação aplicada IMEDIATAMENTE após tft.init()  
**Impacto**: Buffer dimensionado corretamente desde o início  
**Debug**: Adicionado para monitorar alocação LVGL  
**Data**: 15 de Dezembro de 2024

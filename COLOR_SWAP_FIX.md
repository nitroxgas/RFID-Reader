# 🎨 Fix: Cores Trocadas nos Componentes LVGL

## ❌ Problema

**Sintoma**: Teste de cores RGB funciona perfeitamente (vermelho, verde, azul corretos), MAS os componentes do SquareLine Studio aparecem com cores trocadas (azul vira vermelho, etc).

**Causa**: Conflito entre configuração LVGL e SquareLine Studio quanto ao byte swap.

---

## 🔍 Análise do Problema

### SquareLine Studio Exige:

```c
// display/ui/ui.c (linha 20-22)
#if LV_COLOR_16_SWAP !=0
    #error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif
```

**SquareLine força** `LV_COLOR_16_SWAP = 0` no LVGL!

### Display CYD (ILI9341) Precisa:

O ILI9341 do CYD espera cores em formato **BGR** (Blue-Green-Red), não RGB!

- TFT_eSPI: Envia cores em RGB (padrão)
- ILI9341: Espera BGR
- Resultado: **Cores trocadas!**

---

## ✅ Solução

**NÃO** fazer swap no LVGL (SquareLine proíbe), fazer swap no **TFT_eSPI**:

```cpp
void setup() {
  // ...
  
  tft.begin();
  tft.setRotation(1);
  tft.invertDisplay(false);
  tft.setSwapBytes(true);  // ← CRÍTICO!
  
  // ...
}
```

### Por que Funciona?

```
LVGL (RGB) → TFT_eSPI → setSwapBytes(true) → BGR → ILI9341 ✅
```

1. **LVGL** gera cores em RGB (LV_COLOR_16_SWAP=0)
2. **TFT_eSPI** recebe RGB
3. **setSwapBytes(true)** converte RGB → BGR
4. **ILI9341** recebe BGR (formato correto)
5. **Display mostra cores corretas!**

---

## 📊 Antes vs Depois

### ❌ ANTES (setSwapBytes = false)

```cpp
tft.setSwapBytes(false);  // Errado!

// Fluxo:
LVGL (RGB) → TFT → ILI9341 espera BGR
              ↓
         Cores trocadas! ❌
```

**Resultado**:
- Teste RGB: ✅ Correto (TFT_eSPI envia direto)
- LVGL UI: ❌ Cores trocadas (LVGL → TFT sem swap)

### ✅ DEPOIS (setSwapBytes = true)

```cpp
tft.setSwapBytes(true);  // Correto!

// Fluxo:
LVGL (RGB) → TFT → swap → BGR → ILI9341
                     ↓
              Cores corretas! ✅
```

**Resultado**:
- Teste RGB: ✅ Correto
- LVGL UI: ✅ Correto!

---

## 🔧 Código Completo

### `src/display/main.cpp`

```cpp
void setup() {
  // ... UART, backlight ...
  
  // Inicializa TFT
  tft.begin();
  tft.setRotation(1);        // Landscape
  tft.invertDisplay(false);  // Sem inversão
  tft.setSwapBytes(true);    // ← SWAP RGB→BGR para ILI9341
  
  // Teste RGB
  tft.fillScreen(TFT_RED);   // Vermelho
  delay(500);
  tft.fillScreen(TFT_GREEN); // Verde
  delay(500);
  tft.fillScreen(TFT_BLUE);  // Azul
  delay(500);
  
  // LVGL + SquareLine
  initLVGL();
  ui_init();
  createQRCodeOverlay();
}
```

### `include/lv_conf.h`

```c
// MANTÉM LV_COLOR_16_SWAP = 0 (exigência SquareLine)
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0  // ← NÃO ALTERAR!
```

---

## 🧪 Como Verificar

### 1. Teste RGB (TFT direto)

```cpp
tft.fillScreen(TFT_RED);   // Deve ser VERMELHO
tft.fillScreen(TFT_GREEN); // Deve ser VERDE
tft.fillScreen(TFT_BLUE);  // Deve ser AZUL
```

✅ Se teste passa = TFT_eSPI OK

### 2. SquareLine UI (LVGL)

```cpp
ui_init();  // Spinner + Olho azul
```

**Verificar**:
- `ui_Olho`: Deve ser **AZUL** (#2095F6)
- Spinner: Deve ser cor correta
- Fundo: Deve ser cor correta

✅ Se cores corretas = LVGL OK!

---

## 🎨 Cores Específicas do SquareLine

### `ui_Screen1.c`

```c
// Linha 35: Olho deve ser AZUL
lv_obj_set_style_bg_color(ui_Olho, lv_color_hex(0x2095F6), ...);
//                                              ^^^^^^^^
//                                         Azul: #2095F6
```

**Teste Visual**:
- ❌ Se aparece LARANJA/VERMELHO → Swap errado
- ✅ Se aparece AZUL → Swap correto!

---

## ⚠️ IMPORTANTE: Não Alterar!

### ❌ NÃO FAÇA:

```c
// lv_conf.h
#define LV_COLOR_16_SWAP 1  // ❌ SquareLine vai dar erro!
```

**Erro de compilação**:
```
error: "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
```

### ✅ FAÇA:

```cpp
// main.cpp
tft.setSwapBytes(true);  // ✅ Swap no TFT, não LVGL!
```

---

## 📝 Resumo Técnico

| Layer | Formato Saída | Swap? |
|-------|---------------|-------|
| **LVGL** | RGB | ❌ LV_COLOR_16_SWAP=0 (SquareLine) |
| **TFT_eSPI** | RGB → BGR | ✅ setSwapBytes(true) |
| **ILI9341** | BGR (entrada) | - |

**Fluxo Final**:
```
LVGL (RGB) → TFT_eSPI (swap) → ILI9341 (BGR) ✅
```

---

## 🔍 Debug

### Se cores ainda trocadas:

1. **Verificar lv_conf.h**:
   ```c
   #define LV_COLOR_16_SWAP 0  // Deve ser 0!
   ```

2. **Verificar main.cpp**:
   ```cpp
   tft.setSwapBytes(true);  // Deve ser true!
   ```

3. **Rebuild completo**:
   ```bash
   pio run -e display-cyd --target clean
   pio run -e display-cyd
   ```

4. **Verificar ui.c não foi modificado**:
   ```c
   // display/ui/ui.c linha 20
   #if LV_COLOR_16_SWAP !=0
       #error ...
   #endif
   ```

### Se teste RGB trocado:

```cpp
// Remover swap (só se teste RGB falhar)
tft.setSwapBytes(false);
```

Mas isso indica problema no TFT_eSPI, não LVGL!

---

## 📚 Referências

### ILI9341 Datasheet
- Pixel format: RGB565 (16-bit)
- Byte order: Pode ser RGB ou BGR (configurável)
- CYD usa: **BGR** por padrão

### LVGL v8.4 Color Handling
- `LV_COLOR_16_SWAP`: Swaps bytes no nível LVGL
- Quando = 0: LVGL usa RGB nativo
- Quando = 1: LVGL swap para BGR

### TFT_eSPI
- `setSwapBytes(true)`: Swap RGB↔BGR em pushColors()
- Afeta apenas saída para display
- Não afeta desenhos internos TFT

---

## ✅ Checklist Fix

- ✅ `tft.setSwapBytes(true)` em setup()
- ✅ `LV_COLOR_16_SWAP 0` em lv_conf.h
- ✅ Teste RGB correto
- ✅ UI SquareLine cores corretas
- ✅ Olho azul (#2095F6) aparece azul
- ✅ QR Code overlay funcional
- ✅ Compilação sem erros
- ✅ Upload success

---

## 🎊 Resultado Final

```
Display CYD:
┌──────────────────────────────┐
│         ⚙️ Spinner          │
│        ( 👁️ ) AZUL!         │  ← Cores corretas!
│         ⚙️  #2095F6         │
│                              │
│  [QR Code quando URL]        │
│  Branco com borda azul       │  ← Cores corretas!
└──────────────────────────────┘
```

**Status**: ✅ **CORES CORRETAS!**

---

**Data**: 15 de Dezembro de 2024  
**Fix**: `tft.setSwapBytes(true)`  
**Motivo**: ILI9341 espera BGR, SquareLine força LV_COLOR_16_SWAP=0

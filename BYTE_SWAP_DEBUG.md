# 🔍 Debug: Byte Swap TFT_eSPI vs LVGL

## 🎯 Problema

Teste RGB correto ✅, mas cores LVGL trocadas ❌

**Causa**: Conflito entre `setSwapBytes()` e `pushColors(swap)`.

---

## 📊 TFT_eSPI: Duas Formas de Swap

### 1. `setSwapBytes(bool)`
Afeta funções TFT_eSPI:
- `fillScreen()`
- `drawPixel()`
- `drawRect()`
- etc.

**NÃO afeta** `pushColors()` quando parâmetro swap é explícito!

### 2. `pushColors(data, len, swap)`
Swap **independente** de `setSwapBytes()`:
- `swap=true` → força swap nesta chamada
- `swap=false` → sem swap nesta chamada

---

## ⚙️ Combinações Possíveis

### Opção 1: ❌ Double Swap (ERRADO)
```cpp
tft.setSwapBytes(true);
tft.pushColors(data, len, true);  // Swap 2x = cores trocadas!
```

**Resultado**: ❌ Cores trocadas

---

### Opção 2: ✅ Swap no setSwapBytes (TESTANDO)
```cpp
tft.setSwapBytes(true);
tft.pushColors(data, len, false);  // Swap via setSwapBytes
```

**Resultado**: Esperado ✅ Cores corretas

**Implementado em**: `my_disp_flush()` linha 59

---

### Opção 3: ✅ Swap no pushColors (ALTERNATIVA)
```cpp
tft.setSwapBytes(false);
tft.pushColors(data, len, true);  // Swap apenas no pushColors
```

**Resultado**: Esperado ✅ Cores corretas

**Usar se Opção 2 falhar!**

---

## 🧪 Como Testar

### 1. Teste RGB (fillScreen)
```cpp
tft.fillScreen(TFT_RED);   // Vermelho
tft.fillScreen(TFT_GREEN); // Verde
tft.fillScreen(TFT_BLUE);  // Azul
```

Usa `setSwapBytes()`, não `pushColors()`.

### 2. UI LVGL (pushColors)
```cpp
lv_obj_set_style_bg_color(ui_Olho, lv_color_hex(0x2095F6), ...);
//                                              ^^^^^^^^
//                                            AZUL #2095F6
```

Usa `my_disp_flush()` → `pushColors()`.

---

## 🔧 Configuração Atual (Opção 2)

### `src/display/main.cpp`

```cpp
void setup() {
  // ...
  tft.begin();
  tft.setSwapBytes(true);  // ← Swap global ON
  // ...
}

void my_disp_flush(...) {
  // ...
  tft.pushColors(data, len, false);  // ← Swap OFF (usa global)
  // ...
}
```

---

## 📝 Se Opção 2 Falhar

### Trocar para Opção 3:

**1. Setup:**
```cpp
void setup() {
  tft.begin();
  tft.setSwapBytes(false);  // ← Swap global OFF
}
```

**2. my_disp_flush:**
```cpp
void my_disp_flush(...) {
  tft.pushColors(data, len, true);  // ← Swap ON (local)
}
```

---

## 🎨 Verificação Visual

### Olho Azul (#2095F6)

| RGB Hex | Cores | Aparência Correta | Aparência Trocada |
|---------|-------|-------------------|-------------------|
| `0x2095F6` | R=32, G=149, B=246 | AZUL intenso | LARANJA/VERMELHO |

**Se aparece LARANJA** = Swap ainda errado!  
**Se aparece AZUL** = Swap correto! ✅

---

## 🔍 Debug Adicional

### Teste Manual de Cores LVGL

Adicionar no `createQRCodeOverlay()`:

```cpp
void createQRCodeOverlay() {
  // ... código existente ...
  
  // TESTE: Criar label azul
  lv_obj_t *test_label = lv_obj_create(ui_Screen1);
  lv_obj_set_size(test_label, 100, 50);
  lv_obj_align(test_label, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_set_style_bg_color(test_label, lv_color_hex(0x0000FF), 0);  // AZUL puro
  lv_obj_set_style_bg_opa(test_label, 255, 0);
  
  Serial.println("Teste: Label azul (0x0000FF) criado em TOP_LEFT");
}
```

**Verificar**:
- ✅ Se aparece AZUL → Swap OK
- ❌ Se aparece VERMELHO → Swap invertido

---

## 📚 Referência TFT_eSPI

### pushColors() Signature
```cpp
void pushColors(uint16_t *data, uint32_t len, bool swap);
//                                                  ^^^^
//                                            Swap independente!
```

### setSwapBytes() Behavior
```cpp
void setSwapBytes(bool swap);
// Afeta: drawPixel, fillRect, drawLine, etc.
// NÃO afeta: pushColors quando swap é explícito
```

---

## ✅ Checklist Debug

Configuração Atual (Opção 2):
- ✅ `tft.setSwapBytes(true)` em setup()
- ✅ `pushColors(..., false)` em my_disp_flush()
- ⏳ Testando...

Se falhar, Opção 3:
- ⬜ `tft.setSwapBytes(false)` em setup()
- ⬜ `pushColors(..., true)` em my_disp_flush()
- ⬜ Testar novamente

---

## 🎊 Resultado Esperado

```
Display CYD:
┌──────────────────────────────┐
│         ⚙️ Spinner          │
│        ( 👁️ ) AZUL!         │  ← #2095F6 AZUL
│         ⚙️  (não laranja)   │
│                              │
│  [Label teste azul]          │  ← 0x0000FF AZUL
│  ┌────────────────┐          │
│  │   QR Code      │          │
│  │ Borda AZUL     │          │  ← #2095F6 AZUL
│  └────────────────┘          │
└──────────────────────────────┘
```

**TODAS as cores azuis devem aparecer AZUIS, não laranjas/vermelhas!**

---

**Status**: Testando Opção 2  
**Próximo passo**: Se falhar → Opção 3  
**Data**: 15 Dezembro 2024

# 💰 Substituição do Símbolo "$" por Imagem do Baú de Tesouro

## 🎯 Mudança Implementada

### ANTES:
```cpp
// Desenhava símbolo "$" com fonte gigante
tft.setFreeFont(&FreeSansBold24pt7b);
tft.setTextSize(3);
tft.drawString("$", tft.width()/2, tft.height()/2);
```

### DEPOIS:
```cpp
// Exibe imagem do baú de tesouro (LVGL)
lv_obj_t * treasure_img = lv_img_create(lv_scr_act());
lv_img_set_src(treasure_img, &ui_img_bautesouro_png);
lv_obj_align(treasure_img, LV_ALIGN_CENTER, 0, 0);
lv_timer_handler();
delay(300);
lv_obj_del(treasure_img);
```

---

## 📦 Imagem Utilizada

### Arquivo:
- **Path**: `src/display/ui/images/ui_img_bautesouro_png.c`
- **Nome**: `ui_img_bautesouro_png`
- **Asset original**: `assets/BauTesouro.png`
- **Dimensões**: 240x224 pixels
- **Formato**: TRUE_COLOR_ALPHA (RGB com transparência)

### Declaração:
```c
LV_IMG_DECLARE(ui_img_bautesouro_png);
```

---

## 🔧 Código Completo

### 1. Include da UI do SquareLine Studio

```cpp
// UI do SquareLine Studio (imagens)
extern "C" {
  #include "ui/ui.h"
}
```

**Importante**: `extern "C"` porque o ui.h é C puro, mas o main.cpp é C++.

---

### 2. Exibição da Imagem

```cpp
void showTagInfo(const TagMessage& tag) {
  if (tag.type == CONTENT_URL) {
    
    // 1. Animação laugh
    roboEyes.anim_laugh();
    delay(500);
    
    // 2. Limpa tela
    tft.fillScreen(TFT_BLACK);
    
    // 3. Cria objeto de imagem LVGL
    lv_obj_t * treasure_img = lv_img_create(lv_scr_act());
    lv_img_set_src(treasure_img, &ui_img_bautesouro_png);
    lv_obj_set_width(treasure_img, LV_SIZE_CONTENT);
    lv_obj_set_height(treasure_img, LV_SIZE_CONTENT);
    lv_obj_align(treasure_img, LV_ALIGN_CENTER, 0, 0);
    
    // 4. Renderiza LVGL
    lv_timer_handler();
    
    // 5. Aguarda visualização
    delay(300);
    
    // 6. Remove imagem (libera memória)
    lv_obj_del(treasure_img);
    
    // 7. Exibe QR Code
    switchToQRCodeMode(tag.url);
  }
}
```

---

## 🎨 Funções LVGL Utilizadas

### `lv_img_create(parent)`
Cria um objeto de imagem como filho do `parent`.
```cpp
lv_obj_t * treasure_img = lv_img_create(lv_scr_act());
// lv_scr_act() = tela ativa atual
```

### `lv_img_set_src(img, src)`
Define a fonte da imagem (ponteiro para lv_img_dsc_t).
```cpp
lv_img_set_src(treasure_img, &ui_img_bautesouro_png);
```

### `lv_obj_set_width/height(obj, size)`
Define dimensões do objeto.
```cpp
lv_obj_set_width(treasure_img, LV_SIZE_CONTENT);
lv_obj_set_height(treasure_img, LV_SIZE_CONTENT);
// LV_SIZE_CONTENT = usa tamanho natural da imagem
```

### `lv_obj_align(obj, align, x_ofs, y_ofs)`
Alinha o objeto na tela.
```cpp
lv_obj_align(treasure_img, LV_ALIGN_CENTER, 0, 0);
// LV_ALIGN_CENTER = centralizado
// 0, 0 = sem offset X/Y
```

### `lv_timer_handler()`
Processa tarefas pendentes do LVGL (renderização).
```cpp
lv_timer_handler();
// Força renderização imediata
```

### `lv_obj_del(obj)`
Deleta objeto e libera memória.
```cpp
lv_obj_del(treasure_img);
// Importante: evita vazamento de memória
```

---

## 📊 Fluxo de Exibição

```
Tag NDEF URL detectada
       ↓
🎭 roboEyes.anim_laugh() (500ms)
       ↓
🖤 tft.fillScreen(TFT_BLACK)
       ↓
🎨 Cria treasure_img LVGL
       ↓
🖼️ Define src = ui_img_bautesouro_png
       ↓
📐 Alinha no centro
       ↓
⚡ lv_timer_handler() (renderiza)
       ↓
⏱️ delay(300ms) - Visualização
       ↓
🗑️ lv_obj_del(treasure_img) - Limpa
       ↓
📱 switchToQRCodeMode(url)
```

---

## 🎯 Vantagens da Imagem vs Símbolo "$"

| Aspecto | Símbolo "$" | Baú de Tesouro |
|---------|-------------|----------------|
| **Visual** | Texto simples | Imagem rica |
| **Cores** | 1 cor (dourado) | Múltiplas cores |
| **Detalhes** | Limitado | Alta qualidade |
| **Tema** | Genérico | Temático (pirata/tesouro) |
| **Tamanho** | Escalado (pixelado) | Vetorizado suave |
| **Impacto** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

---

## 📐 Especificações da Imagem

### Dimensões:
```c
.header.w = 240  // Largura (pixels)
.header.h = 224  // Altura (pixels)
```

### Formato:
```c
.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA
// TRUE_COLOR = RGB completo (16-bit ou 24-bit)
// ALPHA = Canal de transparência
```

### Tamanho em Memória:
```c
.data_size = sizeof(ui_img_bautesouro_png_data)
// Array de bytes com dados da imagem
```

---

## 🎨 Alinhamento

### Opções de Alinhamento:
```c
LV_ALIGN_CENTER           // Centro (usado)
LV_ALIGN_TOP_LEFT         // Topo esquerdo
LV_ALIGN_TOP_MID          // Topo centro
LV_ALIGN_TOP_RIGHT        // Topo direito
LV_ALIGN_BOTTOM_LEFT      // Base esquerdo
LV_ALIGN_BOTTOM_MID       // Base centro
LV_ALIGN_BOTTOM_RIGHT     // Base direito
LV_ALIGN_LEFT_MID         // Esquerda centro
LV_ALIGN_RIGHT_MID        // Direita centro
```

### Com Offset:
```cpp
lv_obj_align(treasure_img, LV_ALIGN_CENTER, 10, -20);
// 10px para direita, 20px para cima
```

---

## 🐛 Troubleshooting

### Imagem Não Aparece:
1. Verifique se `initLVGL()` foi chamado no `setup()`
2. Certifique-se que `lv_timer_handler()` é chamado
3. Confira se `ui/ui.h` está incluído corretamente

### Erro de Compilação:
```
error: 'ui_img_bautesouro_png' was not declared
```
**Solução**: Adicionar `extern "C" { #include "ui/ui.h" }`

### Imagem Cortada:
```cpp
// Use tamanho da imagem (não escale)
lv_obj_set_width(treasure_img, LV_SIZE_CONTENT);
lv_obj_set_height(treasure_img, LV_SIZE_CONTENT);
```

### Vazamento de Memória:
```cpp
// SEMPRE delete a imagem após usar
lv_obj_del(treasure_img);
```

---

## 💡 Melhorias Futuras

### 1. Animação de Entrada:
```cpp
// Fade in
lv_obj_set_style_opa(treasure_img, 0, 0);
lv_obj_fade_in(treasure_img, 200, 0);
```

### 2. Escala Dinâmica:
```cpp
// Zoom in
lv_obj_set_style_transform_zoom(treasure_img, 50, 0);
lv_anim_t a;
lv_anim_init(&a);
lv_anim_set_var(&a, treasure_img);
// ... configurar animação de zoom
```

### 3. Rotação:
```cpp
lv_obj_set_style_transform_angle(treasure_img, 100, 0);
// 100 = 10° (unidade: 0.1°)
```

---

## 📺 Serial Monitor Esperado

```
📱 Tag detectada!
  ├─ UID: 04:XX:XX:XX:XX:XX:XX
  ├─ Tipo: URL NDEF
  ├─ URL: https://example.com/pay
  └─ Exibindo animação e símbolo $...
[Animação laugh 500ms]
[Baú de tesouro 300ms]
  └─ Exibindo QR Code...
📱 Alternando para modo QR Code...
```

---

## 🎯 Resultado Visual

```
┌────────────────────┐
│                    │
│                    │
│      ╔═══╗         │
│      ║💰 ║         │  ← Baú de tesouro
│      ║$$$║         │     colorido e detalhado
│      ╚═══╝         │     240x224px
│                    │     centralizado
│                    │
└────────────────────┘
```

**Duração**: 300ms  
**Qualidade**: Alta (imagem vetorial)  
**Impacto**: Muito maior que texto simples!

---

## ✅ Checklist de Implementação

- ✅ Include de `ui/ui.h` com `extern "C"`
- ✅ Declaração `LV_IMG_DECLARE(ui_img_bautesouro_png)`
- ✅ Criação do objeto com `lv_img_create()`
- ✅ Definição da fonte com `lv_img_set_src()`
- ✅ Dimensões com `LV_SIZE_CONTENT`
- ✅ Alinhamento centralizado
- ✅ Renderização com `lv_timer_handler()`
- ✅ Delay de 300ms para visualização
- ✅ Limpeza com `lv_obj_del()`
- ✅ Integração com sequência de tag

---

## 📦 Arquivos Modificados

1. **`src/display/main.cpp`**
   - Adicionado include `ui/ui.h`
   - Substituído código do "$" por imagem LVGL
   - Adicionado `lv_obj_del()` para limpeza

2. **Arquivos existentes (não modificados)**
   - `src/display/ui/ui.h` - Declaração da imagem
   - `src/display/ui/images/ui_img_bautesouro_png.c` - Dados da imagem

---

**Status**: ✅ **IMAGEM DO BAÚ DE TESOURO IMPLEMENTADA**  
**Data**: 16 de Dezembro de 2024  
**Substituição**: Símbolo "$" → Imagem PNG do baú  
**Formato**: LVGL TRUE_COLOR_ALPHA  
**Dimensões**: 240x224 pixels  
**Muito mais visual e temático!** 💰🎨✨

# 🎨 Integração com SquareLine Studio

## 📋 Instruções

Esta pasta é destinada aos arquivos gerados pelo **SquareLine Studio** para a interface LVGL do display CYD.

---

## 🚀 Como Usar SquareLine Studio

### 1. Criar Projeto no SquareLine

1. Abra **SquareLine Studio**
2. Crie novo projeto:
   - **Board**: ESP32
   - **Display**: 320x240 (ILI9341)
   - **Color Depth**: 16-bit
   - **LVGL Version**: 8.x

### 2. Configurações do Projeto

```
Project Settings:
├─ Board: ESP32
├─ Display: 320x240
├─ Rotation: Landscape (0°)
├─ Color Depth: 16-bit RGB565
├─ LVGL Version: 8.3.x ou 8.4.x
└─ Export Path: display/ui/
```

### 3. Design da Interface

Crie as seguintes screens:

#### **Screen: Main (Standby)**
- Label: "RFID Reader System" (título)
- Label: Status (aguardando/pronto/erro)
- Spinner ou animação de espera

#### **Screen: TagDetected**
- Label: UID da tag
- Label: URL ou Texto
- QR Code widget (se URL)
- Botão: "Limpar" ou auto-clear

### 4. Exportar Código

1. No SquareLine: **Export** → **Export UI Files**
2. Caminho de destino: `display/ui/`
3. Arquivos gerados:
   ```
   display/ui/
   ├── ui.c
   ├── ui.h
   ├── ui_events.c
   ├── ui_events.h
   ├── screens/
   │   ├── ui_MainScreen.c
   │   └── ui_TagDetectedScreen.c
   └── components/
       └── (componentes customizados)
   ```

---

## 🔧 Integração no Código

### Passo 1: Incluir Headers

No arquivo `display/src/main.cpp`, substitua:

```cpp
// Remover UI temporária:
// createTemporaryUI();

// Adicionar:
#include "../ui/ui.h"
```

### Passo 2: Inicializar UI

No `setup()`, substitua:

```cpp
void setup() {
  // ... inicialização anterior ...
  
  initLVGL();
  
  // UI do SquareLine Studio
  ui_init();  // ← Chamada gerada automaticamente
  
  // ... resto do código ...
}
```

### Passo 3: Atualizar Widgets

Acesse os widgets gerados pelo SquareLine:

```cpp
void showTagInfo(const TagMessage& tag) {
  // Widgets do SquareLine (exemplo)
  lv_label_set_text(ui_LabelUID, tag.uid.c_str());
  lv_label_set_text(ui_LabelURL, tag.url.c_str());
  
  if (tag.type == CONTENT_URL) {
    lv_qrcode_update(ui_QRCode, tag.url.c_str(), tag.url.length());
    lv_scr_load(ui_TagDetectedScreen);  // Muda para tela de tag
  }
}
```

---

## 📦 Exemplo de Estrutura Final

```
display/
├── src/
│   └── main.cpp                 # Código principal (modificado)
├── ui/                          # ← Arquivos do SquareLine
│   ├── ui.c
│   ├── ui.h
│   ├── ui_events.c
│   ├── ui_events.h
│   ├── README_SQUARELINE.md     # Este arquivo
│   ├── screens/
│   │   ├── ui_MainScreen.c
│   │   └── ui_TagDetectedScreen.c
│   └── components/
│       └── ui_QRCodePanel.c
└── include/
    └── (headers customizados)
```

---

## 🎨 Widgets Recomendados

### MainScreen
- `ui_LabelTitle` - Título do app
- `ui_LabelStatus` - Status da conexão
- `ui_SpinnerWait` - Animação de espera
- `ui_PanelInfo` - Painel de informações

### TagDetectedScreen
- `ui_LabelUID` - Mostra UID da tag
- `ui_LabelURL` - Mostra URL detectada
- `ui_QRCode` - Widget QR Code
- `ui_ButtonClear` - Botão para limpar
- `ui_LabelType` - Tipo de conteúdo (URL/Texto/Bruto)

---

## 🔗 Eventos do SquareLine

### Criar Eventos Customizados

No SquareLine, adicione eventos:

1. **ButtonClear** → `onClick` → `clearDisplayEvent()`
2. **Screen Load** → `onLoad` → `screenLoadedEvent()`

Em `display/src/main.cpp`:

```cpp
// Implementar callbacks
void clearDisplayEvent(lv_event_t * e) {
  clearDisplay();
  lv_scr_load(ui_MainScreen);
}

void screenLoadedEvent(lv_event_t * e) {
  Serial.println("Screen carregada!");
}
```

---

## ⚠️ Importante

1. **Não edite arquivos em `ui/` manualmente**
   - Sempre edite no SquareLine Studio e re-exporte

2. **Versionamento Git**
   - Commite os arquivos `ui/*.c` e `ui/*.h`
   - Ignore binários/temporários do SquareLine

3. **Compatibilidade LVGL**
   - Use LVGL 8.3 ou 8.4 (compatível com TFT_eSPI)
   - Evite widgets que requerem LVGL 9.x

---

## 📚 Referências

- [SquareLine Studio Docs](https://docs.squareline.io/)
- [LVGL Documentation](https://docs.lvgl.io/8.3/)
- [TFT_eSPI Library](https://github.com/Bodmer/TFT_eSPI)

---

## ✅ Checklist de Integração

- [ ] Criar projeto no SquareLine Studio
- [ ] Configurar 320x240, 16-bit, ESP32
- [ ] Desenhar screens (Main, TagDetected)
- [ ] Adicionar widgets (Labels, QR Code, Buttons)
- [ ] Exportar para `display/ui/`
- [ ] Incluir `#include "../ui/ui.h"` no main.cpp
- [ ] Substituir `createTemporaryUI()` por `ui_init()`
- [ ] Testar compilação
- [ ] Testar no hardware

---

**Versão**: 1.0  
**Data**: Dezembro 2024  
**Status**: 📝 Template Pronto

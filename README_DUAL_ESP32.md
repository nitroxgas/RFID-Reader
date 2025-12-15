# 🎯 RFID Reader - Arquitetura Dual ESP32

Sistema modular de leitura RFID com display CYD separado.

---

## 📂 Estrutura do Projeto

```
RFID Reader/
├── src/                          # Pasta padrão PlatformIO
│   ├── reader/                   # 📱 ESP32-WROOM (RFID Reader)
│   │   └── main.cpp              # ✅ Código com UART implementado
│   ├── display/                  # 🖥️ ESP32-2432S028R (CYD Display)
│   │   └── main.cpp              # ✅ Código base com LVGL
│   └── common/                   # 🔗 Código compartilhado
│       └── protocol.h            # Protocolo UART
│
├── include/
│   └── lv_conf.h                 # Configuração LVGL
│
├── display/
│   └── ui/                       # 🎨 SquareLine Studio (opcional)
│       └── README_SQUARELINE.md  # Guia de integração
│
├── platformio.ini                # ⚙️ 3 ambientes configurados
├── HOW_TO_COMPILE.md             # 📖 Guia de compilação
├── PLATFORMIO_FIX.md             # 🔧 Solução build_src_filter
└── README.md                     # Este arquivo
```

---

## 🚀 Quick Start

### 1. Compilar Reader (ESP32-WROOM)

```bash
# Via VS Code
Selecione ambiente: reader-wroom → Upload

# Via Terminal
pio run -e reader-wroom --target upload
```

### 2. Compilar Display (CYD)

```bash
# Via VS Code
Selecione ambiente: display-cyd → Upload

# Via Terminal
pio run -e display-cyd --target upload
```

### 3. Conectar Hardware

```
Reader (ESP32-WROOM)  ←→  Display (CYD)

GPIO 17 (TX)  ────────→  GPIO 27 (CN1 Pin 3)
GPIO 16 (RX)  ←────────  GPIO 22 (CN1 Pin 2)
GND          ─────────→  GND (CN1 Pin 1)
```

### 4. Testar

1. Alimente ambos via USB
2. Aproxime tag NTAG no Reader
3. Veja informação no Display CYD

---

## 🔧 Ambientes PlatformIO

| Ambiente | Placa | Função | Arquivo |
|----------|-------|--------|---------|
| **reader-wroom** | ESP32-WROOM | RFID Reader | `reader/src/main.cpp` |
| **display-cyd** | ESP32-2432S028R | Display LVGL | `display/src/main.cpp` |
| **reader-s3lcd** | ESP32-S3-LCD | Legacy (Waveshare) | `reader/src/main.cpp` |

---

## 📡 Protocolo UART

### Formato
```
TAG|UID|URL|TEXT|TYPE\n
```

### Exemplo
```
TAG|04A1B2C3D4E5F6|https://example.com||1
```

### Tipos
- `0` = Dados brutos
- `1` = URL NDEF
- `2` = Texto NDEF

---

## ✨ Features Implementadas

### Reader (`reader/src/main.cpp`)
- ✅ Leitura MFRC522 (NTAG213/215)
- ✅ Parser NDEF completo (35 prefixos URI)
- ✅ Comunicação UART (Serial1)
- ✅ Protocolo estruturado
- ✅ Debug via Serial0

### Display (`display/src/main.cpp`)
- ✅ TFT_eSPI (ILI9341)
- ✅ LVGL 8.4
- ✅ UI temporária funcional
- ✅ QR Code para URLs
- ✅ Recepção UART
- ✅ Preparado para SquareLine Studio

### Compartilhado (`common/protocol.h`)
- ✅ Protocolo UART
- ✅ Funções encode/decode
- ✅ Structs compartilhadas

---

## 🎨 Integração SquareLine Studio

1. **Criar projeto SquareLine**
   - Display: 320x240
   - Color Depth: 16-bit
   - LVGL: 8.4

2. **Exportar para**
   ```
   display/ui/
   ```

3. **Substituir no código**
   ```cpp
   // Remover:
   createTemporaryUI();
   
   // Adicionar:
   #include "../ui/ui.h"
   ui_init();
   ```

Ver: `display/ui/README_SQUARELINE.md`

---

## 📖 Documentação Completa

| Arquivo | Conteúdo |
|---------|----------|
| `HOW_TO_COMPILE.md` | Guia detalhado de compilação |
| `UART_IMPLEMENTATION_SUMMARY.md` | Resumo implementação UART |
| `DUAL_ESP32_PROJECT_STRATEGY.md` | Estratégia e templates |
| `CYD_DUAL_ESP32_ARCHITECTURE.md` | Arquitetura completa |
| `display/ui/README_SQUARELINE.md` | Guia SquareLine Studio |

---

## 🔍 Troubleshooting

### Erro: "Multiple definitions of setup()"
**Solução**: Verificar `src_dir` no `platformio.ini`

### Display não mostra nada
**Checklist**:
- [ ] LVGL compilou sem erros?
- [ ] `lv_timer_handler()` está no loop?
- [ ] Display alimentado (5V)?

### UART não funciona
**Checklist**:
- [ ] Conexões corretas? (TX↔RX cruzados)
- [ ] GND comum conectado?
- [ ] Ambos em 115200 baud?

Ver: `HOW_TO_COMPILE.md` → Troubleshooting

---

## 🎯 Próximos Passos

### Desenvolvimento
- [ ] Criar UI no SquareLine Studio
- [ ] Exportar para `display/ui/`
- [ ] Integrar com `ui_init()`
- [ ] Adicionar animações
- [ ] Testar comunicação completa

### Melhorias
- [ ] Adicionar touch screen
- [ ] Múltiplos screens LVGL
- [ ] Histórico de tags
- [ ] Configurações via display
- [ ] Log de erros

---

## 📊 Exemplo de Saída

### Reader Serial
```
╔══════════════════════════════════════════╗
║  Leitor RFID - ESP32-WROOM            ║
╚══════════════════════════════════════════╝

🔗 Comunicação UART habilitada:
   TX: GPIO17, RX: GPIO16

========================================
         NOVA TAG DETECTADA!
========================================
UID da tag: 04A1B2C3D4E5F6

========================================
🌐 URL DETECTADA (NDEF)
========================================
https://www.google.com
========================================

📤 Enviado para display: TAG|04A1B2C3D4E5F6|https://www.google.com||1
```

### Display Serial
```
╔══════════════════════════════════════════╗
║   ESP32-2432S028R (CYD) Display         ║
╚══════════════════════════════════════════╝

🔗 Inicializando UART...
📺 Inicializando TFT Display...
✅ Sistema pronto!

📩 UART << TAG|04A1B2C3D4E5F6|https://www.google.com||1
📱 Atualizando display com informações da tag...
  ├─ URL: https://www.google.com
  └─ QR Code gerado
```

---

## 🛠️ Comandos Rápidos

```bash
# Compilar Reader
pio run -e reader-wroom --target upload

# Compilar Display
pio run -e display-cyd --target upload

# Monitor Reader
pio device monitor -e reader-wroom

# Monitor Display
pio device monitor -e display-cyd

# Compilar tudo
pio run
```

---

## 💡 Dicas

1. **Use dois terminais**: Um para cada Serial Monitor
2. **VS Code**: Alterne ambientes na barra inferior
3. **Debug**: Serial0 em ambos para diagnóstico
4. **SquareLine**: Mantenha projeto separado, exporte para `display/ui/`

---

## ✅ Status

- ✅ Reader com UART: **IMPLEMENTADO**
- ✅ Display base: **IMPLEMENTADO**
- ✅ Protocolo UART: **DEFINIDO**
- ✅ PlatformIO dual: **CONFIGURADO**
- ⏳ SquareLine UI: **PENDENTE** (opcional)
- ⏳ Testes hardware: **PENDENTE**

---

**Versão**: 1.0  
**Data**: Dezembro 2024  
**Licença**: MIT

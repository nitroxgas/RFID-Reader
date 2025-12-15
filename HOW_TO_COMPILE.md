# 🔧 Como Compilar e Gravar - Dual ESP32

## 📂 Estrutura do Projeto

```
RFID Reader/
├── reader/                    # Código do ESP32-WROOM (RFID Reader)
│   └── src/
│       └── main.cpp
├── display/                   # Código do ESP32-2432S028R (CYD Display)
│   ├── src/
│   │   └── main.cpp
│   └── ui/                    # Código LVGL do SquareLine Studio
│       └── (arquivos .c/.h)
├── common/                    # Código compartilhado
│   └── protocol.h
└── platformio.ini             # Configuração com 3 ambientes
```

---

## 🎯 Ambientes Disponíveis

| Ambiente | Placa | Função | Código |
|----------|-------|--------|--------|
| **reader-wroom** | ESP32-WROOM | Lê RFID + UART | `reader/src/` |
| **display-cyd** | ESP32-2432S028R | Display LVGL | `display/src/` |
| **reader-s3lcd** | ESP32-S3-LCD | Legacy (Waveshare) | `reader/src/` |

---

## 💻 Método 1: VS Code + PlatformIO Extension

### Selecionando o Ambiente

1. **Barra Inferior do VS Code**
   - Clique em "Default (reader-wroom)" na barra inferior
   - Selecione o ambiente desejado:
     - `reader-wroom` → Compilar Reader
     - `display-cyd` → Compilar Display

2. **Sidebar PlatformIO**
   - Abra sidebar do PlatformIO (ícone alienígena 👽)
   - Expanda o ambiente desejado
   - Clique em **Upload** ou **Upload and Monitor**

### Compilar Reader (ESP32-WROOM)

```
1. Selecione ambiente: reader-wroom
2. Conecte ESP32-WROOM via USB
3. PlatformIO → reader-wroom → Upload
4. Monitor Serial: 115200 baud
```

### Compilar Display (CYD)

```
1. Selecione ambiente: display-cyd
2. Conecte CYD via USB
3. PlatformIO → display-cyd → Upload
4. Monitor Serial: 115200 baud
```

---

## 🖥️ Método 2: Terminal / CLI

### Compilar Reader (ESP32-WROOM)

```bash
# Compilar
pio run -e reader-wroom

# Compilar e gravar
pio run -e reader-wroom --target upload

# Monitor serial
pio device monitor -e reader-wroom

# Compilar + Gravar + Monitor
pio run -e reader-wroom --target upload && pio device monitor -e reader-wroom
```

### Compilar Display (CYD)

```bash
# Compilar
pio run -e display-cyd

# Compilar e gravar
pio run -e display-cyd --target upload

# Monitor serial
pio device monitor -e display-cyd

# Compilar + Gravar + Monitor
pio run -e display-cyd --target upload && pio device monitor -e display-cyd
```

### Compilar Ambos

```bash
# Compila todos os ambientes
pio run

# Limpar build de todos
pio run --target clean
```

---

## 🔍 Verificar Compilação

### Verificar sem Gravar

```bash
# Reader
pio run -e reader-wroom --target size

# Display
pio run -e display-cyd --target size
```

### Ver Portas Seriais Disponíveis

```bash
pio device list
```

---

## ⚙️ Configurar Porta Serial

### Método 1: Editar platformio.ini

Descomente e ajuste:

```ini
[env:reader-wroom]
upload_port = COM4      ; Windows
; upload_port = /dev/ttyUSB0  ; Linux
; upload_port = /dev/cu.usbserial-*  ; macOS
monitor_port = COM4
```

### Método 2: Via Linha de Comando

```bash
# Gravar em porta específica
pio run -e reader-wroom --upload-port COM4 --target upload
```

---

## 🐛 Troubleshooting

### Erro: "Please setup environments in platformio.ini"

**Causa**: PlatformIO não reconheceu os ambientes.

**Solução**:
1. Feche e reabra VS Code
2. Execute: `pio run` no terminal
3. Recarregue janela: Ctrl+Shift+P → "Developer: Reload Window"

### Erro: "Multiple definitions of setup()"

**Causa**: PlatformIO está compilando ambos os códigos.

**Solução**: Verificar `src_dir` e `build_src_filter` no `platformio.ini`:

```ini
[env:reader-wroom]
src_dir = reader/src
build_src_filter = 
    +<*>
    -<../../display/>  ; Exclui pasta display
```

### Erro: "Cannot find protocol.h"

**Causa**: Caminho para `common/` não configurado.

**Solução**: Verificar `build_flags`:

```ini
build_flags = 
    -I common
```

### Erro ao gravar: "Failed to connect"

**Solução**:
1. Segure botão **BOOT** no ESP32
2. Clique **Upload**
3. Solte **BOOT** quando começar a gravar

### Display não mostra nada

**Checklist**:
- [ ] Código LVGL compilou sem erros?
- [ ] TFT_eSPI configurado corretamente?
- [ ] `lv_timer_handler()` está no loop?
- [ ] Display está alimentado (5V)?

---

## 🔗 Workflow Completo

### 1. Desenvolver Reader

```bash
# Terminal 1: Monitor Reader
pio device monitor -e reader-wroom

# Terminal 2: Compilar e gravar
pio run -e reader-wroom --target upload
```

### 2. Desenvolver Display

```bash
# Terminal 1: Monitor Display
pio device monitor -e display-cyd

# Terminal 2: Compilar e gravar
pio run -e display-cyd --target upload
```

### 3. Testar Sistema Completo

1. Grave código no Reader
2. Grave código no Display
3. Conecte UART entre ambos:
   ```
   Reader GPIO17 (TX) → Display GPIO27 (RX)
   Reader GPIO16 (RX) → Display GPIO22 (TX)
   Reader GND → Display GND
   ```
4. Alimente ambos
5. Aproxime tag NTAG no Reader
6. Veja informação no Display CYD

---

## 📊 Exemplo de Saída

### Reader (COM4)
```
╔══════════════════════════════════════════╗
║  Leitor RFID - ESP32-WROOM            ║
║         MFRC522 + NTAG213/215         ║
╚══════════════════════════════════════════╝

🔗 Comunicação UART habilitada:
   TX: GPIO17
   RX: GPIO16

✅ Sistema pronto!
⏳ Aguardando tags NFC...

========================================
         NOVA TAG DETECTADA!
========================================
UID da tag: 04A1B2C3D4E5F6
...

📤 Enviado para display: TAG|04A1B2C3D4E5F6|https://example.com||1
```

### Display (COM5)
```
╔══════════════════════════════════════════╗
║   ESP32-2432S028R (CYD) Display         ║
║   RFID Reader System                     ║
╚══════════════════════════════════════════╝

🔗 Inicializando UART (TX: GPIO22, RX: GPIO27)...
📤 UART >> STATUS|DISPLAY_READY
📺 Inicializando TFT Display...
✅ Sistema pronto!

📩 UART << STATUS|READER_READY
📩 UART << TAG|04A1B2C3D4E5F6|https://example.com||1
📱 Atualizando display com informações da tag...
```

---

## 🎯 Comandos Rápidos

```bash
# Compilar tudo
pio run

# Limpar tudo
pio run --target clean

# Recompilar Reader
pio run -e reader-wroom --target clean
pio run -e reader-wroom --target upload

# Recompilar Display
pio run -e display-cyd --target clean
pio run -e display-cyd --target upload

# Listar ambientes
pio project config

# Verificar configuração
pio project metadata
```

---

## ✅ Checklist Pré-Compilação

### Reader
- [ ] Código em `reader/src/main.cpp`
- [ ] Pinos MFRC522 corretos (GPIO 5, 18, 19, 22, 23)
- [ ] Pinos UART corretos (GPIO 16, 17)
- [ ] Biblioteca MFRC522 instalada

### Display
- [ ] Código em `display/src/main.cpp`
- [ ] Pinos TFT corretos (ver platformio.ini)
- [ ] Pinos UART corretos (GPIO 22, 27)
- [ ] Bibliotecas TFT_eSPI e LVGL instaladas
- [ ] Arquivos UI do SquareLine em `display/ui/` (opcional)

---

**Versão**: 1.0  
**Data**: Dezembro 2024  
**Status**: ✅ Pronto para Uso

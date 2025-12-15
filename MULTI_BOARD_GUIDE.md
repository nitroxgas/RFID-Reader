# 🎯 Guia Multi-Board - RFID Reader

## 📋 Ambientes Disponíveis

Este projeto agora suporta **múltiplas placas ESP32** com configuração automática de pinagem.

---

## 🔧 Ambientes Configurados

### 1. **esp32s3-lcd** - ESP32-S3-Touch-LCD-2.1 (Waveshare)

**Pinagem**:
```cpp
SS_PIN   = GPIO3  (Solda no Pad 6)
SCK_PIN  = GPIO4  (Solda no Pad 7)
MOSI_PIN = GPIO5  (Solda no Pad 8)
MISO_PIN = GPIO42 (SD_MISO)
RST_PIN  = GPIO0  (J9 Pin 12 + pull-up 10kΩ)
```

**Características**:
- ✅ Display LCD 480×480 touchscreen
- ⚠️ Requer soldagem em GPIO3, 4, 5
- ⚠️ GPIO0 requer pull-up 10kΩ obrigatório
- ❌ SD Card desabilitado (GPIO42 em uso)
- ❌ Leitura de bateria desabilitada (GPIO4 em uso)

**Quando usar**:
- Projeto final integrado com display
- Prototipagem avançada
- Você tem habilidade de soldagem SMD

---

### 2. **esp32-wroom** - ESP32-WROOM Genérico

**Pinagem (VSPI padrão)**:
```cpp
SS_PIN   = GPIO5  (padrão VSPI)
SCK_PIN  = GPIO18 (padrão VSPI)
MOSI_PIN = GPIO23 (padrão VSPI)
MISO_PIN = GPIO19 (padrão VSPI)
RST_PIN  = GPIO22 (livre)
```

**Características**:
- ✅ Pinagem padrão (fácil de conectar)
- ✅ Sem soldagem necessária
- ✅ Ideal para testes iniciais
- ✅ Jumpers diretos nos pinos
- ✅ Sem restrições de uso

**Quando usar**:
- Testes iniciais do MFRC522
- Prototipagem rápida
- Desenvolvimento sem display
- Verificação de funcionalidade

---

## 🚀 Como Compilar para Cada Ambiente

### Usando PlatformIO CLI:

#### Para ESP32-S3-Touch-LCD-2.1:
```bash
# Compilar
pio run -e esp32s3-lcd

# Upload
pio run -e esp32s3-lcd --target upload

# Monitor Serial
pio device monitor -e esp32s3-lcd
```

#### Para ESP32-WROOM:
```bash
# Compilar
pio run -e esp32-wroom

# Upload
pio run -e esp32-wroom --target upload

# Monitor Serial
pio device monitor -e esp32-wroom
```

### Usando VS Code com PlatformIO:

1. Clique no ícone **PlatformIO** na barra lateral
2. Em **Project Tasks**, expanda o ambiente desejado:
   - `esp32s3-lcd` ou
   - `esp32-wroom`
3. Clique em:
   - **Build** (para compilar)
   - **Upload** (para gravar)
   - **Monitor** (para monitor serial)

### Ambiente Padrão:

Para definir qual ambiente compilar por padrão:

```bash
# Definir esp32s3-lcd como padrão
pio run -e esp32s3-lcd

# Ou definir esp32-wroom como padrão
pio run -e esp32-wroom
```

Ou edite `platformio.ini` e adicione:
```ini
[platformio]
default_envs = esp32-wroom  ; ou esp32s3-lcd
```

---

## 🔌 Diagrama de Conexão

### ESP32-WROOM (Simples)

```
MFRC522          ESP32-WROOM
┌──────────┐     ┌─────────────┐
│ SDA (SS) ├─────┤ GPIO5       │
│ SCK      ├─────┤ GPIO18      │
│ MOSI     ├─────┤ GPIO23      │
│ MISO     ├─────┤ GPIO19      │
│ RST      ├─────┤ GPIO22      │
│ 3.3V     ├─────┤ 3.3V        │
│ GND      ├─────┤ GND         │
└──────────┘     └─────────────┘

✅ Conexão direta com jumpers
✅ Sem soldagem necessária
```

### ESP32-S3-Touch-LCD-2.1 (Avançado)

```
MFRC522          ESP32-S3-Touch-LCD-2.1
┌──────────┐     ┌─────────────────────┐
│ SDA (SS) ├─────┤ GPIO3 (Pad 6) 🔴    │
│ SCK      ├─────┤ GPIO4 (Pad 7) 🔴    │
│ MOSI     ├─────┤ GPIO5 (Pad 8) 🔴    │
│ MISO     ├─────┤ GPIO42 (SD_MISO)    │
│ RST      ├──┬──┤ GPIO0 (J9 Pin 12)   │
│ 3.3V     ├──┼──┤ 3.3V (J9 Pin 6)     │
│ GND      ├──┼──┤ GND (J9 Pin 1)      │
└──────────┘  │  └─────────────────────┘
              │
           [10kΩ] ← Pull-up obrigatório
              │
            3.3V

🔴 Soldagem em pads necessária
⚠️ GPIO0 requer pull-up 10kΩ
```

---

## 📊 Comparação de Ambientes

| Característica | esp32-wroom | esp32s3-lcd |
|----------------|-------------|-------------|
| **Dificuldade** | 🟢 Fácil | 🔴 Difícil |
| **Soldagem** | ❌ Não | ✅ Sim (3 pinos) |
| **Pull-up externo** | ❌ Não | ✅ Sim (GPIO0) |
| **Display integrado** | ❌ Não | ✅ Sim |
| **SD Card** | ✅ Sim | ❌ Não |
| **Bateria** | ✅ Sim | ❌ Não |
| **Tempo montagem** | 5 min | 2-4 horas |
| **Ideal para** | Testes | Produto final |

---

## 🔄 Migração Entre Ambientes

### De esp32-wroom para esp32s3-lcd:

1. **Hardware**:
   - Remova jumpers do ESP32-WROOM
   - Solde GPIO3, 4, 5 no ESP32-S3
   - Adicione pull-up 10kΩ em GPIO0
   - Conecte GPIO42 (SD_MISO)

2. **Software**:
   ```bash
   # Simplesmente compile para o novo ambiente
   pio run -e esp32s3-lcd --target upload
   ```

3. **Nenhuma alteração de código necessária!** ✅

### De esp32s3-lcd para esp32-wroom:

1. **Hardware**:
   - Conecte jumpers conforme pinagem VSPI
   - Não precisa de pull-up adicional

2. **Software**:
   ```bash
   pio run -e esp32-wroom --target upload
   ```

---

## 🧪 Testando os Ambientes

### Script de Teste Rápido:

```bash
# Teste automático em ambos ambientes
echo "=== Testando ESP32-WROOM ==="
pio run -e esp32-wroom

echo ""
echo "=== Testando ESP32-S3-LCD ==="
pio run -e esp32s3-lcd

echo ""
echo "✅ Ambos ambientes compilados com sucesso!"
```

### Verificação do Ambiente Ativo:

Ao iniciar, o monitor serial mostrará:
```
╬══════════════════════════════════════════╬
║  Leitor RFID - ESP32-WROOM            ║  ← Nome do ambiente
║         MFRC522 + NTAG213/215         ║
╚══════════════════════════════════════════╝
```

Ou:
```
╬══════════════════════════════════════════╬
║  Leitor RFID - ESP32-S3-Touch-LCD-2.1 ║  ← Nome do ambiente
║         MFRC522 + NTAG213/215         ║
╚══════════════════════════════════════════╝
```

---

## ⚙️ Configurações Avançadas

### Adicionar Novo Ambiente:

Edite `platformio.ini`:

```ini
[env:meu-esp32]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

lib_deps = 
	miguelbalboa/MFRC522 @ ^1.4.12

build_flags = 
	-DCORE_DEBUG_LEVEL=3
	-DMY_CUSTOM_BOARD=1
```

E no `src/main.cpp`, adicione:

```cpp
#elif defined(MY_CUSTOM_BOARD)
  #define SS_PIN    21
  #define RST_PIN   22
  #define SCK_PIN   18
  #define MISO_PIN  19
  #define MOSI_PIN  23
  #define BOARD_NAME "Meu ESP32 Custom"
```

---

## 🐛 Troubleshooting Multi-Board

### Problema: "Placa não especificada!"

**Causa**: Build flag não foi definida corretamente.

**Solução**:
```bash
# Verifique se está usando -e para especificar o ambiente
pio run -e esp32-wroom
```

### Problema: Pinagem não corresponde

**Causa**: Ambiente errado selecionado.

**Solução**:
1. Verifique qual ambiente está ativo no VS Code
2. Use `pio run -e [ambiente]` explicitamente
3. Confira mensagem no monitor serial

### Problema: Erro de compilação em um ambiente

**Causa**: Dependências específicas do ambiente.

**Solução**:
```bash
# Limpe e recompile
pio run -e esp32-wroom --target clean
pio run -e esp32-wroom
```

---

## 📚 Referências

- **ESP32-WROOM Pinout**: [Espressif Docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/hw-reference/esp32/get-started-devkitc.html)
- **ESP32-S3 Pinout**: [Espressif Docs](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html)
- **PlatformIO Environments**: [PlatformIO Docs](https://docs.platformio.org/en/latest/projectconf/section_env.html)

---

## ✅ Checklist de Migração

### Para começar com ESP32-WROOM:
- [ ] Conecte MFRC522 com jumpers (GPIO5,18,19,22,23)
- [ ] Compile: `pio run -e esp32-wroom`
- [ ] Upload: `pio run -e esp32-wroom --target upload`
- [ ] Teste com tag NFC

### Para migrar para ESP32-S3-LCD:
- [ ] Solde GPIO3, 4, 5
- [ ] Adicione pull-up 10kΩ em GPIO0
- [ ] Conecte GPIO42
- [ ] Compile: `pio run -e esp32s3-lcd`
- [ ] Upload: `pio run -e esp32s3-lcd --target upload`
- [ ] Teste com tag NFC

---

**Versão**: 1.0  
**Data**: Dezembro 2024  
**Status**: ✅ Dual-board support ativo

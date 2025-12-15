# Leitor RFID para ESP32-S3-Touch-LCD-2.1

Projeto de leitura de tags NFC NTAG213 e NTAG215 usando o módulo MFRC522 com o display Waveshare ESP32-S3-Touch-LCD-2.1.

## 📋 Índice

- [Componentes Necessários](#componentes-necessários)
- [Recursos](#recursos)
- [Instalação](#instalação)
- [Configuração de Hardware](#configuração-de-hardware)
- [Como Usar](#como-usar)
- [Saída Serial Esperada](#saída-serial-esperada)
- [Troubleshooting](#troubleshooting)
- [Estrutura do Projeto](#estrutura-do-projeto)

## 🔧 Componentes Necessários

### Hardware

1. **Waveshare ESP32-S3-Touch-LCD-2.1**
   - Display LCD 2.1" touchscreen (480×480)
   - ESP32-S3R8 com 16MB Flash e 8MB PSRAM
   - [Link da documentação](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1)

2. **Módulo MFRC522**
   - Leitor RFID 13.56MHz
   - Compatível com NTAG213, NTAG215, MIFARE Classic, etc.

3. **Tags NFC**
   - NTAG213 (144 bytes de memória)
   - NTAG215 (504 bytes de memória)

4. **Cabos de conexão** (jumpers)

### Software

- **PlatformIO** (ou Arduino IDE)
- **Biblioteca MFRC522** v1.4.12
- **ESP32 Arduino Core** 3.0.x

## ✨ Recursos

- ✅ Leitura de UID de tags NFC
- ✅ Detecção automática de tipo de tag (NTAG213/215, MIFARE, etc.)
- ✅ **Leitura completa de TODOS os dados da tag** (164 bytes NTAG213 / 504 bytes NTAG215)
- ✅ **Parser NDEF completo**:
  - Detecção automática de URLs (http, https, tel, mailto, etc.)
  - Extração de textos NDEF
  - Suporte a 35 prefixos URI (NFC Forum compliant)
- ✅ **Saída simplificada e limpa**:
  - Informações da tag (UID, tipo, subtipo)
  - Estatísticas resumidas (bytes, tipo de conteúdo)
  - URL/Texto extraído em destaque
  - Sem dados hexadecimais ou strings brutas
- ✅ Debounce para evitar leituras duplicadas
- ✅ Diagnóstico de erros de comunicação SPI
- ✅ Monitor serial detalhado (115200 baud)
- ✅ Suporte multi-board (ESP32-WROOM e ESP32-S3-LCD)

## 🎯 Suporte Multi-Board

Este projeto suporta **duas placas** com configuração automática de pinagem:

| Ambiente | Placa | Pinagem | Dificuldade |
|----------|-------|---------|-------------|
| **esp32s3-lcd** | ESP32-S3-Touch-LCD-2.1 | GPIO3,4,5,42,0 | 🔴 Alta (requer soldagem) |
| **esp32-wroom** | ESP32-WROOM Genérico | GPIO5,18,19,22,23 | 🟢 Fácil (jumpers) |

**📖 Guia completo**: [MULTI_BOARD_GUIDE.md](MULTI_BOARD_GUIDE.md)

---

## 📦 Instalação

### Usando PlatformIO

1. Clone ou baixe este projeto
2. Abra a pasta do projeto no VS Code com PlatformIO
3. **Selecione o ambiente desejado**:
   - `esp32-wroom` para testes iniciais (recomendado)
   - `esp32s3-lcd` para display Waveshare

```bash
cd "RFID Reader"

# Para ESP32-WROOM (fácil)
pio run -e esp32-wroom

# Para ESP32-S3-LCD (avançado)
pio run -e esp32s3-lcd
```

### Usando Arduino IDE

1. Instale a biblioteca **MFRC522** pelo Library Manager
2. Configure a placa: 
   - **Board**: "ESP32S3 Dev Module"
   - **USB CDC On Boot**: "Enabled"
   - **Flash Size**: "16MB"
   - **PSRAM**: "OPI PSRAM"
3. Compile e faça upload

## 🔌 Configuração de Hardware

### ⚠️ IMPORTANTE: Soldagem Necessária

Após **análise completa da pinagem** do ESP32-S3-Touch-LCD-2.1:
- ❌ **Display RGB ocupa 26 GPIOs** (IO1-48)
- ❌ **I2C ocupa 2 GPIOs** (Touch, IMU, RTC)
- ❌ **SD Card compartilha pinos com LCD** (IO1, IO2 já ocupados)
- ✅ **Apenas 3 GPIOs livres** nos conectores: GPIO0, GPIO4, GPIO42

**Solução**: Combinação de soldagem + GPIOs disponíveis + strapping pin.

### Pinagem Final Adotada

| MFRC522 | ESP32-S3 | Localização              | Método |
|---------|----------|---------------------------|--------|
| SDA     | GPIO3    | Pad 6 do ESP32-S3         | 🔴 Solda |
| SCK     | GPIO4    | Pad 7 do ESP32-S3         | 🔴 Solda |
| MOSI    | GPIO5    | Pad 8 do ESP32-S3         | 🔴 Solda |
| MISO    | GPIO42   | SD_MISO (desabilitar SD)  | ✅ Acessível |
| RST     | GPIO0    | J9 Pin 12 + pull-up 10kΩ | ⚠️ Strapping |
| 3.3V    | 3.3V     | J9 Pin 6                  | ✅ Conector |
| GND     | GND      | J9 Pin 1                  | ✅ Conector |

⚠️ **AVISOS CRÍTICOS**:
- O MFRC522 opera em **3.3V**. Não use 5V!
- **GPIO0 requer resistor pull-up 10kΩ para 3.3V** (strapping pin)
- **SD Card será desabilitado** (GPIO42 em uso)
- **Soldagem de precisão necessária** em GPIO3, 4, 5

📖 **Documentação Completa**:
- **[NDEF_PROTOCOL.md](NDEF_PROTOCOL.md)** - 📚 Parser NDEF e protocolo (NOVO!)
- **[MULTI_BOARD_GUIDE.md](MULTI_BOARD_GUIDE.md)** - 🎯 Guia multi-board
- **[TAG_READING_OUTPUT.md](TAG_READING_OUTPUT.md)** - 📊 Exemplo de leitura completa
- **[SOLUTION_FINAL.md](SOLUTION_FINAL.md)** - ⭐ Guia ESP32-S3-LCD completo
- **[GPIO_ANALYSIS_FULL.md](GPIO_ANALYSIS_FULL.md)** - Análise técnica detalhada
- [CONNECTOR_PINOUT_REAL.md](CONNECTOR_PINOUT_REAL.md) - Pinagem dos conectores
- [WIRING_DIAGRAM.md](WIRING_DIAGRAM.md) - Diagrama de conexão (legado)

## 🚀 Como Usar

### 1. Upload do Código

#### Para ESP32-WROOM (recomendado para iniciantes):
```bash
# PlatformIO
pio run -e esp32-wroom --target upload

# Monitor serial
pio device monitor -e esp32-wroom
```

#### Para ESP32-S3-Touch-LCD-2.1:
```bash
# PlatformIO
pio run -e esp32s3-lcd --target upload

# Monitor serial
pio device monitor -e esp32s3-lcd
```

### 2. Abrir Monitor Serial

- **Baud Rate**: 115200
- **Line Ending**: Newline

### 3. Testar Leitura

1. Aguarde a mensagem: "Aguardando tags NFC..."
2. Aproxime uma tag NTAG213 ou NTAG215 do leitor
3. As informações da tag serão exibidas no monitor serial

## 📊 Saída Serial Esperada

### Inicialização Bem-Sucedida

```
╔════════════════════════════════════════╗
║  Leitor RFID - ESP32-S3-Touch-LCD-2.1 ║
║         MFRC522 + NTAG213/215         ║
╚════════════════════════════════════════╝

Versão do firmware MFRC522: 0x92
✓ MFRC522 inicializado com sucesso!

----------------------------------
Aguardando tags NFC...
Aproxime uma tag NTAG213 ou NTAG215
----------------------------------
```

### Leitura de Tag NTAG215 com URL (Exemplo)

```
========================================
         NOVA TAG DETECTADA!
========================================
UID da tag: 04A1B2C3D4E5F6
Tamanho do UID: 7 bytes
Tipo PICC: MIFARE Ultralight/NTAG
Subtipo NTAG: NTAG215
========================================

========================================
📊 ESTATÍSTICAS
========================================
Tipo NTAG: NTAG215
Total de bytes: 504
Bytes com dados: 19
Bytes vazios (NULL): 485
Caracteres legíveis: 11
Tipo de conteúdo: URL (NDEF URI)
========================================

========================================
🌐 URL DETECTADA (NDEF)
========================================
https://www.google.com
========================================

Pronto para próxima leitura...
```

**📝 Detalhes**:
- [TAG_READING_OUTPUT.md](TAG_READING_OUTPUT.md) - Exemplos de leitura
- [NDEF_PROTOCOL.md](NDEF_PROTOCOL.md) - Parser NDEF completo

## 🔍 Troubleshooting

### Problema: "Falha na comunicação com MFRC522"

**Soluções**:
- ✓ Verifique todas as conexões dos cabos
- ✓ Confirme que está usando **3.3V** (não 5V)
- ✓ Teste os cabos com um multímetro
- ✓ Verifique se os pinos GPIO estão corretos

### Problema: Tag não é detectada

**Soluções**:
- ✓ Aproxime mais a tag (< 3cm da antena)
- ✓ Verifique se a tag é compatível
- ✓ Teste com outra tag conhecida
- ✓ Verifique se há interferência magnética

### Problema: Leituras intermitentes

**Soluções**:
- ✓ Use cabos mais curtos (< 20cm)
- ✓ Adicione capacitor de 100nF entre VCC-GND do MFRC522
- ✓ Verifique conexões soldadas

### Problema: Porta Serial não abre

**Soluções**:
- ✓ Certifique-se que `ARDUINO_USB_CDC_ON_BOOT=1` está configurado
- ✓ Pressione o botão RESET após upload
- ✓ Use cabo USB com suporte a dados (não apenas carga)

## 📁 Estrutura do Projeto

```
RFID Reader/
├── src/
│   └── main.cpp                    # Código multi-board (auto-detect)
├── include/                        # Headers (vazio por padrão)
├── lib/                            # Bibliotecas locais (vazio)
├── platformio.ini                  # Configuração multi-board
├── NDEF_PROTOCOL.md                # 📚 Parser NDEF e protocolo (NOVO!)
├── MULTI_BOARD_GUIDE.md            # 🎯 Guia multi-board
├── TAG_READING_OUTPUT.md           # 📊 Exemplo de leitura completa
├── SOLUTION_FINAL.md               # ⭐ Guia ESP32-S3-LCD completo
├── GPIO_ANALYSIS_FULL.md           # 📊 Análise técnica completa
├── CONNECTOR_PINOUT_REAL.md        # 🔌 Pinagem dos conectores J9/J8/J10
├── QUICK_REFERENCE.md              # 🚀 Guia rápido (legado)
├── WIRING_DIAGRAM.md               # Diagrama (legado)
├── PINOUT_CONNECTORS.md            # (DESATUALIZADO)
└── README.md                       # Este arquivo
```

## ⚠️ Conclusão da Análise de Pinagem

### Problema Identificado:

❌ **Display RGB ocupa TODOS os pinos disponíveis**:
- 26 GPIOs para LCD (IO1-48)
- 2 GPIOs para I2C (Touch/IMU/RTC)
- 2 GPIOs para USB
- 2 GPIOs para UART
- **SD Card compartilha IO1 e IO2 com o LCD** (inviável)

### Solução Adotada:

✅ **Pinagem híbrida** usando:
1. 🔴 GPIO3, 4, 5 (soldagem em pads)
2. ✅ GPIO42 (SD_MISO - desabilitar SD)
3. ⚠️ GPIO0 (strapping pin + pull-up 10kΩ)

📖 **Documentação completa**: [SOLUTION_FINAL.md](SOLUTION_FINAL.md) e [GPIO_ANALYSIS_FULL.md](GPIO_ANALYSIS_FULL.md)

---

## 📝 Funcionalidades Futuras (Opcional)

- [ ] Escrita de dados em tags NTAG
- [ ] Interface gráfica no display LCD
- [ ] Armazenamento de UIDs em cartão SD
- [ ] Autenticação MIFARE Classic
- [ ] Servidor web para gerenciamento remoto
- [ ] Conexão WiFi para logging em nuvem

## 🔗 Referências

- [Documentação MFRC522](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf)
- [Biblioteca MFRC522 GitHub](https://github.com/miguelbalboa/rfid)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [NTAG213/215 Datasheet](https://www.nxp.com/docs/en/data-sheet/NTAG213_215_216.pdf)
- [Waveshare ESP32-S3-Touch-LCD-2.1](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1)

## 📄 Licença

Este projeto é fornecido como está, sem garantias. Use por sua conta e risco.

## 👨‍💻 Autor

Desenvolvido para leitura de tags NTAG213/215 com ESP32-S3.

---

**Versão**: 1.0  
**Data**: Dezembro 2024

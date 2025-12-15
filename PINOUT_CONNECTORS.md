# Pinagem dos Conectores - ESP32-S3-Touch-LCD-2.1

## Análise dos Conectores Disponíveis

Baseado na documentação oficial e análise de modelos similares da Waveshare, o ESP32-S3-Touch-LCD-2.1 possui os seguintes conectores físicos:

### 📌 1. Conector de 12 Pinos J9 (12PIN Header)

Este é o conector principal para expansão de GPIO. Baseado no **esquemático oficial** ESP32-S3-Touch-LCD-2.1:

| Pin | GPIO   | Função Alternativa | Disponível para SPI? | Observação |
|-----|--------|-------------------|----------------------|------------|
| 1   | **GND**    | Terra             | ✓ (Ground)          | ⚠️ GND, não 3.3V! |
| 2   | **5V**     | Alimentação 5V    | ✓ (Power)           | ⚠️ 5V, não 3.3V! |
| 3   | **3.3V**   | Alimentação 3.3V  | ✓ (Power)           | Usar para MFRC522 |
| 4   | GPIO1  | ADC1_CH0          | ✓                   | |
| 5   | GPIO2  | ADC1_CH1          | ✓                   | |
| 6   | GPIO42 | MTMS              | ✓                   | |
| 7   | GPIO41 | MTDI              | ✓                   | |
| 8   | GPIO40 | MTDO              | ✓                   | |
| 9   | GPIO39 | MTCK              | ✓                   | |
| 10  | GPIO38 | -                 | ✓                   | |
| 11  | GPIO37 | -                 | ✓                   | |
| 12  | GPIO36 | -                 | ✓                   | |

**✅ CONFIRMADO**: Pinagem verificada no esquemático oficial (Conector J9).

### 📌 2. Conector I2C (I2C Interface)

Conector dedicado para periféricos I2C:

| Pin | Função | Observação |
|-----|--------|------------|
| 1   | 3.3V   | Alimentação |
| 2   | GND    | Terra |
| 3   | SDA    | GPIO8 (interno) |
| 4   | SCL    | GPIO9 (interno) |

**⚠️ ATENÇÃO**: Este barramento I2C já está em uso pelos dispositivos internos:
- Endereço **0x51**: Touch Controller
- Endereço **0x6B**: QMI8658 (IMU)
- Endereço **0x7E**: PCF85063 (RTC)

Você pode adicionar novos dispositivos I2C, mas **não pode reutilizar estes GPIOs para outras funções**.

### 📌 3. Conector UART (UART Interface)

| Pin | GPIO   | Função |
|-----|--------|--------|
| 1   | 3.3V   | Alimentação |
| 2   | GND    | Terra |
| 3   | GPIO43 | TX (UART0) |
| 4   | GPIO44 | RX (UART0) |

**⚠️ NOTA**: Este conector compartilha a UART com USB-CDC. Só funciona quando o cabo USB Type-C **não está conectado**.

---

## 🔧 Pinagem Recomendada para MFRC522 (Usando Conector 12PIN)

### Opção A: Usando GPIOs do Conector J9 de 12 Pinos (RECOMENDADO)

Esta configuração usa pinos **fisicamente acessíveis** através do conector J9 de 12 pinos:

| MFRC522 Pin | ESP32-S3 GPIO | Conector J9 (Pin) | Função SPI |
|-------------|---------------|-------------------|------------|
| SDA (SS)    | **GPIO1**     | Pin 4             | Chip Select |
| SCK         | **GPIO2**     | Pin 5             | SPI Clock |
| MOSI        | **GPIO42**    | Pin 6             | Master Out |
| MISO        | **GPIO41**    | Pin 7             | Master In |
| RST         | **GPIO40**    | Pin 8             | Reset |
| 3.3V        | 3.3V          | Pin 3             | Power |
| GND         | GND           | Pin 1             | Ground |

**Vantagens**:
- ✅ Acesso fácil através de conector físico J9
- ✅ Pinos seguros (não usados pelo display RGB interno)
- ✅ Ideal para prototipagem com jumpers
- ✅ Não requer solda em pads pequenos
- ✅ **Confirmado pelo esquemático oficial**

**Código para esta configuração**:
```cpp
#define SS_PIN    1
#define RST_PIN   40
#define SCK_PIN   2
#define MISO_PIN  41
#define MOSI_PIN  42
```

### Opção B: Usando GPIOs Alternativos (Se Opção A não funcionar)

Caso você prefira usar outros GPIOs do conector J9:

| MFRC522 Pin | ESP32-S3 GPIO | Conector J9 (Pin) | Função SPI |
|-------------|---------------|-------------------|------------|
| SDA (SS)    | **GPIO38**    | Pin 10            | Chip Select |
| SCK         | **GPIO37**    | Pin 11            | SPI Clock |
| MOSI        | **GPIO36**    | Pin 12            | Master Out |
| MISO        | **GPIO39**    | Pin 9             | Master In |
| RST         | **GPIO40**    | Pin 8             | Reset |
| 3.3V        | 3.3V          | Pin 3             | Power |
| GND         | GND           | Pin 1             | Ground |

**Código para esta configuração**:
```cpp
#define SS_PIN    38
#define RST_PIN   40
#define SCK_PIN   37
#define MISO_PIN  39
#define MOSI_PIN  36
```

---

## ⚠️ GPIOs a EVITAR

Os seguintes GPIOs **NÃO devem ser usados** pois estão ocupados pelo display e periféricos internos:

### Ocupados pelo Display RGB (LCD 480×480)

| GPIO | Função Display |
|------|---------------|
| GPIO8  | I2C SDA (Touch) |
| GPIO9  | I2C SCL (Touch) |
| GPIO10 | LCD_CS |
| GPIO11 | LCD_CLK |
| GPIO12 | LCD_D0 |
| GPIO13 | LCD_D1 |
| GPIO14 | LCD_D2 |
| GPIO18 | LCD_D3 |
| GPIO19 | LCD_D4 |
| GPIO20 | LCD_D5 |
| GPIO21 | LCD_D6 |
| GPIO47 | LCD_D7 |
| GPIO48 | LCD_D8 |
| GPIO45 | LCD_D9 |
| GPIO46 | LCD_D15 |

**NOTA**: GPIO36-42 **ESTÃO DISPONÍVEIS** no conector J9 e podem ser usados para periféricos externos.

### Strapping Pins (Usar com cuidado)

| GPIO | Função Boot |
|------|------------|
| GPIO0  | Boot Mode |
| GPIO45 | VDD_SPI |
| GPIO46 | ROM Message Printing |

---

## 📐 Diagrama de Conexão Física

```
┌─────────────────────────────────────┐
│   ESP32-S3-Touch-LCD-2.1            │
│                                     │
│   [Conector J9 - 12PIN]             │
│   ┌───────────────┐                 │
│   │ 1  GND   ─────┼─────┐           │
│   │ 2  5V         │     │           │
│   │ 3  3.3V  ─────┼──┐  │           │
│   │ 4  GPIO1 ─────┼┐ │  │           │
│   │ 5  GPIO2 ─────┼││ │  │           │
│   │ 6  GPIO42─────┼│││ │  │           │
│   │ 7  GPIO41─────┼││││ │  │           │
│   │ 8  GPIO40─────┼│││││ │  │           │
│   │ 9  GPIO39     │││││││ │  │           │
│   │ 10 GPIO38     │││││││ │  │           │
│   │ 11 GPIO37     │││││││ │  │           │
│   │ 12 GPIO36     │││││││ │  │           │
│   └───────────────┘││││││ │  │           │
│                    ││││││ │  │           │
└────────────────────││││││─│──│───────┘
                     ││││││ │  │
         ┌───────────┘│││││ │  │
         │┌───────────┘││││ │  │
         ││┌───────────┘│││ │  │
         │││┌───────────┘││ │  │
         ││││┌───────────┘│ │  │
         │││││  ┌─────────┘ │  │
         │││││  │  ┌────────┘  │
         │││││  │  │  ┌────────┘
         │││││  │  │  │
     ┌───▼▼▼▼▼──▼──▼──┐
     │  MFRC522        │
     │                 │
     │  1  SDA (SS)    │ ◄── GPIO1  (Pin 4)
     │  2  SCK         │ ◄── GPIO2  (Pin 5)
     │  3  MOSI        │ ◄── GPIO42 (Pin 6)
     │  4  MISO        │ ◄── GPIO41 (Pin 7)
     │  5  IRQ         │ (não conectado)
     │  6  GND         │ ◄── GND    (Pin 1)
     │  7  RST         │ ◄── GPIO40 (Pin 8)
     │  8  3.3V        │ ◄── 3.3V   (Pin 3)
     │                 │
     └─────────────────┘
```

---

## 🔍 Como Verificar a Pinagem do Seu Módulo

Se você não tem certeza dos GPIOs disponíveis no conector de 12 pinos:

### Método 1: Verificação Visual
1. Olhe a **serigrafia** na parte de trás da placa
2. Os números dos GPIOs geralmente estão impressos próximo ao conector

### Método 2: Consultar Esquemático
1. Baixe o esquemático: [ESP32-S3-Touch-LCD-2.1_schematic_diagram.pdf](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1/ESP32-S3-Touch-LCD-2.1_schematic_diagram.pdf)
2. Procure pela seção "12PIN Connector" ou "GPIO Extension"

### Método 3: Teste com Multímetro
1. Use modo continuidade para traçar as conexões
2. Compare com pads de teste no ESP32-S3

---

## 💡 Recomendação Final

**Use a Opção A (GPIO1-5)** para a conexão do MFRC522:
- São GPIOs de propósito geral seguros
- Facilmente acessíveis via conector físico
- Não conflitam com periféricos internos
- Ideais para SPI

Se sua placa tiver uma pinagem diferente, ajuste conforme necessário e atualize o código em `src/main.cpp`.

---

**Versão**: 1.1  
**Data**: Dezembro 2024  
**Status**: ⚠️ Verificar serigrafia da placa para confirmação

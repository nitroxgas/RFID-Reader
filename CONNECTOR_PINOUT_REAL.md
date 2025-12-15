# Pinagem Real dos Conectores - ESP32-S3-Touch-LCD-2.1

## ✅ Pinagem Verificada Fisicamente

### 📌 Conector J9 (12 Pinos)

| Pin | Sinal       | Descrição                          | Status                      |
|-----|-------------|------------------------------------|-----------------------------|
| 1   | GND         | Terra                              | ✓ Disponível                |
| 2   | USB_5V      | Alimentação 5V (USB)               | ✓ Disponível                |
| 3   | IO19        | GPIO19 (Display RGB - LCD_D4)      | ❌ Ocupado pelo Display     |
| 4   | IO10        | GPIO10 (Display RGB - LCD_CS)      | ❌ Ocupado pelo Display     |
| 5   | GND         | Terra                              | ✓ Disponível                |
| 6   | 3.3V        | Alimentação 3.3V                   | ✓ Disponível                |
| 7   | SCL         | I2C Clock (GPIO9)                  | ❌ Ocupado por I2C Interno  |
| 8   | SDA         | I2C Data (GPIO8)                   | ❌ Ocupado por I2C Interno  |
| 9   | UART_TXD    | UART TX (GPIO43)                   | ⚠️ Compartilhado com USB    |
| 10  | UART_RXD    | UART RX (GPIO44)                   | ⚠️ Compartilhado com USB    |
| 11  | GND         | Terra                              | ✓ Disponível                |
| 12  | GPIO0       | GPIO0                              | ⚠️ Strapping Pin (Boot)     |

**Resumo J9**: Apenas GPIO0 está livre, mas é um strapping pin (não recomendado para SPI).

---

### 📌 Conector J8 (4 Pinos - I2C)

| Pin | Sinal | Descrição                    | Status                     |
|-----|-------|------------------------------|----------------------------|
| 1   | SCL   | I2C Clock (GPIO9)            | ❌ Ocupado por I2C Interno |
| 2   | SDA   | I2C Data (GPIO8)             | ❌ Ocupado por I2C Interno |
| 3   | 3.3V  | Alimentação 3.3V             | ✓ Disponível               |
| 4   | GND   | Terra                        | ✓ Disponível               |

**Resumo J8**: Conector I2C dedicado, já em uso pelos periféricos internos:
- **0x15**: Touch Controller (CST816D)
- **0x6B**: IMU (QMI8658C)
- **0x51**: RTC (PCF85063)

---

### 📌 Conector J10 (4 Pinos - UART)

| Pin | Sinal     | Descrição              | Status                   |
|-----|-----------|------------------------|--------------------------|
| 1   | UART_RXD  | UART RX (GPIO44)       | ⚠️ Compartilhado com USB |
| 2   | UART_TXD  | UART TX (GPIO43)       | ⚠️ Compartilhado com USB |
| 3   | 3.3V      | Alimentação 3.3V       | ✓ Disponível             |
| 4   | GND       | Terra                  | ✓ Disponível             |

**Resumo J10**: UART0 compartilhado com USB CDC. Só funciona quando USB **não está conectado**.

---

## 🔌 Pinagem MFRC522 (RFID-RC522)

| Pin | Sinal | Descrição                     | Requerimento              |
|-----|-------|-------------------------------|---------------------------|
| 1   | SDA   | Chip Select (SS)              | GPIO livre                |
| 2   | SCK   | SPI Clock                     | GPIO livre                |
| 3   | MOSI  | Master Out Slave In           | GPIO livre                |
| 4   | MISO  | Master In Slave Out           | GPIO livre                |
| 5   | IRQ   | Interrupt (Opcional)          | GPIO livre (não usado)    |
| 6   | GND   | Terra                         | GND                       |
| 7   | RST   | Reset                         | GPIO livre                |
| 8   | 3.3V  | Alimentação 3.3V              | 3.3V                      |

**Total necessário**: 5 GPIOs livres (SDA, SCK, MOSI, MISO, RST)

---

## ❌ Problema: GPIOs Insuficientes nos Conectores

### Análise:
Os conectores físicos disponíveis (J9, J8, J10) **não possuem 5 GPIOs livres** necessários para o MFRC522:

- **J9**: Apenas GPIO0 livre (strapping pin, não recomendado)
- **J8**: I2C já ocupado
- **J10**: UART compartilhado com USB

### GPIOs Ocupados pelo Display RGB:
```
GPIO8  - I2C SDA (Touch)
GPIO9  - I2C SCL (Touch)
GPIO10 - LCD_CS
GPIO11 - LCD_CLK
GPIO12 - LCD_D0
GPIO13 - LCD_D1
GPIO14 - LCD_D2
GPIO18 - LCD_D3
GPIO19 - LCD_D4
GPIO20 - LCD_D5
GPIO21 - LCD_D6
GPIO47 - LCD_D7
GPIO48 - LCD_D8
GPIO45 - LCD_D9
GPIO46 - LCD_D15
```

---

## 💡 Soluções Alternativas

### Opção 1: Soldagem em Pads do ESP32-S3 (RECOMENDADO)

Usar GPIOs não expostos nos conectores, soldando fios diretamente nos pads do chip ESP32-S3:

#### GPIOs Candidatos (Não usados pelo display):
- **GPIO1** (ADC1_CH0)
- **GPIO2** (ADC1_CH1)
- **GPIO3** (ADC1_CH2)
- **GPIO4** (ADC1_CH3)
- **GPIO5**
- **GPIO6**
- **GPIO7**

#### Pinagem Sugerida com Soldagem:

| MFRC522 Pin | ESP32-S3 GPIO | Localização          | Função SPI    |
|-------------|---------------|----------------------|---------------|
| SDA (SS)    | **GPIO1**     | Pad no chip          | Chip Select   |
| SCK         | **GPIO2**     | Pad no chip          | SPI Clock     |
| MOSI        | **GPIO3**     | Pad no chip          | Master Out    |
| MISO        | **GPIO4**     | Pad no chip          | Master In     |
| RST         | **GPIO5**     | Pad no chip          | Reset         |
| 3.3V        | 3.3V          | J9 Pin 6 ou J8 Pin 3 | Power         |
| GND         | GND           | J9 Pin 1/5/11        | Ground        |

**Código para esta configuração:**
```cpp
#define SS_PIN    1
#define RST_PIN   5
#define SCK_PIN   2
#define MISO_PIN  4
#define MOSI_PIN  3
```

**Vantagens**:
- ✅ GPIOs livres e seguros
- ✅ Não conflita com periféricos internos
- ✅ Melhor desempenho SPI

**Desvantagens**:
- ❌ Requer soldagem de precisão
- ❌ Dificulta manutenção
- ❌ Risco de danificar a placa

---

### Opção 2: Usar GPIO0 + Soldagem Híbrida

Combinar GPIO0 do conector J9 com soldagem de alguns pinos:

| MFRC522 Pin | ESP32-S3 GPIO | Localização    | Função SPI    |
|-------------|---------------|----------------|---------------|
| SDA (SS)    | **GPIO0**     | J9 Pin 12      | Chip Select   |
| SCK         | **GPIO2**     | Solda no chip  | SPI Clock     |
| MOSI        | **GPIO3**     | Solda no chip  | Master Out    |
| MISO        | **GPIO4**     | Solda no chip  | Master In     |
| RST         | **GPIO5**     | Solda no chip  | Reset         |
| 3.3V        | 3.3V          | J9 Pin 6       | Power         |
| GND         | GND           | J9 Pin 1       | Ground        |

**⚠️ ATENÇÃO**: GPIO0 é um strapping pin. Pode causar problemas no boot se estiver conectado incorretamente.

**Código para esta configuração:**
```cpp
#define SS_PIN    0
#define RST_PIN   5
#define SCK_PIN   2
#define MISO_PIN  4
#define MOSI_PIN  3
```

---

### Opção 3: Módulo Expansor I2C para SPI

Usar um chip expansor I2C-to-SPI (como MCP23017 + bit-banging):

**Vantagens**:
- ✅ Usa apenas conector J8 (I2C)
- ✅ Sem soldagem
- ✅ Fácil de reverter

**Desvantagens**:
- ❌ Hardware adicional necessário
- ❌ Desempenho reduzido (bit-banging)
- ❌ Mais complexo de implementar
- ❌ Pode ter conflitos com I2C interno

**Não recomendado** para este projeto devido à complexidade.

---

### Opção 4: Módulo ESP32-S3 Externo com Bridge

Usar um segundo ESP32-S3 dedicado ao RFID, comunicando com o display via UART:

**Não recomendado** - excessivamente complexo.

---

## ✅ Recomendação Final

### Melhor Opção: **Opção 1 - Soldagem em Pads do ESP32-S3**

Usar GPIO1, 2, 3, 4, 5 soldando fios diretamente nos pads do chip:

```
MFRC522          ESP32-S3 (Solda)         Alimentação
┌──────────┐     ┌──────────────┐         ┌─────────┐
│ 1  SDA   ├─────┤ GPIO1  (Pad) │         │         │
│ 2  SCK   ├─────┤ GPIO2  (Pad) │         │   J9    │
│ 3  MOSI  ├─────┤ GPIO3  (Pad) │         │         │
│ 4  MISO  ├─────┤ GPIO4  (Pad) │         │ Pin 6   │
│ 5  IRQ   │ NC  │              │         │ 3.3V  ──┼──┐
│ 6  GND   ├─────┼──────────────┤         │         │  │
│ 7  RST   ├─────┤ GPIO5  (Pad) │         │ Pin 1   │  │
│ 8  3.3V  ├─────┼──────────────┤         │ GND   ──┼──┤
└──────────┘     └──────────────┘         └─────────┘  │
                                                        │
                                          ┌─────────────┴───┐
                                          │   Usar 3.3V e   │
                                          │   GND do J9     │
                                          └─────────────────┘
```

### Localização dos Pads GPIO1-5:

Consulte o **datasheet do ESP32-S3-WROOM-1** para localizar os pads:
- GPIO1: Pad 4
- GPIO2: Pad 5
- GPIO3: Pad 6
- GPIO4: Pad 7
- GPIO5: Pad 8

**Ferramentas necessárias**:
- Ferro de solda de ponta fina (< 1mm)
- Fio AWG 30 ou mais fino
- Flux para solda
- Lupa ou microscópio
- Multímetro para continuidade

---

## 📐 Diagrama de Conexão Recomendado

```
┌─────────────────────────────────────────────┐
│   ESP32-S3-Touch-LCD-2.1                    │
│                                             │
│   ┌─────────────┐                           │
│   │  ESP32-S3   │                           │
│   │   (Chip)    │                           │
│   │             │                           │
│   │  Pad 4 ─────┼─┐  GPIO1                  │
│   │  Pad 5 ─────┼─┼─┐  GPIO2                │
│   │  Pad 6 ─────┼─┼─┼─┐  GPIO3              │
│   │  Pad 7 ─────┼─┼─┼─┼─┐  GPIO4            │
│   │  Pad 8 ─────┼─┼─┼─┼─┼─┐  GPIO5          │
│   └─────────────┘ │ │ │ │ │                 │
│                   │ │ │ │ │                 │
│   [J9 Connector]  │ │ │ │ │                 │
│   Pin 6: 3.3V ────┼─┼─┼─┼─┼─────┐           │
│   Pin 1: GND ─────┼─┼─┼─┼─┼───┐ │           │
│                   │ │ │ │ │   │ │           │
└───────────────────│─│─│─│─│───│─│───────────┘
                    │ │ │ │ │   │ │
                    │ │ │ │ │   │ │
                ┌───▼─▼─▼─▼─▼───▼─▼───┐
                │   MFRC522            │
                │                      │
                │  1  SDA (SS)  ◄──────┤ GPIO1
                │  2  SCK       ◄──────┤ GPIO2
                │  3  MOSI      ◄──────┤ GPIO3
                │  4  MISO      ◄──────┤ GPIO4
                │  5  IRQ       │      │ (NC)
                │  6  GND       ◄──────┤ GND (J9 Pin 1)
                │  7  RST       ◄──────┤ GPIO5
                │  8  3.3V      ◄──────┤ 3.3V (J9 Pin 6)
                │                      │
                └──────────────────────┘
```

---

## 📝 Histórico de Revisões

| Versão | Data       | Descrição                                      |
|--------|------------|------------------------------------------------|
| 1.0    | 2024-12-12 | Pinagem inicial baseada em suposições          |
| 2.0    | 2024-12-12 | Pinagem corrigida baseada em esquemático       |
| 3.0    | 2024-12-12 | **Pinagem real verificada fisicamente**        |
|        |            | Identificado que conectores não têm GPIOs livres |
|        |            | Recomendação: Soldagem em pads do ESP32-S3    |

---

**Status**: ✅ Documentação completa e verificada  
**Recomendação**: Opção 1 (Soldagem em GPIO1-5)  
**Dificuldade**: Alta (requer soldagem de precisão)

# Esquema de Ligação - ESP32-S3-Touch-LCD-2.1 + MFRC522

## Visão Geral
Este documento descreve a conexão entre o display **Waveshare ESP32-S3-Touch-LCD-2.1** e o módulo leitor RFID **MFRC522**.

## Especificações dos Componentes

### ESP32-S3-Touch-LCD-2.1
- **Processador**: ESP32-S3R8 (Dual-core, 240MHz)
- **Display**: 2.1" LCD Capacitivo Touch (480×480)
- **Flash**: 16MB
- **PSRAM**: 8MB
- **Interfaces disponíveis**: 
  - Conector 12PIN
  - Interface I2C
  - Interface UART
  - Porta USB Type-C

### MFRC522
- **Interface**: SPI
- **Tensão de operação**: 3.3V
- **Frequência**: 13.56 MHz
- **Suporta**: NTAG213, NTAG215, MIFARE Classic, etc.

## Pinagem SPI para MFRC522

O MFRC522 utiliza comunicação SPI. A seguir, a conexão **usando o conector J9 de 12 pinos** disponível no ESP32-S3-Touch-LCD-2.1:

### Tabela de Conexões (Conector J9)

| MFRC522 Pin | ESP32-S3 GPIO | Conector J9 (Pin) | Função       | Descrição                    |
|-------------|---------------|-------------------|--------------|------------------------------|
| SDA (SS)    | **GPIO1**     | Pin 4             | Chip Select  | Slave Select (Ativo em LOW)  |
| SCK         | **GPIO2**     | Pin 5             | SPI Clock    | Clock do barramento SPI      |
| MOSI        | **GPIO42**    | Pin 6             | Master Out   | Dados ESP32 → MFRC522        |
| MISO        | **GPIO41**    | Pin 7             | Master In    | Dados MFRC522 → ESP32        |
| IRQ         | Não conectado | -                 | Interrupt    | (Opcional)                   |
| GND         | GND           | Pin 1             | Ground       | Terra comum                  |
| RST         | **GPIO40**    | Pin 8             | Reset        | Reset do módulo MFRC522      |
| 3.3V        | 3.3V          | Pin 3             | Power        | Alimentação 3.3V             |

**✅ NOTA**: Pinagem confirmada pelo **esquemático oficial** (Conector J9).

### Diagrama de Conexão

```
ESP32-S3-Touch-LCD-2.1                    MFRC522
┌─────────────────────────┐              ┌──────────┐
│                         │              │          │
│  [Conector J9 - 12PIN]   │              │          │
│  ┌───────────────┐       │              │          │
│  │ Pin 1: GND    ├──────┼──────────────┤ GND      │
│  │ Pin 2: 5V          │       │              │          │
│  │ Pin 3: 3.3V   ├──────┼──────────────┤ 3.3V     │
│  │ Pin 4: GPIO1  ├──────┼──────────────┤ SDA (SS) │
│  │ Pin 5: GPIO2  ├──────┼──────────────┤ SCK      │
│  │ Pin 6: GPIO42 ├──────┼──────────────┤ MOSI     │
│  │ Pin 7: GPIO41 ├──────┼──────────────┤ MISO     │
│  │ Pin 8: GPIO40 ├──────┼──────────────┤ RST      │
│  │ Pin 9: GPIO39      │       │              │          │
│  │ Pin 10: GPIO38     │       │              │          │
│  │ Pin 11: GPIO37     │       │              │          │
│  │ Pin 12: GPIO36     │       │              │          │
│  └───────────────┘       │              │          │
│                         │              │          │
└─────────────────────────┘              └──────────┘

⚠️ IMPORTANTE: Pin 1 = GND, Pin 2 = 5V, Pin 3 = 3.3V
(Confirme com o esquemático oficial)
```

## Notas Importantes

### ⚠️ Considerações de Hardware

1. **Tensão de Alimentação**: 
   - O MFRC522 opera em **3.3V**. Não conecte diretamente em 5V.
   - Use a saída de 3.3V do ESP32-S3.

2. **Pinos Disponíveis**:
   - Os GPIOs escolhidos (GPIO1, 2, 40, 41, 42) estão **disponíveis no conector J9 de 12 pinos**.
   - Estes pinos são seguros e não conflitam com o display RGB interno.
   - **✅ Pinagem confirmada pelo esquemático oficial**.
   - Evite usar GPIO0, GPIO45, GPIO46 (pinos de strapping/boot).

3. **Conector J9 de 12 Pinos**:
   - **Pin 1 = GND** (⚠️ não é 3.3V!)
   - **Pin 2 = 5V** (⚠️ não é GND!)
   - **Pin 3 = 3.3V** (use este para alimentar o MFRC522)
   - Confirme com o esquemático: [ESP32-S3-Touch-LCD-2.1_schematic_diagram.pdf](https://files.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.1/ESP32-S3-Touch-LCD-2.1_schematic_diagram.pdf)

4. **Comprimento dos Cabos**:
   - Mantenha cabos SPI curtos (< 30cm) para evitar interferências.
   - Use cabos blindados se necessário.

5. **Capacitor de Desacoplamento**:
   - Recomenda-se um capacitor de 100nF entre VCC e GND do MFRC522, próximo ao módulo.

### 🔧 Configuração de Software

Os pinos definidos no código (`main.cpp`):
```cpp
// Usando GPIOs do conector J9 de 12 pinos
// Baseado no esquemático oficial
#define SS_PIN    1   // Pin 4 do conector J9
#define RST_PIN   40  // Pin 8 do conector J9
#define SCK_PIN   2   // Pin 5 do conector J9
#define MISO_PIN  41  // Pin 7 do conector J9
#define MOSI_PIN  42  // Pin 6 do conector J9
```

**✅ CONFIRMADO**: Pinagem verificada no esquemático oficial. Consulte `PINOUT_CONNECTORS.md` para opções alternativas.

## Testes de Funcionamento

### Verificação de Conexão
1. Compile e faça upload do código
2. Abra o Monitor Serial (115200 baud)
3. Você deve ver: "MFRC522 inicializado com sucesso!"
4. Aproxime uma tag NFC NTAG213 ou NTAG215
5. O UID da tag será exibido no monitor serial

### Troubleshooting

**Problema**: "Falha na inicialização do MFRC522"
- ✓ Verifique todas as conexões
- ✓ Confirme tensão de 3.3V no MFRC522
- ✓ Verifique se os pinos GPIO estão corretos

**Problema**: Tag não é detectada
- ✓ Aproxime a tag da antena do MFRC522 (< 5cm)
- ✓ Verifique se a tag é compatível (NTAG213/215, MIFARE Classic)
- ✓ Teste com outra tag conhecida

**Problema**: Leituras intermitentes
- ✓ Reduza comprimento dos cabos
- ✓ Adicione capacitor de desacoplamento
- ✓ Verifique conexões soltas

## Recursos Adicionais

- [Datasheet MFRC522](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf)
- [Biblioteca MFRC522 Arduino](https://github.com/miguelbalboa/rfid)
- [Especificações NTAG213/215](https://www.nxp.com/docs/en/data-sheet/NTAG213_215_216.pdf)
- [Documentação ESP32-S3](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

---

**Última atualização**: Dezembro 2024
**Versão**: 1.0

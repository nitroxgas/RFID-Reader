#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// ============================================
// CONFIGURAÇÃO DE PINOS - MÚLTIPLAS PLACAS
// Seleciona automaticamente baseado no ambiente PlatformIO
// ============================================

#if defined(BOARD_ESP32S3_LCD)
  // ============================================
  // ESP32-S3-Touch-LCD-2.1 (Waveshare)
  // - Requer soldagem em GPIO3, 4, 5
  // - GPIO42 (SD_MISO) - desabilitar SD Card
  // - GPIO0 com pull-up 10kΩ obrigatório
  // ============================================
  #define SS_PIN    3    // GPIO3 (Solda no Pad 6)
  #define RST_PIN   0    // GPIO0 (J9 Pin 12 + pull-up 10kΩ)
  #define SCK_PIN   4    // GPIO4 (Solda no Pad 7)
  #define MISO_PIN  42   // GPIO42 (SD_MISO)
  #define MOSI_PIN  5    // GPIO5 (Solda no Pad 8)
  #define BOARD_NAME "ESP32-S3-Touch-LCD-2.1"
  
#elif defined(BOARD_ESP32_WROOM)
  // ============================================
  // ESP32-WROOM (Genérico)
  // - Pinagem padrão VSPI
  // - Fácil de conectar (sem soldagem)
  // - Ideal para testes iniciais
  // ============================================
  #define SS_PIN    5    // GPIO5 (padrão VSPI)
  #define RST_PIN   22   // GPIO22 (livre)
  #define SCK_PIN   18   // GPIO18 (padrão VSPI)
  #define MISO_PIN  19   // GPIO19 (padrão VSPI)
  #define MOSI_PIN  23   // GPIO23 (padrão VSPI)
  #define BOARD_NAME "ESP32-WROOM"
  
#else
  // ============================================
  // Fallback: ESP32 Genérico (VSPI)
  // ============================================
  #define SS_PIN    5
  #define RST_PIN   22
  #define SCK_PIN   18
  #define MISO_PIN  19
  #define MOSI_PIN  23
  #define BOARD_NAME "ESP32 (padrão)"
  #warning "Placa não especificada! Usando pinagem padrão VSPI."
#endif

// Criar instância do MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

// ============================================
// VARIÁVEIS GLOBAIS
// ============================================
unsigned long lastCardReadTime = 0;
const unsigned long CARD_READ_DELAY = 2000; // 2 segundos entre leituras da mesma tag

// ============================================
// FUNÇÕES AUXILIARES
// ============================================

/**
 * Converte um array de bytes para string hexadecimal
 */
String bytesToHexString(byte *buffer, byte bufferSize) {
  String hexString = "";
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) hexString += "0";
    hexString += String(buffer[i], HEX);
  }
  hexString.toUpperCase();
  return hexString;
}

/**
 * Detecta o tipo de tag NFC
 */
String getCardType(byte piccType) {
  switch (piccType) {
    case MFRC522::PICC_TYPE_MIFARE_MINI:
      return "MIFARE Mini";
    case MFRC522::PICC_TYPE_MIFARE_1K:
      return "MIFARE 1KB";
    case MFRC522::PICC_TYPE_MIFARE_4K:
      return "MIFARE 4KB";
    case MFRC522::PICC_TYPE_MIFARE_UL:
      return "MIFARE Ultralight/NTAG";
    case MFRC522::PICC_TYPE_MIFARE_PLUS:
      return "MIFARE Plus";
    case MFRC522::PICC_TYPE_MIFARE_DESFIRE:
      return "MIFARE DESFire";
    case MFRC522::PICC_TYPE_TNP3XXX:
      return "TNP3XXX";
    case MFRC522::PICC_TYPE_NOT_COMPLETE:
      return "Incompleto";
    case MFRC522::PICC_TYPE_UNKNOWN:
    default:
      return "Desconhecido";
  }
}

/**
 * Lê dados de uma página da tag NTAG
 */
bool readNTAGPage(byte pageAddr, byte *buffer) {
  MFRC522::StatusCode status;
  byte size = 18;
  
  status = mfrc522.MIFARE_Read(pageAddr, buffer, &size);
  
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Erro ao ler página ");
    Serial.print(pageAddr);
    Serial.print(": ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
  }
  return true;
}

/**
 * Determina se é NTAG213 ou NTAG215 baseado no tamanho
 * Retorna 0 para tipo desconhecido, 213 para NTAG213, 215 para NTAG215
 */
int detectNTAGType() {
  byte buffer[18];
  
  // Tenta ler a página de configuração (página 16 para NTAG213, existe em ambos)
  if (readNTAGPage(16, buffer)) {
    // NTAG213 tem 45 páginas (180 bytes user memory - páginas 4-44)
    // NTAG215 tem 135 páginas (504 bytes user memory - páginas 4-129)
    
    // Tenta ler página 130 (só existe no NTAG215)
    if (readNTAGPage(130, buffer)) {
      return 215;
    } else {
      return 213;
    }
  }
  
  return 0; // Tipo não determinado
}

/**
 * Converte nome do tipo NTAG para string
 */
String getNTAGTypeName(int ntagType) {
  switch(ntagType) {
    case 213: return "NTAG213";
    case 215: return "NTAG215";
    default: return "NTAG (tipo não determinado)";
  }
}

/**
 * Converte URI Identifier Code para prefixo de URL
 * Conforme especificação NFC Forum URI Record Type Definition
 */
String getURIPrefix(byte code) {
  switch(code) {
    case 0x00: return "";                    // Sem prefixo
    case 0x01: return "http://www.";         // Mais comum
    case 0x02: return "https://www.";
    case 0x03: return "http://";
    case 0x04: return "https://";
    case 0x05: return "tel:";
    case 0x06: return "mailto:";
    case 0x07: return "ftp://anonymous:anonymous@";
    case 0x08: return "ftp://ftp.";
    case 0x09: return "ftps://";
    case 0x0A: return "sftp://";
    case 0x0B: return "smb://";
    case 0x0C: return "nfs://";
    case 0x0D: return "ftp://";
    case 0x0E: return "dav://";
    case 0x0F: return "news:";
    case 0x10: return "telnet://";
    case 0x11: return "imap:";
    case 0x12: return "rtsp://";
    case 0x13: return "urn:";
    case 0x14: return "pop:";
    case 0x15: return "sip:";
    case 0x16: return "sips:";
    case 0x17: return "tftp:";
    case 0x18: return "btspp://";
    case 0x19: return "btl2cap://";
    case 0x1A: return "btgoep://";
    case 0x1B: return "tcpobex://";
    case 0x1C: return "irdaobex://";
    case 0x1D: return "file://";
    case 0x1E: return "urn:epc:id:";
    case 0x1F: return "urn:epc:tag:";
    case 0x20: return "urn:epc:pat:";
    case 0x21: return "urn:epc:raw:";
    case 0x22: return "urn:epc:";
    case 0x23: return "urn:nfc:";
    default: return "[Código desconhecido]";
  }
}

/**
 * Detecta e extrai URL de mensagem NDEF
 * Retorna string vazia se não for URL ou não encontrar
 */
String extractNDEFUrl(byte* data, int dataSize) {
  // Verifica se começa com TLV NDEF Message (0x03)
  if (dataSize < 3 || data[0] != 0x03) {
    return ""; // Não é mensagem NDEF
  }
  
  int messageLength = data[1];
  if (messageLength == 0 || messageLength > dataSize - 2) {
    return ""; // Tamanho inválido
  }
  
  // Offset para início do NDEF Record
  int offset = 2;
  
  // Lê Header do NDEF Record
  byte header = data[offset++];
  
  // Verifica flags do header
  bool MB = (header & 0x80) != 0;  // Message Begin
  bool ME = (header & 0x40) != 0;  // Message End
  bool CF = (header & 0x20) != 0;  // Chunk Flag
  bool SR = (header & 0x10) != 0;  // Short Record
  bool IL = (header & 0x08) != 0;  // ID Length present
  byte TNF = header & 0x07;        // Type Name Format
  
  // Verifica se é Well-Known type (TNF = 0x01) - usado para URI
  if (TNF != 0x01) {
    return ""; // Não é tipo conhecido
  }
  
  // Lê Type Length
  byte typeLength = data[offset++];
  
  // Lê Payload Length
  int payloadLength;
  if (SR) {
    // Short Record - 1 byte de payload length
    payloadLength = data[offset++];
  } else {
    // Normal Record - 4 bytes de payload length (big-endian)
    payloadLength = (data[offset] << 24) | (data[offset+1] << 16) | 
                    (data[offset+2] << 8) | data[offset+3];
    offset += 4;
  }
  
  // Lê ID Length se presente
  if (IL) {
    byte idLength = data[offset++];
    offset += idLength; // Pula o ID
  }
  
  // Lê Record Type
  String recordType = "";
  for (int i = 0; i < typeLength; i++) {
    recordType += (char)data[offset++];
  }
  
  // Verifica se é URI Record (Type = "U")
  if (recordType != "U") {
    return ""; // Não é URI
  }
  
  // Primeiro byte do payload é o URI Identifier Code
  byte uriCode = data[offset++];
  String url = getURIPrefix(uriCode);
  
  // Lê o restante do payload (a URL em si)
  for (int i = 1; i < payloadLength && offset < dataSize; i++) {
    byte b = data[offset++];
    if (b == 0x00 || b == 0xFE) break; // Fim dos dados
    url += (char)b;
  }
  
  return url;
}

/**
 * Detecta e extrai texto de mensagem NDEF
 * Retorna string vazia se não for texto ou não encontrar
 */
String extractNDEFText(byte* data, int dataSize) {
  if (dataSize < 3 || data[0] != 0x03) {
    return ""; // Não é mensagem NDEF
  }
  
  int messageLength = data[1];
  if (messageLength == 0 || messageLength > dataSize - 2) {
    return "";
  }
  
  int offset = 2;
  byte header = data[offset++];
  byte TNF = header & 0x07;
  
  if (TNF != 0x01) return "";
  
  byte typeLength = data[offset++];
  
  bool SR = (header & 0x10) != 0;
  int payloadLength;
  if (SR) {
    payloadLength = data[offset++];
  } else {
    payloadLength = (data[offset] << 24) | (data[offset+1] << 16) | 
                    (data[offset+2] << 8) | data[offset+3];
    offset += 4;
  }
  
  bool IL = (header & 0x08) != 0;
  if (IL) {
    byte idLength = data[offset++];
    offset += idLength;
  }
  
  String recordType = "";
  for (int i = 0; i < typeLength; i++) {
    recordType += (char)data[offset++];
  }
  
  // Verifica se é Text Record (Type = "T")
  if (recordType != "T") {
    return "";
  }
  
  // Primeiro byte: Status byte (bit 7 = UTF-16, bits 0-5 = tamanho código idioma)
  byte statusByte = data[offset++];
  byte langCodeLength = statusByte & 0x3F;
  bool isUTF16 = (statusByte & 0x80) != 0;
  
  // Pula código de idioma (ex: "en")
  offset += langCodeLength;
  
  // Lê o texto
  String text = "";
  int textLength = payloadLength - 1 - langCodeLength;
  for (int i = 0; i < textLength && offset < dataSize; i++) {
    byte b = data[offset++];
    if (b == 0x00 || b == 0xFE) break;
    text += (char)b;
  }
  
  return text;
}

/**
 * Lê todos os dados do usuário da tag NTAG
 */
void readAllNTAGData(int ntagType) {
  if (ntagType == 0) {
    Serial.println("⚠️ Tipo NTAG desconhecido, não é possível ler dados.");
    return;
  }
  
  // Define range de páginas baseado no tipo
  byte startPage = 4;  // Primeira página de dados do usuário
  byte endPage;
  int totalBytes;
  
  if (ntagType == 213) {
    endPage = 44;      // NTAG213: páginas 4-44 (41 páginas × 4 bytes = 164 bytes)
    totalBytes = 164;
  } else { // NTAG215
    endPage = 129;     // NTAG215: páginas 4-129 (126 páginas × 4 bytes = 504 bytes)
    totalBytes = 504;
  }
  
  // Buffer para armazenar todos os dados
  byte* allData = new byte[totalBytes];
  int dataIndex = 0;
  bool readSuccess = true;
  
  // Lê todas as páginas (silenciosamente)
  for (byte page = startPage; page <= endPage; page++) {
    byte buffer[18];
    
    if (readNTAGPage(page, buffer)) {
      // Armazena os 4 bytes da página
      for (byte i = 0; i < 4; i++) {
        if (dataIndex < totalBytes) {
          allData[dataIndex++] = buffer[i];
        }
      }
    } else {
      Serial.print("\n⚠️ Erro ao ler página ");
      Serial.println(page);
      readSuccess = false;
      break;
    }
  }
  
  if (!readSuccess) {
    delete[] allData;
    return;
  }
  
  // Tenta extrair URL NDEF
  String ndefUrl = extractNDEFUrl(allData, dataIndex);
  String ndefText = extractNDEFText(allData, dataIndex);
  
  // ========================================
  // SEÇÃO 2: ESTATÍSTICAS
  // ========================================
  Serial.println("\n========================================");
  Serial.println("� ESTATÍSTICAS");
  Serial.println("========================================");
  Serial.print("Tipo NTAG: ");
  Serial.println(getNTAGTypeName(ntagType));
  Serial.print("Total de bytes: ");
  Serial.println(dataIndex);
  
  
  // Conta bytes não nulos
  int nonNullBytes = 0;
  int nullBytes = 0;
  for (int i = 0; i < dataIndex; i++) {
    if (allData[i] != 0x00) {
      nonNullBytes++;
    } else {
      nullBytes++;
    }
  }
  Serial.print("Bytes com dados: ");
  Serial.println(nonNullBytes);
  Serial.print("Bytes vazios (NULL): ");
  Serial.println(nullBytes);
  
  // Conta caracteres imprimíveis
  int printableChars = 0;
  for (int i = 0; i < dataIndex; i++) {
    if (allData[i] >= 32 && allData[i] <= 126) printableChars++;
  }
  Serial.print("Caracteres legíveis: ");
  Serial.println(printableChars);
  
  // Mostra tipo de conteúdo NDEF
  Serial.print("Tipo de conteúdo: ");
  if (ndefUrl.length() > 0) {
    Serial.println("URL (NDEF URI)");
  } else if (ndefText.length() > 0) {
    Serial.println("Texto (NDEF)");
  } else if (allData[0] == 0x03) {
    Serial.println("NDEF desconhecido");
  } else {
    Serial.println("Dados brutos");
  }
  Serial.println("========================================");
  
  // ========================================
  // SEÇÃO 3: URL DETECTADA (se houver)
  // ========================================
  if (ndefUrl.length() > 0) {
    Serial.println("\n========================================");
    Serial.println("🌐 URL DETECTADA (NDEF)");
    Serial.println("========================================");
    Serial.println(ndefUrl);
    Serial.println("========================================");
  } else if (ndefText.length() > 0) {
    Serial.println("\n========================================");
    Serial.println("📝 TEXTO DETECTADO (NDEF)");
    Serial.println("========================================");
    Serial.println(ndefText);
    Serial.println("========================================");
  }
  
  Serial.println();
  
  delete[] allData;
}

/**
 * Exibe informações detalhadas da tag
 */
void displayCardInfo() {
  Serial.println("\n========================================");
  Serial.println("         NOVA TAG DETECTADA!");
  Serial.println("========================================");
  
  // UID
  Serial.print("UID da tag: ");
  String uid = bytesToHexString(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println(uid);
  
  // Tamanho do UID
  Serial.print("Tamanho do UID: ");
  Serial.print(mfrc522.uid.size);
  Serial.println(" bytes");
  
  // Tipo PICC
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print("Tipo PICC: ");
  Serial.println(getCardType(piccType));
  
  // Para NTAG, detecta se é 213 ou 215 e lê todos os dados
  if (piccType == MFRC522::PICC_TYPE_MIFARE_UL) {
    int ntagType = detectNTAGType();
    Serial.print("Subtipo NTAG: ");
    Serial.println(getNTAGTypeName(ntagType));
    Serial.println("========================================\n");
    
    // Lê TODOS os dados da tag
    readAllNTAGData(ntagType);
  } else {
    Serial.println("========================================\n");
  }
}

// ============================================
// CONFIGURAÇÃO INICIAL
// ============================================
void setup() {
  // Inicializa comunicação serial
  Serial.begin(115200);
  while (!Serial); // Aguarda porta serial abrir (necessário para USB CDC)
  
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("╬══════════════════════════════════════════╬");
  Serial.print("║  Leitor RFID - ");
  Serial.print(BOARD_NAME);
  for(int i = strlen(BOARD_NAME); i < 26; i++) Serial.print(" ");
  Serial.println("║");
  Serial.println("║         MFRC522 + NTAG213/215         ║");
  Serial.println("╚══════════════════════════════════════════╝");
  Serial.println();
  
  // Configura pinos SPI customizados
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  
  // Inicializa MFRC522
  mfrc522.PCD_Init();
  
  // Aguarda inicialização
  delay(100);
  
  // Verifica comunicação com o MFRC522
  byte version = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  
  Serial.print("Versão do firmware MFRC522: 0x");
  Serial.println(version, HEX);
  
  if (version == 0x00 || version == 0xFF) {
    Serial.println("⚠️  ERRO: Falha na comunicação com MFRC522!");
    Serial.println("    Verifique as conexões:");
    
    #if defined(BOARD_ESP32S3_LCD)
      Serial.println("    - SDA (SS)  -> GPIO3  (Solda no Pad 6)");
      Serial.println("    - SCK       -> GPIO4  (Solda no Pad 7)");
      Serial.println("    - MOSI      -> GPIO5  (Solda no Pad 8)");
      Serial.println("    - MISO      -> GPIO42 (SD_MISO)");
      Serial.println("    - RST       -> GPIO0  (J9 Pin 12 + pull-up 10kΩ)");
      Serial.println("    - 3.3V      -> 3.3V   (J9 Pin 6)");
      Serial.println("    - GND       -> GND    (J9 Pin 1)");
      Serial.println("");
      Serial.println("    ⚠️ GPIO0 requer resistor pull-up 10kΩ!");
    #elif defined(BOARD_ESP32_WROOM)
      Serial.println("    - SDA (SS)  -> GPIO5");
      Serial.println("    - SCK       -> GPIO18");
      Serial.println("    - MOSI      -> GPIO23");
      Serial.println("    - MISO      -> GPIO19");
      Serial.println("    - RST       -> GPIO22");
      Serial.println("    - 3.3V      -> 3.3V");
      Serial.println("    - GND       -> GND");
    #else
      Serial.print("    - SDA (SS)  -> GPIO"); Serial.println(SS_PIN);
      Serial.print("    - SCK       -> GPIO"); Serial.println(SCK_PIN);
      Serial.print("    - MOSI      -> GPIO"); Serial.println(MOSI_PIN);
      Serial.print("    - MISO      -> GPIO"); Serial.println(MISO_PIN);
      Serial.print("    - RST       -> GPIO"); Serial.println(RST_PIN);
      Serial.println("    - 3.3V      -> 3.3V");
      Serial.println("    - GND       -> GND");
    #endif
    
    while (1); // Trava o programa
  }
  
  Serial.println("✓ MFRC522 inicializado com sucesso!");
  Serial.println();
  
  // Exibe detalhes do leitor
  mfrc522.PCD_DumpVersionToSerial();
  
  Serial.println("\n----------------------------------");
  Serial.println("Aguardando tags NFC...");
  Serial.println("Aproxime uma tag NTAG213 ou NTAG215");
  Serial.println("----------------------------------\n");
}

// ============================================
// LOOP PRINCIPAL
// ============================================
void loop() {
  // Verifica se há nova tag presente
  if (!mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  
  // Verifica se consegue ler a tag
  if (!mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  
  // Debounce: evita leituras múltiplas da mesma tag
  unsigned long currentTime = millis();
  if (currentTime - lastCardReadTime < CARD_READ_DELAY) {
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }
  lastCardReadTime = currentTime;
  
  // Exibe informações da tag
  displayCardInfo();
  
  // Para a comunicação com a tag
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  
  // Aguarda um tempo antes da próxima leitura
  Serial.println("Pronto para próxima leitura...\n");
}

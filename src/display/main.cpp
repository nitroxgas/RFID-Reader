#include <Arduino.h>
#include <TFT_eSPI.h>
#include "RoboEyesTFT_eSPI.h"
#include <TFT_eTouch.h>
#include <SPI.h>
#include <Preferences.h>
#include <nvs_flash.h>  // Para inicializar NVS
#include <FS.h>
#include <SD.h>

// LVGL para QR Code
#include <lvgl.h>

// UI do SquareLine Studio (imagens)
extern "C" {
  #include "ui/ui.h"
}

// Imagens (RGB888)
#include "BauTesouro.h"
#include "MoedaOuro.h"
#include "TesouroJaPilhado.h"

// Inclui protocolo compartilhado
#include "../common/protocol.h"

// ============================================
// ESP32-2432S028R (CYD) - Display Controller
// RoboEyes + QR Code LVGL
// ============================================

// Pinos UART (conectar ao Reader ESP32-WROOM)
#define UART_RX_PIN  27  // GPIO27 (CN1 Pin 3) <- Reader TX (GPIO17)
#define UART_TX_PIN  22  // GPIO22 (CN1 Pin 2) -> Reader RX (GPIO16)

// Pinos SPI para SD Card
#define SDSPI_CS    5
#define SDSPI_CLK   18
#define SDSPI_MOSI  23
#define SDSPI_MISO  19

// Pino Backlight do Display (CRÍTICO!)
#define TFT_BL  21       // GPIO21 - Backlight PWM

// Pinos Touch (CYD) - 4-wire resistivo
/* #define ETOUCH_CS    33
#define TOUCH_IRQ   36
#define TOUCH_MOSI  32
#define TOUCH_MISO  39
#define TOUCH_CLK   25 */

// Display
TFT_eSPI tft = TFT_eSPI();

// Touchscreen TFT_eTouch (mais estável)
// TFT_eTouch touch(TOUCH_CS, TOUCH_IRQ, TOUCH_MOSI, TOUCH_MISO, TOUCH_CLK);

SPIClass hSPI(HSPI);
TFT_eTouch<TFT_eSPI> touch(tft, ETOUCH_CS, 0xFF, hSPI); 

// RoboEyes (portrait mode: 240x320)
TFT_RoboEyes roboEyes(tft, true, 1);

// LVGL Display Buffer (serão alocados sob demanda)
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

// Display Mode States
enum DisplayMode {
  EYES_MODE,
  QRCODE_MODE,
  COIN_MODE,        // ⭐ NOVO: Moeda de ouro
  LOOTED_MODE       // ⭐ NOVO: Tesouro já pilhado
};

DisplayMode currentMode = EYES_MODE;
bool lvglInitialized = false;

// UI State
bool tagPresent = false;
String currentUID = "";
String currentURL = "";
String currentText = "";
ContentType currentType = CONTENT_RAW;

// QR Code variables
lv_obj_t *qr_code = NULL;
lv_obj_t *panel_qr = NULL;
lv_obj_t *qr_screen = NULL;
unsigned long qrCodeShowTime = 0;
const unsigned long QR_CODE_TIMEOUT = 180000;  // 3 minutos em ms

// ⭐ NOVO: Variáveis de controle do fluxo de verificação
const unsigned long REWARD_TIMEOUT = 60000;      // 1 minuto para moeda/mensagem
bool waitingForTagCheck = false;                 // Flag para verificação pendente
String pendingTagUID = "";	                       // UID da tag sendo verificada
unsigned long rewardShowTime = 0;                // Tempo de início da recompensa

// ⭐ NOVO: Preferences para armazenamento persistente
Preferences prefs;
const char* PREFS_NAMESPACE = "rfid_tags";
const char* PREFS_COUNT_KEY = "count";
const char* PREFS_TAG_PREFIX = "tag_";

// ⭐ NOVO: Tag especial para admin/debug
const String ADMIN_TAG_UID = "0431430F320289";
int consecutiveAdminReads = 0;
String lastReadUID = "";
bool showingResetMessage = false;
unsigned long adminMessageShowTime = 0;         // Tempo de início da mensagem de admin
const unsigned long ADMIN_MESSAGE_TIMEOUT = 30000; // 30 segundos para mensagem de admin

// Mood change variables
unsigned long lastMoodChange = 0;
const unsigned long MOOD_CHANGE_INTERVAL = 30000;  // 1 minuto
const uint8_t MOODS[] = {0, TIRED, ANGRY, HAPPY};  // 0 = DEFAULT
const int NUM_MOODS = 4;

// Touch variables
bool touchEnabled = false;
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE = 300;  // 300ms debounce
bool touchProcessing = false;  // Flag para evitar múltiplas leituras


// ============================================
// FUNÇÕES LVGL - Display Driver
// ============================================

/**
 * Callback para flush do display
 */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  
  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t*)&color_p->full, w * h, false);
  tft.endWrite();
  
  lv_disp_flush_ready(disp);
}
/**
 * Inicializa LVGL
 */
void initLVGL() {
  if (lvglInitialized) return;
  
  Serial.println("\n🔧 Inicializando LVGL...");
  
  lv_init();
  
  // Calcula tamanho do buffer (largura * linhas)
  size_t bufferLines = 20;
  size_t bufferSize = TFT_WIDTH * bufferLines * sizeof(lv_color_t);
  
  Serial.printf("  ├─ Alocando buffers: %d x %d linhas = %d pixels (%d bytes)\n", 
                TFT_WIDTH, bufferLines, TFT_WIDTH * bufferLines, bufferSize);
  
  // Aloca buffers dinamicamente
  if (buf1 == NULL) {
    buf1 = (lv_color_t*)heap_caps_malloc(bufferSize, MALLOC_CAP_DMA);
    if (buf1 == NULL) {
      Serial.printf("❌ ERRO: Falha ao alocar buf1 (%d bytes)!\n", bufferSize);
      Serial.printf("  └─ Heap livre: %d bytes\n", ESP.getFreeHeap());
      return;
    }
    Serial.printf("  ├─ buf1 alocado em: %p\n", buf1);
  }
  
  if (buf2 == NULL) {
    buf2 = (lv_color_t*)heap_caps_malloc(bufferSize, MALLOC_CAP_DMA);
    if (buf2 == NULL) {
      Serial.printf("❌ ERRO: Falha ao alocar buf2 (%d bytes)!\n", bufferSize);
      Serial.printf("  └─ Heap livre: %d bytes\n", ESP.getFreeHeap());
      return;
    }
    Serial.printf("  ├─ buf2 alocado em: %p\n", buf2);
  }
  
  // Configura buffer duplo para melhor performance
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, TFT_WIDTH * bufferLines);
  Serial.printf("  ├─ draw_buf inicializado: %d pixels\n", TFT_WIDTH * bufferLines);
  
  // Registra driver do display
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &draw_buf;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  lv_disp_drv_register(&disp_drv);
  
  Serial.printf("  ├─ Display driver: %dx%d\n", disp_drv.hor_res, disp_drv.ver_res);
  Serial.printf("  └─ Heap livre após LVGL: %d bytes\n", ESP.getFreeHeap());
  Serial.println("✅ LVGL inicializado com sucesso!\n");
  
  lvglInitialized = true;
}

// ============================================
// UI TEMPORÁRIA (COMENTADA - Usando SquareLine)
// ============================================

/* UI TEMPORÁRIA - DESABILITADA
lv_obj_t *screen_main;
lv_obj_t *label_title;
lv_obj_t *label_status;
lv_obj_t *label_uid;
lv_obj_t *label_content;
lv_obj_t *panel_qr;
lv_obj_t *qr_code;

void createTemporaryUI() {
  Serial.println("Criando UI temporária...");
  screen_main = lv_obj_create(NULL);
  lv_scr_load(screen_main);
  label_title = lv_label_create(screen_main);
  lv_label_set_text(label_title, "RFID Reader System");
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x0080FF), 0);
  label_status = lv_label_create(screen_main);
  lv_label_set_text(label_status, "Aguardando leitor...");
  lv_obj_align(label_status, LV_ALIGN_TOP_MID, 0, 45);
  lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, 0);
  label_uid = lv_label_create(screen_main);
  lv_label_set_text(label_uid, "");
  lv_obj_align(label_uid, LV_ALIGN_CENTER, 0, -60);
  lv_obj_set_style_text_font(label_uid, &lv_font_montserrat_12, 0);
  label_content = lv_label_create(screen_main);
  lv_label_set_text(label_content, "");
  lv_obj_align(label_content, LV_ALIGN_CENTER, 0, -30);
  lv_obj_set_style_text_font(label_content, &lv_font_montserrat_10, 0);
  lv_label_set_long_mode(label_content, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(label_content, 280);
  lv_obj_set_style_text_align(label_content, LV_TEXT_ALIGN_CENTER, 0);
  panel_qr = lv_obj_create(screen_main);
  lv_obj_set_size(panel_qr, 200, 200);
  lv_obj_align(panel_qr, LV_ALIGN_CENTER, 0, 50);
  lv_obj_set_style_bg_color(panel_qr, lv_color_white(), 0);
  lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
  qr_code = lv_qrcode_create(panel_qr, 180, lv_color_black(), lv_color_white());
  lv_obj_center(qr_code);
  Serial.println("UI temporária criada!");
  Serial.println("✨ Substitua por código do SquareLine Studio em display/ui/");
}
*/

// ============================================
// DESENHO DO BAÚ DE TESOURO - TFT DIRETO
// ============================================

/**
 * Exibe imagem do baú de tesouro diretamente com TFT_eSPI
 */
void drawTreasureChest() {
  Serial.println("🎨 Desenhando baú de tesouro (RGB888->RGB565)...");
  
  // CRÍTICO: Garante swap correto para RGB565
  tft.setSwapBytes(true);
  
  // Calcula posição centralizada
  int16_t x_offset = (tft.width() - BAUTESOURO_WIDTH) / 2;
  int16_t y_offset = (tft.height() - BAUTESOURO_HEIGHT) / 2;
  
  // Buffer para uma linha de pixels em RGB565
  uint16_t* lineBuffer = (uint16_t*)malloc(BAUTESOURO_WIDTH * sizeof(uint16_t));
  if (lineBuffer == NULL) {
    Serial.println("❌ Erro ao alocar buffer para linha!");
    return;
  }
  
  // Desenha linha por linha
  for (int y = 0; y < BAUTESOURO_HEIGHT; y++) {
    // Converte linha de RGB888 para RGB565
    for (int x = 0; x < BAUTESOURO_WIDTH; x++) {
      int pixelIndex = y * BAUTESOURO_WIDTH + x;
      
      // Lê valor RGB888 de 32-bit (0x00RRGGBB) da PROGMEM
      uint32_t rgb888 = pgm_read_dword(&BauTesouro[pixelIndex]);
      
      // Extrai componentes RGB
      uint8_t r = (rgb888 >> 16) & 0xFF;
      uint8_t g = (rgb888 >> 8) & 0xFF;
      uint8_t b = rgb888 & 0xFF;
      
      // Converte para RGB565
      lineBuffer[x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    
    // Envia linha para o display
    tft.pushImage(x_offset, y_offset + y, BAUTESOURO_WIDTH, 1, lineBuffer);
  }
  
  free(lineBuffer);
  Serial.println("✅ Baú desenhado com sucesso!");
}

// ============================================
// SISTEMA DE ARMAZENAMENTO NVS (PREFERENCES)
// ============================================

/**
 * Verifica se uma tag já foi lida anteriormente
 */
bool isTagAlreadyRead(String uid) {
  prefs.begin(PREFS_NAMESPACE, true); // read-only
  int count = prefs.getInt(PREFS_COUNT_KEY, 0);
  
  for (int i = 0; i < count; i++) {
    String key = String(PREFS_TAG_PREFIX) + String(i);
    String storedUID = prefs.getString(key.c_str(), "");
    if (storedUID == uid) {
      prefs.end();
      return true; // Tag já existe
    }
  }
  
  prefs.end();
  return false; // Tag nova
}

/**
 * Salva uma tag como lida
 */
void saveTagAsRead(String uid) {
  prefs.begin(PREFS_NAMESPACE, false); // read-write
  
  int count = prefs.getInt(PREFS_COUNT_KEY, 0);
  String key = String(PREFS_TAG_PREFIX) + String(count);
  
  prefs.putString(key.c_str(), uid);
  prefs.putInt(PREFS_COUNT_KEY, count + 1);
  
  prefs.end();
  
  Serial.printf("✅ Tag salva! Total de tags lidas: %d\n", count + 1);
}

/**
 * Retorna quantidade de tags lidas
 */
int getReadTagsCount() {
  prefs.begin(PREFS_NAMESPACE, true);
  int count = prefs.getInt(PREFS_COUNT_KEY, 0);
  prefs.end();
  return count;
}

/**
 * Limpa todas as tags armazenadas (opcional, para debug)
 */
void clearAllTags() {
  prefs.begin(PREFS_NAMESPACE, false);
  prefs.clear();
  prefs.end();
  Serial.println("⚠️ Todas as tags foram apagadas!");
}

/**
 * Lista todas as tags armazenadas via Serial
 */
void listAllTags() {
  Serial.println("\n📊 ========== LISTA DE TAGS LIDAS ===========");
  
  prefs.begin(PREFS_NAMESPACE, true); // read-only
  int count = prefs.getInt(PREFS_COUNT_KEY, 0);
  
  Serial.printf("📊 Total de tags armazenadas: %d\n\n", count);
  
  if (count == 0) {
    Serial.println("⚠️ Nenhuma tag armazenada ainda.");
  } else {
    Serial.println("├───┬──────────────────────────");
    Serial.println("│ # │ UID                  │");
    Serial.println("├───┼──────────────────────────");
    
    for (int i = 0; i < count; i++) {
      String key = String(PREFS_TAG_PREFIX) + String(i);
      String uid = prefs.getString(key.c_str(), "N/A");
      Serial.printf("│%3d│ %-20s│\n", i+1, uid.c_str());
    }
    
    Serial.println("└───┴──────────────────────────");
  }
  
  prefs.end();
  Serial.println("📊 =========================================\n");
}

/**
 * Faz backup das tags para arquivo no SD Card
 * Retorna true se sucesso, false se falha
 */
bool backupTagsToSD() {
  Serial.println("\n💾 Iniciando backup de tags para SD Card...");
  
  // Inicializa SD Card se não estiver inicializado
  if (!SD.begin(SDSPI_CS, hSPI)) {
    Serial.println("❌ Erro: SD Card não detectado!");
    return false;
  }
  
  // Lê tags da NVS
  prefs.begin(PREFS_NAMESPACE, true);
  int count = prefs.getInt(PREFS_COUNT_KEY, 0);
  
  if (count == 0) {
    Serial.println("⚠️ Nenhuma tag para fazer backup.");
    prefs.end();
    return false;
  }
  
  // Cria nome de arquivo com timestamp
  String filename = "/rfid_backup_" + String(millis()) + ".txt";
  
  File file = SD.open(filename.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("❌ Erro ao criar arquivo de backup!");
    prefs.end();
    return false;
  }
  
  // Escreve header
  file.println("========================================");
  file.println("RFID TAGS BACKUP");
  file.println("Sistema: ESP32 RFID Reader Display CYD");
  file.printf("Data: %lu ms\n", millis());
  file.printf("Total de tags: %d\n", count);
  file.println("========================================\n");
  
  // Escreve cada tag
  for (int i = 0; i < count; i++) {
    String key = String(PREFS_TAG_PREFIX) + String(i);
    String uid = prefs.getString(key.c_str(), "N/A");
    file.printf("%d,%s\n", i+1, uid.c_str());
  }
  
  file.println("\n========================================");
  file.println("FIM DO BACKUP");
  file.println("========================================");
  
  file.close();
  prefs.end();
  
  Serial.printf("✅ Backup criado com sucesso!\n");
  Serial.printf("📁 Arquivo: %s\n", filename.c_str());
  Serial.printf("📊 %d tags salvas\n\n", count);
  
  return true;
}

/**
 * Exibe mensagem simples na tela (sem imagens)
 */
void showSimpleMessage(const char* line1, const char* line2 = nullptr, 
                       const char* line3 = nullptr, const char* line4 = nullptr) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  
  int y = 80;
  int lineSpacing = 30;
  
  if (line1) tft.drawString(line1, tft.width()/2, y);
  if (line2) tft.drawString(line2, tft.width()/2, y + lineSpacing);
  if (line3) tft.drawString(line3, tft.width()/2, y + lineSpacing*2);
  if (line4) tft.drawString(line4, tft.width()/2, y + lineSpacing*3);
}

// ============================================
// DESENHO DA MOEDA DE OURO
// ============================================

/**
 * Exibe imagem da moeda de ouro diretamente com TFT_eSPI
 */
void drawGoldenCoin() {
  Serial.println("🪙 Desenhando moeda de ouro (RGB888->RGB565)...");
  
  tft.setSwapBytes(true);
  
  // Calcula posição centralizada
  int16_t x_offset = (tft.width() - MOEDAOURO_WIDTH) / 2;
  int16_t y_offset = (tft.height() - MOEDAOURO_HEIGHT) / 2;
  
  // Buffer para uma linha de pixels em RGB565
  uint16_t* lineBuffer = (uint16_t*)malloc(MOEDAOURO_WIDTH * sizeof(uint16_t));
  if (lineBuffer == NULL) {
    Serial.println("❌ Erro ao alocar buffer para linha!");
    return;
  }
  
  // Desenha linha por linha
  for (int y = 0; y < MOEDAOURO_HEIGHT; y++) {
    for (int x = 0; x < MOEDAOURO_WIDTH; x++) {
      int pixelIndex = y * MOEDAOURO_WIDTH + x;
      uint32_t rgb888 = pgm_read_dword(&MoedaOuro[pixelIndex]);
      
      uint8_t r = (rgb888 >> 16) & 0xFF;
      uint8_t g = (rgb888 >> 8) & 0xFF;
      uint8_t b = rgb888 & 0xFF;
      
      lineBuffer[x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    
    tft.pushImage(x_offset, y_offset + y, MOEDAOURO_WIDTH, 1, lineBuffer);
  }
  
  free(lineBuffer);
  Serial.println("✅ Moeda de ouro desenhada com sucesso!");
}

// ============================================
// DESENHO DA MENSAGEM "TESOURO JÁ PILHADO"
// ============================================

/**
 * Exibe imagem de tesouro já pilhado com TFT_eSPI
 */
void drawLootedMessage() {
  Serial.println("☠️ Desenhando mensagem de tesouro pilhado (RGB888->RGB565)...");
  
  tft.setSwapBytes(true);
  
  // Calcula posição centralizada
  int16_t x_offset = (tft.width() - TESOUROPILHADO_WIDTH) / 2;
  int16_t y_offset = (tft.height() - TESOUROPILHADO_HEIGHT) / 2;
  
  // Buffer para uma linha de pixels em RGB565
  uint16_t* lineBuffer = (uint16_t*)malloc(TESOUROPILHADO_WIDTH * sizeof(uint16_t));
  if (lineBuffer == NULL) {
    Serial.println("❌ Erro ao alocar buffer para linha!");
    return;
  }
  
  // Desenha linha por linha
  for (int y = 0; y < TESOUROPILHADO_HEIGHT; y++) {
    for (int x = 0; x < TESOUROPILHADO_WIDTH; x++) {
      int pixelIndex = y * TESOUROPILHADO_WIDTH + x;
      uint32_t rgb888 = pgm_read_dword(&TesouroJaPilhado[pixelIndex]);
      
      uint8_t r = (rgb888 >> 16) & 0xFF;
      uint8_t g = (rgb888 >> 8) & 0xFF;
      uint8_t b = rgb888 & 0xFF;
      
      lineBuffer[x] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    
    tft.pushImage(x_offset, y_offset + y, TESOUROPILHADO_WIDTH, 1, lineBuffer);
  }
  
  free(lineBuffer);
  Serial.println("✅ Mensagem de tesouro pilhado desenhada com sucesso!");
  
  // Adiciona texto sobre a imagem
  /* tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("AhhhhRRR!", tft.width()/2, 40);
  tft.setTextSize(1);
  tft.drawString("Voce esta tentando", tft.width()/2, 70);
  tft.drawString("pilhar o tesouro", tft.width()/2, 85);
  tft.drawString("novamente?", tft.width()/2, 100);
  tft.drawString("Contente-se com", tft.width()/2, 120);
  tft.drawString("seu quinhao!", tft.width()/2, 135); */
}

// ============================================
// QR CODE - MODO ALTERNATIVO
// ============================================

/**
 * Inicializa LVGL sob demanda (só quando precisar do QR Code)
 */
void initializeLVGLIfNeeded() {
  if (!lvglInitialized) {
    Serial.println("📦 Inicializando LVGL para QR Code...");
    initLVGL();
    lvglInitialized = true;
    Serial.println("✅ LVGL inicializado!");
  }
}

/**
 * Cria tela QR Code (LVGL)
 */
void createQRCodeScreen() {
  initializeLVGLIfNeeded();
  
  Serial.println("📱 Criando tela de QR Code...");
  
  // Cria tela preta para QR Code
  qr_screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(qr_screen, lv_color_black(), 0);
  
  // Panel branco para QR Code centralizado
  panel_qr = lv_obj_create(qr_screen);
  lv_obj_set_size(panel_qr, 188, 188);
  lv_obj_align(panel_qr, LV_ALIGN_CENTER, 0, -9);
  lv_obj_set_style_bg_color(panel_qr, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(panel_qr, 255, 0);
  lv_obj_set_style_border_width(panel_qr, 3, 0);
  lv_obj_set_style_border_color(panel_qr, lv_color_hex(0x2095F6), 0);
  lv_obj_clear_flag(panel_qr, LV_OBJ_FLAG_SCROLLABLE);
  
  // QR Code 200x200
  qr_code = lv_qrcode_create(panel_qr, 163, lv_color_black(), lv_color_white());
  lv_obj_center(qr_code);
  
  Serial.println("✅ Tela QR Code criada!");
}

/**
 * Alterna para modo RoboEyes
 */
void switchToEyesMode() {
  Serial.println("👀 Alternando para modo Eyes...");
  
  // Garante swap correto para RoboEyes
  tft.setSwapBytes(true);
  
  currentMode = EYES_MODE;
  tft.fillScreen(TFT_BLACK);
  // RoboEyes continuará automaticamente no loop
  Serial.println("✅ Modo Eyes ativo!");
}

/**
 * Alterna para modo QR Code
 */
void switchToQRCodeMode(const String& url) {
  Serial.println("📱 Alternando para modo QR Code...");
  
  currentMode = QRCODE_MODE;
  
  // Desenha baú de tesouro com TFT_eSPI (RGB888 -> RGB565)
  tft.fillScreen(TFT_BLACK);
  drawTreasureChest();
  
  // Aguarda 500ms
  delay(1000);
  
  // Inicializa LVGL para QR Code
  initializeLVGLIfNeeded();
  
  // 📱 Agora exibe o QR Code
  Serial.println("📱 Exibindo QR Code...");
  
  // Cria tela se não existir
  if (qr_screen == NULL) {
    createQRCodeScreen();
  }
  
  // Atualiza QR Code com a URL
  lv_qrcode_update(qr_code, url.c_str(), url.length());
  
  // Carrega tela QR Code
  lv_scr_load(qr_screen);
  
  // Registra tempo
  qrCodeShowTime = millis();
  
  Serial.println("✅ QR Code exibido (timeout: 3 min)");
}

/**
 * Alterna para modo Moeda de Ouro
 */
void switchToCoinMode() {
  Serial.println("🪙 Alternando para modo Moeda de Ouro...");
  
  currentMode = COIN_MODE;
  tft.fillScreen(TFT_BLACK);
  drawGoldenCoin();
  
  // Registra tempo de início
  rewardShowTime = millis();
  
  Serial.println("✅ Moeda de ouro exibida (timeout: 1 min)");
}

/**
 * Alterna para modo Tesouro Já Pilhado
 */
void switchToLootedMode() {
  Serial.println("☠️ Alternando para modo Tesouro Já Pilhado...");
  
  currentMode = LOOTED_MODE;
  tft.fillScreen(TFT_BLACK);
  drawLootedMessage();
  
  // Registra tempo de início
  rewardShowTime = millis();
  
  Serial.println("✅ Mensagem de tesouro pilhado exibida (timeout: 1 min)");
}

/**
 * Verifica e processa a tag após timeout ou toque
 */
void checkAndRewardTag() {
  if (pendingTagUID.length() == 0) return;
  
  Serial.println("\n🔍 Verificando tag...");
  Serial.println("  ├─ UID: " + pendingTagUID);
  
  if (isTagAlreadyRead(pendingTagUID)) {
    // Tag já foi lida - mensagem de tesouro pilhado
    Serial.println("  └─ ⚠️ Tag já foi lida anteriormente!");
    switchToLootedMode();
  } else {
    // Tag nova - salva e mostra moeda
    Serial.println("  ├─ ✅ Tag nova! Salvando...");
    saveTagAsRead(pendingTagUID);
    Serial.println("  └─ 🎆 Recompensa: Moeda de Ouro!");
    switchToCoinMode();
  }
  
  // Limpa flags
  waitingForTagCheck = false;
  pendingTagUID = "";
  qrCodeShowTime = 0;
}

// ============================================
// FUNÇÕES DE MUDANÇA DE HUMOR
// ============================================

/**
 * Muda o humor dos olhos aleatoriamente
 */
void changeRandomMood() {
  // Escolhe humor aleatório
  uint8_t randomMood = MOODS[random(NUM_MOODS)];
  
  // Aplica humor
  if (randomMood == 0) {
    // DEFAULT - limpa todos os humores
    roboEyes.setMood(0);
    Serial.println("👀 Humor alterado: DEFAULT");
  } else {
    roboEyes.setMood(randomMood);
    switch(randomMood) {
      case TIRED:
        Serial.println("👀 Humor alterado: TIRED (Cansado)");
        break;
      case ANGRY:
        Serial.println("👀 Humor alterado: ANGRY (Bravo)");
        break;
      case HAPPY:
        Serial.println("👀 Humor alterado: HAPPY (Feliz)");
        break;
    }
  }
}

// ============================================
// FUNÇÕES DE TOUCH
// ============================================

/**
 * Verifica e processa toques na tela
 */
void handleTouch() {
  if (!touchEnabled || touchProcessing) return;
  int16_t t_x , t_y; 
  // TFT_eTouch: Método simples e eficaz
  if (touch.getXY(t_x, t_y)) {
    // Debounce - evita múltiplos toques
    if (millis() - lastTouchTime < TOUCH_DEBOUNCE) {
      return;
    }
    
    // Marca que está processando
    touchProcessing = true;    
    
    // Valida coordenadas
    if (t_x > tft.width() || t_y > tft.height()) {
      Serial.printf("⚠️ Touch fora da tela: (%d, %d)\n", t_x, t_y);
      touchProcessing = false;
      return;
    }
    
    lastTouchTime = millis();
    
    Serial.printf("👆 Touch válido em: (%d, %d)\n", t_x, t_y);
    
    // Ação baseada no modo atual
    if (currentMode == QRCODE_MODE) {
      // ⭐ NOVO: Touch durante exibição do tesouro
      if (waitingForTagCheck) {
        // Força verificação imediata
        Serial.println("👆 Touch durante tesouro - verificando tag...");
        checkAndRewardTag();
      } else {
        // Volta para olhos
        Serial.println("📱 Touch no QR Code - voltando aos olhos...");
        switchToEyesMode();
      }
      
    } else if (currentMode == COIN_MODE || currentMode == LOOTED_MODE) {
      // ⭐ NOVO: Touch na moeda ou mensagem - volta aos olhos
      Serial.println("👆 Touch na recompensa - voltando aos olhos...");
      switchToEyesMode();
      rewardShowTime = 0;  // Reseta timer
      
    } else if (currentMode == EYES_MODE) {
      // ⭐ NOVO: Se está mostrando mensagem de reset/admin
      if (showingResetMessage) {
        Serial.println("👆 Touch na mensagem de admin - voltando aos olhos...");
        showingResetMessage = false;
        adminMessageShowTime = 0;  // Reseta timer
        switchToEyesMode();
        touchProcessing = false;
        return;
      }
      
      // Se está mostrando olhos, executa animação confused e muda humor
      Serial.println("👀 Touch nos olhos - executando animação confused...");
      roboEyes.anim_confused();
      
      // Muda para um humor aleatório após a animação
      delay(800); // Aguarda animação confused
      changeRandomMood();
      Serial.println("  └─ Humor alterado!");
    }
    
    // Aguarda liberar o toque
    delay(100);
    touchProcessing = false;
  }
}

// ============================================
// FUNÇÕES DE ATUALIZAÇÃO DA UI
// ============================================

/**
 * Mostra informações da tag detectada
 */
void showTagInfo(const TagMessage& tag) {
  Serial.println("📱 Tag detectada!");
  Serial.println("  ├─ UID: " + tag.uid);
  
  // ⭐ NOVO: Verifica se é a tag especial de admin
  if (tag.uid == ADMIN_TAG_UID) {
    Serial.println("  ├─ 🔑 TAG ADMIN DETECTADA!");
    
    // Verifica se é leitura consecutiva
    if (lastReadUID == ADMIN_TAG_UID) {
      consecutiveAdminReads++;
      Serial.printf("  ├─ Leituras consecutivas: %d/3\n", consecutiveAdminReads);
    } else {
      consecutiveAdminReads = 1;
      Serial.println("  ├─ Primeira leitura admin");
    }
    
    lastReadUID = tag.uid;
    
    // a) Sempre lista as tags no console
    Serial.println("  ├─ Listando tags armazenadas...");
    listAllTags();
    
    // b) Se 3 leituras consecutivas: backup, limpa e mostra mensagem
    if (consecutiveAdminReads >= 3) {
      Serial.println("  └─ ⚠️ 3 LEITURAS CONSECUTIVAS - INICIANDO RESET!");
      Serial.println();
      
      // Faz backup
      bool backupOk = backupTagsToSD();
      
      // Limpa a tabela principal
      clearAllTags();
      
      // Exibe mensagem na tela
      showSimpleMessage(
        "LISTA ZERADA",
        backupOk ? "Backup: OK" : "Backup: FALHOU",
        "Tags apagadas",
        "Toque para voltar"
      );
      
      // Reseta contador
      consecutiveAdminReads = 0;
      
      // Aguarda toque para voltar
      Serial.println("⏳ Aguardando toque na tela para voltar aos olhos...");
      
      // Flag especial para não processar verificação normal
      waitingForTagCheck = false;
      pendingTagUID = "";
      showingResetMessage = true;  // Flag para handleTouch
      
    } else {
      Serial.println("  └─ Leia mais " + String(3 - consecutiveAdminReads) + "x para resetar");
      
      // Executa animação
      roboEyes.anim_laugh();
      delay(500);
      
      // ⭐ NOVO: Mostra mensagem visual na tela
      prefs.begin(PREFS_NAMESPACE, true); // read-only
      int tagsCount = prefs.getInt(PREFS_COUNT_KEY, 0);
      prefs.end();
      
      int timesToClear = 3 - consecutiveAdminReads;
      
      // Cria strings temporárias para conversão
      String line2 = String(tagsCount) + " tags detected";
      String line3 = String(timesToClear) + "x times to clear";
      
      showSimpleMessage(
        "Ye Captain!",
        line2.c_str(),
        line3.c_str(),
        "Touch to continue"
      );
      
      showingResetMessage = true;  // Aguarda toque para voltar
      adminMessageShowTime = millis();  // Marca tempo de início
      Serial.println("⏳ Aguardando toque ou 30 segundos para voltar aos olhos...");
    }
    
    tagPresent = true;
    return; // Não processa o resto
  }
  
  // Se não é tag admin, reseta contador de admin
  if (lastReadUID == ADMIN_TAG_UID) {
    consecutiveAdminReads = 0;
  }
  lastReadUID = tag.uid;
  
  // Mostra conteúdo baseado no tipo
  if (tag.type == CONTENT_URL && tag.url.length() > 0) {
    Serial.println("  ├─ Tipo: URL NDEF");
    Serial.println("  ├─ URL: " + tag.url);
    Serial.println("  └─ Exibindo animação e baú de tesouro...");
    
    // Executa animação laugh
    roboEyes.anim_laugh();
    delay(500); // Aguarda animação
    
    // switchToQRCodeMode irá mostrar baú + QR Code
    switchToQRCodeMode(tag.url);
    
    // ⭐ NOVO: Registra tag para verificação posterior
    waitingForTagCheck = true;
    pendingTagUID = tag.uid;
    Serial.println("⏳ Aguardando 3 minutos ou toque para verificar tag...");
    
  } else if (tag.type == CONTENT_TEXT && tag.text.length() > 0) {
    Serial.println("  ├─ Tipo: Texto");
    Serial.println("  └─ Conteúdo: " + tag.text);
    // Texto não exibe QR Code, mantém olhos
    
  } else {
    Serial.println("  └─ Tipo: Dados brutos (não-NDEF)");
    // Dados brutos não exibem QR Code, mantém olhos
  }
  
  tagPresent = true;
}

/**
 * Limpa display e volta para estado inicial (RoboEyes)
 */
void clearDisplay() {
  Serial.println("🔄 Limpando display...");
  
  // Volta para modo olhos
  if (currentMode == QRCODE_MODE) {
    switchToEyesMode();
  }
  
  qrCodeShowTime = 0;
  tagPresent = false;
  
  Serial.println("✅ Display limpo - voltando para RoboEyes");
}

/**
 * Atualiza status da conexão
 */
void updateConnectionStatus(String status) {
  // Log apenas - UI do SquareLine não tem label de status para atualizar
  Serial.print("🔗 Status: ");
  Serial.println(status);
}

/**
 * Verifica timeout do QR Code (3 minutos)
 */
void checkQRCodeTimeout() {
  // ⭐ NOVO: Se está aguardando verificação de tag
  if (waitingForTagCheck && qrCodeShowTime > 0 && 
      (millis() - qrCodeShowTime >= QR_CODE_TIMEOUT)) {
    Serial.println("⏰ Timeout de 3 minutos - verificando tag...");
    checkAndRewardTag();
    return;
  }
  
  // Se QR Code está visível e passou do timeout (sem verificação pendente)
  if (currentMode == QRCODE_MODE && qrCodeShowTime > 0 && 
      (millis() - qrCodeShowTime >= QR_CODE_TIMEOUT) && !waitingForTagCheck) {
    Serial.println("⏰ Timeout: Retornando para RoboEyes após 3 minutos");
    switchToEyesMode();
    qrCodeShowTime = 0;
  }
}

/**
 * Verifica timeout da recompensa (moeda/mensagem - 1 minuto)
 */
void checkRewardTimeout() {
  if ((currentMode == COIN_MODE || currentMode == LOOTED_MODE) && rewardShowTime > 0) {
    // Verifica timeout de 1 minuto
    if (millis() - rewardShowTime >= REWARD_TIMEOUT) {
      Serial.println("⏰ Timeout de recompensa - voltando aos olhos");
      switchToEyesMode();
      rewardShowTime = 0;
    }
  }
}

/**
 * ⭐ NOVO: Verifica timeout da mensagem de admin (30 segundos)
 */
void checkAdminMessageTimeout() {
  if (showingResetMessage && adminMessageShowTime > 0) {
    // Verifica timeout de 30 segundos
    if (millis() - adminMessageShowTime >= ADMIN_MESSAGE_TIMEOUT) {
      Serial.println("⏰ Timeout de mensagem admin - voltando aos olhos");
      showingResetMessage = false;
      adminMessageShowTime = 0;
      switchToEyesMode();
    }
  }
}

// ============================================
// COMUNICAÇÃO UART
// ============================================

/**
 * Processa mensagem recebida do Reader via UART
 */
void processUARTMessage(String message) {
  message.trim();
  
  if (message.length() == 0) {
    return;
  }
  
  Serial.print("📩 UART << ");
  Serial.println(message);
  
  String msgType = CommProtocol::getMessageType(message);
  
  if (msgType == MSG_TAG) {
    // Decodifica e mostra tag
    TagMessage tag = CommProtocol::decodeTag(message);
    
    // Salva estado atual
    currentUID = tag.uid;
    currentURL = tag.url;
    currentText = tag.text;
    currentType = tag.type;
    tagPresent = true;
    
    // Atualiza UI
    showTagInfo(tag);
    
    // Envia ACK
    Serial1.println(CommProtocol::encodeAck());
    
  } else if (msgType == MSG_STATUS) {
    // Atualiza status
    int sep = message.indexOf('|');
    String status = message.substring(sep + 1);
    updateConnectionStatus(status);
    
    // Envia ACK
    Serial1.println(CommProtocol::encodeAck());
    
  } else if (msgType == MSG_ERROR) {
    // Mostra erro
    int sep = message.indexOf('|');
    String error = "ERRO: " + message.substring(sep + 1);
    updateConnectionStatus(error);
    
  } else {
    Serial.print("⚠️  Mensagem desconhecida: ");
    Serial.println(message);
  }
}

/**
 * Verifica e processa mensagens UART
 */
void checkUARTMessages() {
  while (Serial1.available()) {
    String message = Serial1.readStringUntil('\n');
    processUARTMessage(message);
  }
}

// ============================================
// TASK AUTO-CLEAR (Limpa display após 5s)
// ============================================

unsigned long tagDisplayTime = 0;
const unsigned long TAG_DISPLAY_DURATION = 5000; // 5 segundos

void checkAutoClear() {
  if (tagPresent && tagDisplayTime > 0) {
    if (millis() - tagDisplayTime > TAG_DISPLAY_DURATION) {
      clearDisplay();
      tagDisplayTime = 0;
    }
  }
}

// ============================================
// SETUP
// ============================================

void setup() {
  // Serial para debug
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n\n");
  Serial.println("╔══════════════════════════════════════════╗");
  Serial.println("║   ESP32-2432S028R (CYD) Display         ║");
  Serial.println("║   RFID Reader System                     ║");
  Serial.println("╚══════════════════════════════════════════╝");
  Serial.println();
  
  // Inicializa UART para Reader
  Serial.print("🔗 Inicializando UART (TX: GPIO");
  Serial.print(UART_TX_PIN);
  Serial.print(", RX: GPIO");
  Serial.print(UART_RX_PIN);
  Serial.println(")...");
  
  Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  delay(100);
  
  // Envia status inicial
  Serial1.println(CommProtocol::encodeStatus("DISPLAY_READY"));
  Serial.println("📤 UART >> STATUS|DISPLAY_READY");
  
  // Inicializa display
  Serial.println("📺 Inicializando TFT Display...");
  
  // CRÍTICO: Liga o backlight ANTES de inicializar TFT
  Serial.println("  ↳ Ligando backlight (GPIO21)...");
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // Liga backlight 100%
  delay(100);
  
  // Inicializa TFT
  Serial.println("  ↓ Inicializando SPI e TFT...");
  tft.init();
  tft.invertDisplay(1);
  tft.setRotation(2);  // Mudado de 4 para 1 para remover espelhamento
  
  tft.setSwapBytes(true);
  
  // Configura gamma
  tft.writecommand(ILI9341_GAMMASET);
  tft.writedata(2);
  delay(120);
  tft.writecommand(ILI9341_GAMMASET);
  tft.writedata(1); 
  
  tft.fillScreen(TFT_BLACK);
  
  Serial.printf("✅ TFT Display inicializado! Resolução: %dx%d (rotação %d)\n", 
                tft.width(), tft.height(), tft.getRotation());
  Serial.printf("  └─ Heap livre: %d bytes\n", ESP.getFreeHeap());
  
  // Inicializa Touchscreen TFT_eTouch
  Serial.println("\n👆 Inicializando Touchscreen (TFT_eTouch)...");
  
  // Inicializa touchscreen
  hSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, ETOUCH_CS);
  touch.init();
  
  TFT_eTouchBase::Calibation calibation = { 233, 3785, 3731, 120, 2 };
  touch.setCalibration(calibation);
  
  touchEnabled = true;
  Serial.println("✅ Touchscreen TFT_eTouch inicializado!");
  Serial.println("  ├─ Biblioteca: TFT_eTouch (estável)");
  Serial.println("  ├─ Calibração: Automática");
  Serial.printf("  └─ Rotação: %d\n", tft.getRotation());
  
  // Inicializa RoboEyes
  Serial.println("\n👀 Inicializando RoboEyes...");
  roboEyes.setScreenSize(235, 235);
  roboEyes.setWidth(50,50);
  roboEyes.setHeight(50,50);
  roboEyes.setBorderradius(5,5);
  //roboEyes.setSpacebetween(10);
  roboEyes.setColors(TFT_BLUE, TFT_BLACK);
  roboEyes.setIdleMode(true, 5, 2);
  roboEyes.setCyclops(true);
  roboEyes.setCuriosity(true);
  roboEyes.begin();
  roboEyes.setAutoblinker(true, 3, 4);  // Piscar a cada 3 segundos
  Serial.println("✅ RoboEyes inicializado! Piscarão a cada 3s");
  
  // Inicializa gerador de números aleatórios
  randomSeed(analogRead(0));
  
  // Define primeiro humor aleatório
  changeRandomMood();
  lastMoodChange = millis();
  
  // ⭐ NOVO: Inicializa sistema de armazenamento
  Serial.println("\n💾 Inicializando sistema de armazenamento (NVS)...");
  
  // Inicializa NVS Flash (CRÍTICO!)
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    Serial.println("⚠️ NVS precisa ser apagado, reinicializando...");
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  Serial.println("✅ NVS Flash inicializado!");
  
  prefs.begin(PREFS_NAMESPACE, true); // read-only para checar
  int tagsCount = prefs.getInt(PREFS_COUNT_KEY, 0);
  prefs.end();
  
  Serial.printf("✅ Sistema de armazenamento pronto!\n");
  Serial.printf("📊 Total de tags lidas anteriormente: %d\n", tagsCount);
  
  // ⭐ DEBUG: Descomentar para limpar todas as tags
  // clearAllTags();
  // Serial.println("⚠️ Todas as tags foram limpas!");
  
  Serial.println("\n✅ Sistema pronto!");
  Serial.println("⏳ Aguardando dados do Reader via UART...\n");
}


// ============================================
// LOOP
// ============================================

void loop() {
  // Verifica toques na tela
  handleTouch();
  
  // Verifica mensagens UART
  checkUARTMessages();
  
  // Verifica timeout do QR Code (3 minutos)
  checkQRCodeTimeout();
  
  // ⭐ NOVO: Verifica timeout da recompensa (1 minuto)
  checkRewardTimeout();
  
  // ⭐ NOVO: Verifica timeout da mensagem de admin (30 segundos)
  checkAdminMessageTimeout();
  
  // Auto-clear após timeout
  checkAutoClear();
  
  // Atualiza display baseado no modo atual
  if (currentMode == EYES_MODE) {
    // Atualiza animação RoboEyes (humor só muda com toque)
    roboEyes.update();
    delay(10);    
  } else if (currentMode == QRCODE_MODE) {
    // Modo QR Code: processa LVGL
    lv_timer_handler();
    lv_tick_inc(5);
    delay(5);
  } else if (currentMode == COIN_MODE || currentMode == LOOTED_MODE) {
    // ⭐ NOVO: Modos estáticos - não precisa atualizar
    delay(50);
  }
}

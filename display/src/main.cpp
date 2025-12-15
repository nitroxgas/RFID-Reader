#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// Inclui protocolo compartilhado
#include "../../common/protocol.h"

// Inclui UI gerada pelo SquareLine Studio (será criada depois)
// #include "../ui/ui.h"

// ============================================
// ESP32-2432S028R (CYD) - Display Controller
// Código preparado para SquareLine Studio
// ============================================

// Pinos UART (conectar ao Reader ESP32-WROOM)
#define UART_RX_PIN  27  // GPIO27 (CN1 Pin 3) <- Reader TX (GPIO17)
#define UART_TX_PIN  22  // GPIO22 (CN1 Pin 2) -> Reader RX (GPIO16)

// Display
TFT_eSPI tft = TFT_eSPI();

// LVGL Display Buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[TFT_WIDTH * 20];  // Buffer 1
static lv_color_t buf2[TFT_WIDTH * 20];  // Buffer 2 (double buffering)

// UI State
bool tagPresent = false;
String currentUID = "";
String currentURL = "";
String currentText = "";
ContentType currentType = CONTENT_RAW;

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
  tft.pushColors((uint16_t*)&color_p->full, w * h, true);
  tft.endWrite();
  
  lv_disp_flush_ready(disp);
}

/**
 * Inicializa LVGL
 */
void initLVGL() {
  Serial.println("Inicializando LVGL...");
  
  lv_init();
  
  // Configura buffer duplo para melhor performance
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, TFT_WIDTH * 20);
  
  // Registra driver do display
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.draw_buf = &draw_buf;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  lv_disp_drv_register(&disp_drv);
  
  Serial.println("LVGL inicializado!");
}

// ============================================
// UI TEMPORÁRIA (Será substituída pelo SquareLine)
// ============================================

lv_obj_t *screen_main;
lv_obj_t *label_title;
lv_obj_t *label_status;
lv_obj_t *label_uid;
lv_obj_t *label_content;
lv_obj_t *panel_qr;
lv_obj_t *qr_code;

/**
 * Cria UI temporária (será substituída pelo SquareLine Studio)
 */
void createTemporaryUI() {
  Serial.println("Criando UI temporária...");
  
  // Screen principal
  screen_main = lv_obj_create(NULL);
  lv_scr_load(screen_main);
  
  // Título
  label_title = lv_label_create(screen_main);
  lv_label_set_text(label_title, "RFID Reader System");
  lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x0080FF), 0);
  
  // Status
  label_status = lv_label_create(screen_main);
  lv_label_set_text(label_status, "Aguardando leitor...");
  lv_obj_align(label_status, LV_ALIGN_TOP_MID, 0, 45);
  lv_obj_set_style_text_font(label_status, &lv_font_montserrat_14, 0);
  
  // UID
  label_uid = lv_label_create(screen_main);
  lv_label_set_text(label_uid, "");
  lv_obj_align(label_uid, LV_ALIGN_CENTER, 0, -60);
  lv_obj_set_style_text_font(label_uid, &lv_font_montserrat_12, 0);
  
  // Conteúdo (URL/Texto)
  label_content = lv_label_create(screen_main);
  lv_label_set_text(label_content, "");
  lv_obj_align(label_content, LV_ALIGN_CENTER, 0, -30);
  lv_obj_set_style_text_font(label_content, &lv_font_montserrat_10, 0);
  lv_label_set_long_mode(label_content, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(label_content, 280);
  lv_obj_set_style_text_align(label_content, LV_TEXT_ALIGN_CENTER, 0);
  
  // Panel para QR Code
  panel_qr = lv_obj_create(screen_main);
  lv_obj_set_size(panel_qr, 200, 200);
  lv_obj_align(panel_qr, LV_ALIGN_CENTER, 0, 50);
  lv_obj_set_style_bg_color(panel_qr, lv_color_white(), 0);
  lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
  
  // QR Code
  qr_code = lv_qrcode_create(panel_qr, 180, lv_color_black(), lv_color_white());
  lv_obj_center(qr_code);
  
  Serial.println("UI temporária criada!");
  Serial.println("✨ Substitua por código do SquareLine Studio em display/ui/");
}

// ============================================
// FUNÇÕES DE ATUALIZAÇÃO DA UI
// ============================================

/**
 * Mostra informações da tag detectada
 */
void showTagInfo(const TagMessage& tag) {
  Serial.println("📱 Atualizando display com informações da tag...");
  
  // Atualiza status
  lv_label_set_text(label_status, "Tag Detectada!");
  lv_obj_set_style_text_color(label_status, lv_color_hex(0x00FF00), 0);
  
  // Mostra UID
  String uidText = "UID: " + tag.uid;
  lv_label_set_text(label_uid, uidText.c_str());
  
  // Mostra conteúdo baseado no tipo
  if (tag.type == CONTENT_URL && tag.url.length() > 0) {
    lv_label_set_text(label_content, tag.url.c_str());
    
    // Mostra QR Code com a URL
    lv_qrcode_update(qr_code, tag.url.c_str(), tag.url.length());
    lv_obj_clear_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
    
    Serial.println("  ├─ URL: " + tag.url);
    Serial.println("  └─ QR Code gerado");
    
  } else if (tag.type == CONTENT_TEXT && tag.text.length() > 0) {
    lv_label_set_text(label_content, tag.text.c_str());
    lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
    
    Serial.println("  └─ Texto: " + tag.text);
    
  } else {
    lv_label_set_text(label_content, "Dados brutos");
    lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
    
    Serial.println("  └─ Dados brutos (não-NDEF)");
  }
}

/**
 * Limpa display e volta para estado inicial
 */
void clearDisplay() {
  Serial.println("🔄 Limpando display...");
  
  lv_label_set_text(label_status, "Aguardando tag...");
  lv_obj_set_style_text_color(label_status, lv_color_white(), 0);
  lv_label_set_text(label_uid, "");
  lv_label_set_text(label_content, "");
  lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
  
  tagPresent = false;
}

/**
 * Atualiza status da conexão
 */
void updateConnectionStatus(String status) {
  lv_label_set_text(label_status, status.c_str());
  
  if (status.indexOf("READY") >= 0 || status.indexOf("pronto") >= 0) {
    lv_obj_set_style_text_color(label_status, lv_color_hex(0x00FF00), 0);
  } else if (status.indexOf("ERRO") >= 0 || status.indexOf("ERROR") >= 0) {
    lv_obj_set_style_text_color(label_status, lv_color_hex(0xFF0000), 0);
  } else {
    lv_obj_set_style_text_color(label_status, lv_color_white(), 0);
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
  tft.begin();
  tft.setRotation(1);  // Landscape (320x240)
  tft.fillScreen(TFT_BLACK);
  
  // Inicializa LVGL
  initLVGL();
  
  // Cria UI temporária (substituir por SquareLine)
  // NOTA: Quando usar SquareLine Studio, substitua por:
  // ui_init();  // Função gerada pelo SquareLine
  createTemporaryUI();
  
  Serial.println();
  Serial.println("✅ Sistema pronto!");
  Serial.println("⏳ Aguardando dados do Reader via UART...");
  Serial.println();
}

// ============================================
// LOOP
// ============================================

void loop() {
  // Processa tarefas LVGL (CRITICAL!)
  lv_timer_handler();
  
  // Verifica mensagens UART
  checkUARTMessages();
  
  // Auto-clear após timeout
  checkAutoClear();
  
  // Delay mínimo para LVGL
  delay(5);
}

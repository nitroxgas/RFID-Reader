#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "RoboEyesTFT_eSPI.h"

// Inclui protocolo compartilhado
#include "../common/protocol.h"

// ============================================
// ESP32-2432S028R (CYD) - Display Controller
// RoboEyes + QR Code LVGL
// ============================================

// Pinos UART (conectar ao Reader ESP32-WROOM)
#define UART_RX_PIN  27  // GPIO27 (CN1 Pin 3) <- Reader TX (GPIO17)
#define UART_TX_PIN  22  // GPIO22 (CN1 Pin 2) -> Reader RX (GPIO16)

// Pino Backlight do Display (CRÍTICO!)
#define TFT_BL  21       // GPIO21 - Backlight PWM

// Display
TFT_eSPI tft = TFT_eSPI();

// Create a RoboEyes instance (false = landscape, rotação 1)
TFT_RoboEyes roboEyes(tft, false, 1);

// LVGL Display Buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[TFT_WIDTH * 20];  // Buffer 1
static lv_color_t buf2[TFT_WIDTH * 20];  // Buffer 2 (double buffering)

// Display Mode States
enum DisplayMode {
  EYES_MODE,
  QRCODE_MODE
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
  tft.pushColors((uint16_t*)&color_p->full, w * h, false);  // swap via setSwapBytes(), não aqui
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
  lv_obj_set_size(panel_qr, 220, 220);
  lv_obj_align(panel_qr, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_bg_color(panel_qr, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(panel_qr, 255, 0);
  lv_obj_set_style_border_width(panel_qr, 3, 0);
  lv_obj_set_style_border_color(panel_qr, lv_color_hex(0x2095F6), 0);
  lv_obj_clear_flag(panel_qr, LV_OBJ_FLAG_SCROLLABLE);
  
  // QR Code 200x200
  qr_code = lv_qrcode_create(panel_qr, 200, lv_color_black(), lv_color_white());
  lv_obj_center(qr_code);
  
  Serial.println("✅ Tela QR Code criada!");
}

/**
 * Alterna para modo RoboEyes
 */
void switchToEyesMode() {
  Serial.println("👀 Alternando para modo RoboEyes...");
  currentMode = EYES_MODE;
  roboEyes.open();
  tft.fillScreen(TFT_BLACK);
  Serial.println("✅ Modo RoboEyes ativo!");
}

/**
 * Alterna para modo QR Code
 */
void switchToQRCodeMode(const String& url) {
  Serial.println("📱 Alternando para modo QR Code...");
  currentMode = QRCODE_MODE;
  
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

// ============================================
// FUNÇÕES DE ATUALIZAÇÃO DA UI
// ============================================

/**
 * Mostra informações da tag detectada
 */
void showTagInfo(const TagMessage& tag) {
  Serial.println("📱 Tag detectada!");
  Serial.println("  ├─ UID: " + tag.uid);
  
  // Mostra conteúdo baseado no tipo
  if (tag.type == CONTENT_URL && tag.url.length() > 0) {
    Serial.println("  ├─ Tipo: URL NDEF");
    Serial.println("  ├─ URL: " + tag.url);
    Serial.println("  └─ Exibindo QR Code...");
    
    // Alterna para modo QR Code
    switchToQRCodeMode(tag.url);
    
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
  // Se QR Code está visível e passou do timeout
  if (currentMode == QRCODE_MODE && qrCodeShowTime > 0 && 
      (millis() - qrCodeShowTime >= QR_CODE_TIMEOUT)) {
    Serial.println("⏰ Timeout: Retornando para RoboEyes após 3 minutos");
    switchToEyesMode();
    qrCodeShowTime = 0;
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
  
  tft.begin();
  tft.setRotation(1);  // Landscape 320x240 (rotação 1)
  tft.invertDisplay(false);
  tft.setSwapBytes(true);  // Swap para cores corretas
  
  // Teste do display - preencher tela
  Serial.println("  ↓ Teste: Preenchendo tela com vermelho...");
  tft.fillScreen(TFT_RED);
  delay(300);
  
  Serial.println("  ↓ Teste: Preenchendo tela com verde...");
  tft.fillScreen(TFT_GREEN);
  delay(300);
  
  Serial.println("  ↓ Teste: Preenchendo tela com azul...");
  tft.fillScreen(TFT_BLUE);
  delay(300);
  
  Serial.println("  ↓ Teste: Tela preta...");
  tft.fillScreen(TFT_BLACK);
  delay(200);
  
  Serial.println("✅ TFT Display inicializado!");
  
  // Inicializa RoboEyes para tela 320x240
  Serial.println("👀 Inicializando RoboEyes...");
  roboEyes.setScreenSize(320, 240);  // Define tamanho da tela
  roboEyes.begin(50);  // 50 FPS
  
  // Customiza olhos para ocupar boa parte da tela
  roboEyes.setColors(TFT_WHITE, TFT_BLACK);  // Olhos brancos, fundo preto
  roboEyes.setWidth(100, 100);   // Largura dos olhos
  roboEyes.setHeight(100, 100);  // Altura dos olhos
  roboEyes.setBorderradius(15, 15);  // Bordas arredondadas
  roboEyes.setSpacebetween(20);  // Espaço entre olhos
  
  // Animações
  roboEyes.setAutoblinker(true, 3, 2);  // Piscar automático
  roboEyes.setIdleMode(true, 5, 3);     // Movimento idle (olhar ao redor)
  roboEyes.open();  // Abre os olhos
  
  Serial.println("✅ RoboEyes inicializado!");
  Serial.println("  └─ Dimensões: 320x240");
  Serial.println("  └─ Olhos: 100x100 cada");
  
  // LVGL será inicializado sob demanda quando precisar do QR Code
  
  Serial.println();
  Serial.println("✅ Sistema pronto!");
  Serial.println("⏳ Aguardando dados do Reader via UART...");
  Serial.println();
}

// ============================================
// LOOP
// ============================================

void loop() {
  // Verifica mensagens UART
  checkUARTMessages();
  
  // Verifica timeout do QR Code (3 minutos)
  checkQRCodeTimeout();
  
  // Auto-clear após timeout
  checkAutoClear();
  
  // Atualiza display baseado no modo atual
  if (currentMode == EYES_MODE) {
    // Modo RoboEyes: atualiza animação dos olhos
    roboEyes.update();
    delay(5);
    
  } else if (currentMode == QRCODE_MODE) {
    // Modo QR Code: processa LVGL
    lv_timer_handler();
    lv_tick_inc(5);
    delay(5);
  }
}

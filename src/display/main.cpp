#include <Arduino.h>
#include <TFT_eSPI.h>
#include "RoboEyesTFT_eSPI.h"
#include <TFT_eTouch.h>
#include <SPI.h>

// LVGL para QR Code
#include <lvgl.h>

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
const unsigned long QR_CODE_TIMEOUT = 120000;  // 2 minutos em ms

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
  lv_obj_align(panel_qr, LV_ALIGN_CENTER, 0, -20);
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
 * Alterna para modo RoboEyes (TEMPORARIAMENTE SEM ROBOEYES)
 */
void switchToEyesMode() {
  Serial.println("👀 Alternando para modo Eyes...");
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
  
  initializeLVGLIfNeeded();
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
      // Se está mostrando QR Code, volta para olhos
      Serial.println("📱 Touch no QR Code - voltando aos olhos...");
      switchToEyesMode();
      
    } else if (currentMode == EYES_MODE) {
      // Se está mostrando olhos, executa animação confused
      Serial.println("👀 Touch nos olhos - executando animação confused...");
      roboEyes.anim_confused();
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
  
  // Inicializa TFT
  Serial.println("  ↓ Inicializando SPI e TFT...");
  tft.init();
  tft.invertDisplay(true);
  tft.setRotation(4);    // Portrait 240x320
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
  roboEyes.setScreenSize(240, 260);
  roboEyes.setWidth(50,50);
  roboEyes.setHeight(50,50);
  //roboEyes.setBorderradius(8,8);
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
  
  // Verifica timeout do QR Code (2 minutos)
  checkQRCodeTimeout();
  
  // Auto-clear após timeout
  checkAutoClear();
  
  // Atualiza display baseado no modo atual
  if (currentMode == EYES_MODE) {
    // Muda humor aleatoriamente a cada 1 minuto
    if (millis() - lastMoodChange >= MOOD_CHANGE_INTERVAL) {
      changeRandomMood();
      roboEyes.anim_laugh();
      lastMoodChange = millis();
    }
    
    // Atualiza animação RoboEyes
    roboEyes.update();
    delay(10);    
  } else if (currentMode == QRCODE_MODE) {
    // Modo QR Code: processa LVGL
    lv_timer_handler();
    lv_tick_inc(5);
    delay(5);
  }
}

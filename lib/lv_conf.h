/**
 * @file lv_conf.h
 * Configuração LVGL para ESP32-2432S028R (CYD)
 */

#if 1 // Set it to "1" to enable content

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

// ============================================
// CONFIGURAÇÕES BÁSICAS
// ============================================
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1  // SWAP necessário para TFT_eSPI
#define LV_COLOR_SCREEN_TRANSP 1  // Suporte a canal alpha

// ============================================
// MEMÓRIA
// ============================================
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (64U * 1024U)  // 64KB

// ============================================
// DISPLAY (PORTRAIT 240x320)
// ============================================
#define LV_HOR_RES_MAX 240
#define LV_VER_RES_MAX 320
#define LV_DPI_DEF 130

// ============================================
// FONTES
// ============================================
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 1  // Habilitar
#define LV_FONT_MONTSERRAT_12 1  // Habilitar
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1  // Habilitar
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

#define LV_FONT_DEFAULT &lv_font_montserrat_14

// ============================================
// WIDGETS EXTRAS
// ============================================
#define LV_USE_ANIMIMG 1
#define LV_USE_CALENDAR 0
#define LV_USE_CHART 0
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN 1
#define LV_USE_KEYBOARD 0
#define LV_USE_LED 1
#define LV_USE_LIST 1
#define LV_USE_MENU 0
#define LV_USE_METER 0
#define LV_USE_MSGBOX 1
#define LV_USE_SPAN 0
#define LV_USE_SPINBOX 0
#define LV_USE_SPINNER 1
#define LV_USE_TABVIEW 1
#define LV_USE_TILEVIEW 0
#define LV_USE_WIN 0

// QR CODE - IMPORTANTE!
#define LV_USE_QRCODE 1

// ============================================
// TEMAS
// ============================================
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0

// ============================================
// LOG (desabilitar para produção)
// ============================================
#define LV_USE_LOG 1
#if LV_USE_LOG
  #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
  #define LV_LOG_PRINTF 1
#endif

// ============================================
// PERFORMANCE
// ============================================
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0
#define LV_USE_REFR_DEBUG 0

// ============================================
// OUTRAS CONFIGURAÇÕES
// ============================================
#define LV_TXT_ENC LV_TXT_ENC_UTF8
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ 0

#define LV_TICK_CUSTOM 0
#define LV_DISP_DEF_REFR_PERIOD 30

#endif // LV_CONF_H

#endif // #if 1

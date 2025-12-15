#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>

// ============================================
// PROTOCOLO DE COMUNICAÇÃO UART
// Entre Reader (ESP32-WROOM) e Display (CYD)
// ============================================

// Tipos de mensagem
#define MSG_TAG     "TAG"
#define MSG_STATUS  "STATUS"
#define MSG_ERROR   "ERROR"
#define MSG_CMD     "CMD"
#define MSG_ACK     "ACK"

// Tipos de conteúdo NDEF
enum ContentType {
  CONTENT_RAW = 0,
  CONTENT_URL = 1,
  CONTENT_TEXT = 2
};

// Estrutura de mensagem TAG
struct TagMessage {
  String uid;
  String url;
  String text;
  ContentType type;
};

// ============================================
// FUNÇÕES DE CODIFICAÇÃO/DECODIFICAÇÃO
// ============================================

class CommProtocol {
public:
  /**
   * Codifica mensagem TAG para string
   * Formato: TAG|UID|URL|TEXT|TYPE\n
   */
  static String encodeTag(const TagMessage& tag) {
    String message = String(MSG_TAG) + "|";
    message += tag.uid + "|";
    message += tag.url + "|";
    message += tag.text + "|";
    message += String((int)tag.type);
    return message;
  }
  
  /**
   * Decodifica string para mensagem TAG
   * Formato esperado: TAG|UID|URL|TEXT|TYPE\n
   */
  static TagMessage decodeTag(const String& message) {
    TagMessage tag;
    
    // Remove prefixo "TAG|"
    int start = message.indexOf('|') + 1;
    
    // UID
    int sep1 = message.indexOf('|', start);
    tag.uid = message.substring(start, sep1);
    
    // URL
    int sep2 = message.indexOf('|', sep1 + 1);
    tag.url = message.substring(sep1 + 1, sep2);
    
    // Text
    int sep3 = message.indexOf('|', sep2 + 1);
    tag.text = message.substring(sep2 + 1, sep3);
    
    // Type
    String typeStr = message.substring(sep3 + 1);
    typeStr.trim();
    tag.type = (ContentType)typeStr.toInt();
    
    return tag;
  }
  
  /**
   * Codifica mensagem STATUS
   * Formato: STATUS|mensagem\n
   */
  static String encodeStatus(const String& status) {
    return String(MSG_STATUS) + "|" + status;
  }
  
  /**
   * Codifica mensagem ERROR
   * Formato: ERROR|codigo_erro\n
   */
  static String encodeError(const String& error) {
    return String(MSG_ERROR) + "|" + error;
  }
  
  /**
   * Codifica mensagem ACK
   * Formato: ACK\n
   */
  static String encodeAck() {
    return String(MSG_ACK);
  }
  
  /**
   * Verifica tipo de mensagem
   * Retorna: TAG, STATUS, ERROR, CMD, ACK ou "" se inválido
   */
  static String getMessageType(const String& message) {
    int sep = message.indexOf('|');
    if (sep > 0) {
      return message.substring(0, sep);
    }
    // Mensagem sem separador (ex: ACK)
    if (message == MSG_ACK) {
      return MSG_ACK;
    }
    return "";
  }
  
  /**
   * Valida formato de mensagem
   */
  static bool isValidMessage(const String& message) {
    String type = getMessageType(message);
    return (type == MSG_TAG || 
            type == MSG_STATUS || 
            type == MSG_ERROR || 
            type == MSG_CMD || 
            type == MSG_ACK);
  }
};

#endif // PROTOCOL_H

/**
 * Sistema de Armazenamento de Tags usando SPIFFS
 * Alternativa ao NVS Preferences para maior confiabilidade
 * 
 * Uso: Substitui Preferences por sistema de arquivos SPIFFS
 */

#ifndef TAG_STORAGE_SPIFFS_H
#define TAG_STORAGE_SPIFFS_H

#include <Arduino.h>
#include <SPIFFS.h>

class TagStorageSPIFFS {
private:
    const char* TAGS_FILE = "/tags.txt";
    const char* BACKUP_FILE = "/tags_backup.txt";
    
public:
    /**
     * Inicializa SPIFFS
     */
    bool begin() {
        Serial.println("💾 Inicializando SPIFFS...");
        
        if (!SPIFFS.begin(true)) {  // true = format on fail
            Serial.println("❌ Falha ao montar SPIFFS!");
            return false;
        }
        
        Serial.println("✅ SPIFFS montado com sucesso!");
        
        // Verifica se arquivo existe, se não, cria
        if (!SPIFFS.exists(TAGS_FILE)) {
            Serial.println("📝 Criando arquivo de tags...");
            File file = SPIFFS.open(TAGS_FILE, "w");
            if (file) {
                file.println("# RFID Tags Storage");
                file.close();
                Serial.println("✅ Arquivo criado!");
            }
        }
        
        return true;
    }
    
    /**
     * Verifica se uma tag já foi lida
     */
    bool isTagAlreadyRead(String uid) {
        File file = SPIFFS.open(TAGS_FILE, "r");
        if (!file) {
            Serial.println("❌ Erro ao abrir arquivo para leitura!");
            return false;
        }
        
        bool found = false;
        while (file.available()) {
            String line = file.readStringUntil('\n');
            line.trim();
            
            // Ignora comentários e linhas vazias
            if (line.length() == 0 || line.startsWith("#")) continue;
            
            if (line == uid) {
                found = true;
                break;
            }
        }
        
        file.close();
        return found;
    }
    
    /**
     * Salva uma tag como lida
     */
    void saveTagAsRead(String uid) {
        // Verifica se já existe
        if (isTagAlreadyRead(uid)) {
            Serial.println("⚠️ Tag já existe no arquivo!");
            return;
        }
        
        // Adiciona ao arquivo
        File file = SPIFFS.open(TAGS_FILE, "a");  // append mode
        if (!file) {
            Serial.println("❌ Erro ao abrir arquivo para escrita!");
            return;
        }
        
        file.println(uid);
        file.close();
        
        int count = getReadTagsCount();
        Serial.printf("✅ Tag salva! Total de tags lidas: %d\n", count);
    }
    
    /**
     * Retorna quantidade de tags lidas
     */
    int getReadTagsCount() {
        File file = SPIFFS.open(TAGS_FILE, "r");
        if (!file) return 0;
        
        int count = 0;
        while (file.available()) {
            String line = file.readStringUntil('\n');
            line.trim();
            
            // Ignora comentários e linhas vazias
            if (line.length() > 0 && !line.startsWith("#")) {
                count++;
            }
        }
        
        file.close();
        return count;
    }
    
    /**
     * Lista todas as tags via Serial
     */
    void listAllTags() {
        Serial.println("\n📊 ========== LISTA DE TAGS LIDAS ===========");
        
        int count = getReadTagsCount();
        Serial.printf("📊 Total de tags armazenadas: %d\n\n", count);
        
        if (count == 0) {
            Serial.println("⚠️ Nenhuma tag armazenada ainda.");
        } else {
            Serial.println("├───┬──────────────────────────");
            Serial.println("│ # │ UID                  │");
            Serial.println("├───┼──────────────────────────");
            
            File file = SPIFFS.open(TAGS_FILE, "r");
            if (file) {
                int index = 1;
                while (file.available()) {
                    String line = file.readStringUntil('\n');
                    line.trim();
                    
                    if (line.length() > 0 && !line.startsWith("#")) {
                        Serial.printf("│%3d│ %-20s│\n", index, line.c_str());
                        index++;
                    }
                }
                file.close();
                Serial.println("└───┴──────────────────────────");
            }
        }
        
        Serial.println("📊 =========================================\n");
    }
    
    /**
     * Faz backup para SD Card
     */
    bool backupToSD() {
        Serial.println("\n💾 Iniciando backup de tags para SD Card...");
        
        // Lê arquivo SPIFFS
        File srcFile = SPIFFS.open(TAGS_FILE, "r");
        if (!srcFile) {
            Serial.println("❌ Erro ao abrir arquivo de tags!");
            return false;
        }
        
        // Inicializa SD
        if (!SD.begin(5)) {  // CS pin 5
            Serial.println("❌ Erro: SD Card não detectado!");
            srcFile.close();
            return false;
        }
        
        // Cria arquivo de backup
        String filename = "/rfid_backup_" + String(millis()) + ".txt";
        File dstFile = SD.open(filename.c_str(), FILE_WRITE);
        if (!dstFile) {
            Serial.println("❌ Erro ao criar arquivo de backup!");
            srcFile.close();
            return false;
        }
        
        // Copia conteúdo
        dstFile.println("========================================");
        dstFile.println("RFID TAGS BACKUP");
        dstFile.println("Sistema: ESP32 RFID Reader Display CYD");
        dstFile.printf("Data: %lu ms\n", millis());
        dstFile.printf("Total de tags: %d\n", getReadTagsCount());
        dstFile.println("========================================\n");
        
        int index = 1;
        while (srcFile.available()) {
            String line = srcFile.readStringUntil('\n');
            line.trim();
            
            if (line.length() > 0 && !line.startsWith("#")) {
                dstFile.printf("%d,%s\n", index, line.c_str());
                index++;
            }
        }
        
        dstFile.println("\n========================================");
        dstFile.println("FIM DO BACKUP");
        dstFile.println("========================================");
        
        srcFile.close();
        dstFile.close();
        
        Serial.printf("✅ Backup criado com sucesso!\n");
        Serial.printf("📁 Arquivo: %s\n", filename.c_str());
        Serial.printf("📊 %d tags salvas\n\n", index - 1);
        
        return true;
    }
    
    /**
     * Limpa todas as tags
     */
    void clearAllTags() {
        // Faz backup antes de limpar
        if (SPIFFS.exists(TAGS_FILE)) {
            SPIFFS.remove(BACKUP_FILE);  // Remove backup antigo
            SPIFFS.rename(TAGS_FILE, BACKUP_FILE);  // Renomeia atual para backup
        }
        
        // Cria novo arquivo vazio
        File file = SPIFFS.open(TAGS_FILE, "w");
        if (file) {
            file.println("# RFID Tags Storage");
            file.close();
        }
        
        Serial.println("⚠️ Todas as tags foram apagadas!");
        Serial.println("📁 Backup local salvo em: /tags_backup.txt");
    }
    
    /**
     * Obtém estatísticas do SPIFFS
     */
    void printStats() {
        size_t totalBytes = SPIFFS.totalBytes();
        size_t usedBytes = SPIFFS.usedBytes();
        
        Serial.println("\n📊 Estatísticas SPIFFS:");
        Serial.printf("  ├─ Total: %d bytes\n", totalBytes);
        Serial.printf("  ├─ Usado: %d bytes (%.1f%%)\n", usedBytes, (float)usedBytes / totalBytes * 100);
        Serial.printf("  └─ Livre: %d bytes\n\n", totalBytes - usedBytes);
    }
};

#endif // TAG_STORAGE_SPIFFS_H

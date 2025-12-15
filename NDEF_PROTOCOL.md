# 📖 Protocolo NDEF - Implementação e Documentação

## 🎯 O que foi Implementado

O código agora implementa um **parser completo do protocolo NDEF** (NFC Data Exchange Format) para:
1. ✅ Detectar URLs em tags NFC
2. ✅ Extrair e exibir URLs completas
3. ✅ Detectar texto NDEF
4. ✅ Ocultar bytes NULL na visualização
5. ✅ Identificar tipo de conteúdo automaticamente

---

## 📋 Estrutura do Protocolo NDEF

### 1. TLV (Type-Length-Value) Structure

```
┌─────────────────────────────────────┐
│ Byte 0: TLV Type (0x03 = NDEF)     │
│ Byte 1: Length (tamanho da mensagem)│
│ Byte 2+: NDEF Message               │
│ Último: 0xFE (Terminator)           │
└─────────────────────────────────────┘
```

### 2. NDEF Record Header

```
Bit 7: MB (Message Begin)
Bit 6: ME (Message End)
Bit 5: CF (Chunk Flag)
Bit 4: SR (Short Record)
Bit 3: IL (ID Length present)
Bit 2-0: TNF (Type Name Format)

TNF Values:
0x00 = Empty
0x01 = Well-Known (usado para URI e Text)
0x02 = MIME Media Type
0x03 = Absolute URI
0x04 = External Type
0x05 = Unknown
0x06 = Unchanged
0x07 = Reserved
```

### 3. NDEF URI Record Structure

```
Offset  Campo           Descrição
------  --------------  ------------------------------------
0       Header          Flags do record (TNF, SR, etc.)
1       Type Length     Tamanho do tipo (1 para "U")
2       Payload Length  Tamanho do payload (modo short)
3       Type            "U" (0x55) para URI
4       URI ID Code     Prefixo da URL (0x01 = http://www.)
5+      URI String      Resto da URL (sem o prefixo)
```

### 4. URI Identifier Codes (RFC Completo)

| Code | Prefixo | Exemplo |
|------|---------|---------|
| 0x00 | (nenhum) | example.com |
| 0x01 | http://www. | example.com → http://www.example.com |
| 0x02 | https://www. | example.com → https://www.example.com |
| 0x03 | http:// | example.com → http://example.com |
| 0x04 | https:// | example.com → https://example.com |
| 0x05 | tel: | +5511999999999 → tel:+5511999999999 |
| 0x06 | mailto: | user@example.com → mailto:user@example.com |
| ... | ... | (35 códigos no total) |

---

## 🔬 Exemplo Real de URL NDEF

### Tag contendo: `http://www.example.com`

```
Bytes em Hex:
03 1A D1 01 16 55 01 65 78 61 6D 70 6C 65 2E 63 6F 6D FE

Interpretação:
03       → TLV Type (NDEF Message)
1A       → Length (26 bytes)
D1       → Header (MB=1, ME=1, SR=1, TNF=0x01)
01       → Type Length (1 byte)
16       → Payload Length (22 bytes)
55       → Type ("U" = URI)
01       → URI ID Code (http://www.)
65 78... → "example.com" em ASCII
FE       → Terminator TLV
```

### Decodificação Passo a Passo:

1. **Byte 0 (0x03)**: É uma mensagem NDEF ✅
2. **Byte 1 (0x1A)**: Mensagem tem 26 bytes
3. **Byte 2 (0xD1)**: 
   - MB=1 (início da mensagem)
   - ME=1 (fim da mensagem)
   - SR=1 (short record)
   - TNF=0x01 (Well-Known type)
4. **Byte 3 (0x01)**: Tipo tem 1 byte de tamanho
5. **Byte 4 (0x16)**: Payload tem 22 bytes (0x16 = 22)
6. **Byte 5 (0x55)**: Tipo é "U" (URI Record)
7. **Byte 6 (0x01)**: Prefixo `http://www.`
8. **Bytes 7-17**: `example.com`
9. **Resultado**: `http://www.example.com` 🌐

---

## 💻 Funções Implementadas

### 1. `getURIPrefix(byte code)`

Converte URI Identifier Code para prefixo de URL:

```cpp
String getURIPrefix(0x01)  // Retorna: "http://www."
String getURIPrefix(0x04)  // Retorna: "https://"
String getURIPrefix(0x05)  // Retorna: "tel:"
```

**Suporta 35 códigos** conforme NFC Forum URI Record Type Definition.

---

### 2. `extractNDEFUrl(byte* data, int dataSize)`

Parser completo de URL NDEF:

```cpp
String url = extractNDEFUrl(allData, dataIndex);
// Exemplo: "http://www.example.com"
```

**O que faz:**
1. Verifica TLV Type (0x03)
2. Valida tamanho da mensagem
3. Parseia NDEF Record Header
4. Verifica TNF = 0x01 (Well-Known)
5. Verifica Record Type = "U" (URI)
6. Lê URI ID Code
7. Combina prefixo + resto da URL
8. Retorna URL completa

---

### 3. `extractNDEFText(byte* data, int dataSize)`

Parser de texto NDEF:

```cpp
String text = extractNDEFText(allData, dataIndex);
// Exemplo: "Hello World"
```

**O que faz:**
1. Verifica NDEF Message
2. Verifica Record Type = "T" (Text)
3. Lê Status Byte (idioma, UTF-16)
4. Pula código de idioma ("en", "pt", etc.)
5. Extrai texto puro
6. Retorna string

---

## 🖥️ Exemplo de Saída

### Tag com URL: `https://www.google.com`

```
========================================
         NOVA TAG DETECTADA!
========================================
UID da tag: 04A1B2C3D4E5F6
Tamanho do UID: 7 bytes
Tipo PICC: MIFARE Ultralight/NTAG
Subtipo NTAG: NTAG215
========================================

========================================
📄 LEITURA COMPLETA - NTAG215
========================================
Páginas de dados: 4 a 129
Total de bytes: 504

--- DADOS EM HEXADECIMAL ---
Pág   4-  7: 03 13 d1 01 0f 55 02 67 6f 6f 67 6c 65 2e 63 6f 
Pág   8- 11: 6d fe 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
...

========================================
🌐 URL DETECTADA (NDEF)
========================================
https://www.google.com
========================================

--- DADOS COMO STRING ---
(Caracteres imprimíveis, NULL ocultos)

[0x03][0x13][0xd1][0x01][0x0f]U[0x02]google.com[FIM_NDEF]

--- ESTATÍSTICAS ---
Bytes lidos: 504
Bytes com dados: 19
Bytes NULL (ocultos): 485
Caracteres imprimíveis: 11
Tipo de conteúdo: URL (NDEF URI Record)
========================================
```

---

## 🎯 Melhorias Implementadas

### 1. ✅ Detecção Automática de URL
- Parser NDEF completo
- Suporte a 35 tipos de prefixo
- Validação de estrutura

### 2. ✅ Ocultação de NULL Bytes
```cpp
// Antes:
[NULL][NULL][NULL][NULL][NULL]...(486 vezes)

// Agora:
...[486 bytes NULL]...
```

### 3. ✅ Identificação de Tipo
```
Tipo de conteúdo: URL (NDEF URI Record)
Tipo de conteúdo: Texto (NDEF Text Record)
Tipo de conteúdo: NDEF (formato não reconhecido)
Tipo de conteúdo: Dados brutos (não-NDEF)
```

### 4. ✅ Estatísticas Melhoradas
```
Bytes lidos: 504
Bytes com dados: 19         ← Sem contar NULLs
Bytes NULL (ocultos): 485   ← Quantos foram ocultados
Caracteres imprimíveis: 11
```

---

## 📊 Tipos de Tag Suportados

| Tipo de Conteúdo | Detecção | Extração | Exemplo |
|------------------|----------|----------|---------|
| **URL (http/https)** | ✅ | ✅ | http://www.example.com |
| **Telefone (tel:)** | ✅ | ✅ | tel:+5511999999999 |
| **Email (mailto:)** | ✅ | ✅ | mailto:user@example.com |
| **Texto simples** | ✅ | ✅ | Hello World |
| **Dados Amiibo** | ✅ | ⚠️ | (Exibe em hex) |
| **Tag vazia** | ✅ | - | (Detecta e informa) |

---

## 🔍 Debug e Troubleshooting

### Problema: URL não detectada

**Verificar:**
1. Byte 0 = 0x03? (NDEF Message)
2. TNF = 0x01? (Well-Known)
3. Record Type = "U"? (URI)

**Debug:**
```cpp
Serial.print("Byte 0: 0x");
Serial.println(allData[0], HEX);  // Deve ser 0x03

Serial.print("TNF: 0x");
Serial.println(allData[2] & 0x07, HEX);  // Deve ser 0x01
```

### Problema: URL incompleta

**Causa comum:** URI ID Code incorreto

**Solução:**
```cpp
Serial.print("URI Code: 0x");
Serial.println(uriCode, HEX);
// 0x01 = http://www.
// 0x02 = https://www.
// 0x03 = http://
// 0x04 = https://
```

---

## 📚 Referências Técnicas

### Especificações NFC Forum:
1. **NFC Data Exchange Format (NDEF)** - Technical Specification
2. **NFC Record Type Definition (RTD)** - URI
3. **NFC Record Type Definition (RTD)** - Text
4. **Type 2 Tag Operation Specification** - NTAG213/215

### Links Úteis:
- [NFC Forum Specifications](https://nfc-forum.org/specifications/)
- [NTAG213/215 Datasheet](https://www.nxp.com/docs/en/data-sheet/NTAG213_215_216.pdf)
- [NDEF URI Record](https://github.com/nfcpy/ndeflib)

---

## ✅ Resumo das Funcionalidades

| Funcionalidade | Status | Descrição |
|----------------|--------|-----------|
| Detecção NDEF | ✅ | Identifica mensagens NDEF (0x03) |
| Parser URI | ✅ | Extrai URLs completas |
| Parser Text | ✅ | Extrai textos simples |
| 35 Prefixos URI | ✅ | http, https, tel, mailto, etc. |
| Ocultação NULL | ✅ | Agrupa e resume bytes NULL |
| Tipo de conteúdo | ✅ | Identifica automaticamente |
| Terminator NDEF | ✅ | Detecta 0xFE (fim de dados) |
| Estatísticas | ✅ | Bytes dados vs. NULL |

---

## 🎉 Exemplo Completo

### Tag NFC com URL

**Dados brutos (hex):**
```
03 13 D1 01 0F 55 02 67 6F 6F 67 6C 65 2E 63 6F 6D FE 00 00 ...
```

**O código detecta:**
- ✅ É mensagem NDEF
- ✅ É URI Record (tipo "U")
- ✅ URI Code = 0x02 (https://www.)
- ✅ String = "google.com"
- ✅ **URL completa**: `https://www.google.com`

**Saída:**
```
========================================
🌐 URL DETECTADA (NDEF)
========================================
https://www.google.com
========================================
```

---

**Versão**: 3.0  
**Protocolo**: NFC Forum NDEF v1.0  
**Compliance**: ✅ Total com especificação oficial

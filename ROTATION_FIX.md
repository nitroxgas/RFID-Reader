# 🔄 Fix: Rotação Automática e Posicionamento Dinâmico

## ❌ Problema Reportado

1. **Olhos sobrepostos** (um sobre o outro)
2. **Orientação errada** da tela
3. **Faixa manchada** na lateral

**Diagnóstico**: Tela em PORTRAIT (240x320) quando deveria estar em LANDSCAPE (320x240)!

---

## ✅ Solução Implementada

### 1. **Teste Automático de Rotações**

O código agora **testa todas as 4 rotações possíveis** e escolhe automaticamente a landscape:

```cpp
// Testa todas as rotações
for (int rot = 0; rot < 4; rot++) {
  tft.setRotation(rot);
  Serial.printf("  Rotação %d: %dx%d\n", rot, tft.width(), tft.height());
}

// Escolhe a primeira que for LANDSCAPE (width > height)
for (int rot = 0; rot < 4; rot++) {
  tft.setRotation(rot);
  if (tft.width() > tft.height()) {
    Serial.printf("✅ Usando rotação %d (LANDSCAPE): %dx%d\n", 
                  rot, tft.width(), tft.height());
    break;
  }
}
```

**Vantagem**: Funciona independente da configuração do TFT_eSPI!

---

### 2. **Posicionamento Dinâmico dos Olhos**

As posições agora são **calculadas dinamicamente** baseado na resolução real:

```cpp
void updateEyePositions() {
  int w = tft.width();
  int h = tft.height();
  
  eyeY = h / 2;             // Centro vertical
  eyeLeftX = w / 4;         // 1/4 da largura (esquerdo)
  eyeRightX = (3 * w) / 4;  // 3/4 da largura (direito)
}
```

**Antes** (hardcoded):
```cpp
eyeLeftX = 80;    // ❌ Fixo - não funciona em portrait
eyeRightX = 240;  // ❌ Fixo - sobrepostos em 240x320
```

**Agora** (dinâmico):
```cpp
// Se 320x240 (landscape):
eyeLeftX = 320 / 4 = 80
eyeRightX = (3 * 320) / 4 = 240
eyeY = 240 / 2 = 120

// Se 240x320 (portrait, caso falhe):
eyeLeftX = 240 / 4 = 60
eyeRightX = (3 * 240) / 4 = 180
eyeY = 320 / 2 = 160
```

---

### 3. **Teste de Grade de Orientação**

Novo teste visual para verificar se a orientação está correta:

```
┌───────────────────────────────┐
│🔴                        🟢   │ ← Cantos coloridos
│                               │
│                               │
│──────────── + ────────────────│ ← Cruz branca (centro)
│                               │
│                               │
│🔵                        🟡   │
└───────────────────────────────┘

🔴 = Vermelho (superior esquerdo)
🟢 = Verde (superior direito)
🔵 = Azul (inferior esquerdo)
🟡 = Amarelo (inferior direito)
+ = Cruz branca (centro)
```

**Exibido por 2 segundos** após os testes de cores.

---

## 🎬 Nova Sequência Visual

### 1. **Boot** (7 segundos):

```
Backlight liga
  ↓
Vermelho (500ms)
  ↓
Verde (500ms)
  ↓
Azul (500ms)
  ↓
Preto (500ms)
  ↓
Grade de orientação (2s)
  🔴──────────🟢
  │          │
  │    +     │
  │          │
  🔵──────────🟡
  ↓
Tela preta
  ↓
Olhos aparecem separados!
```

---

### 2. **Monitor Serial Esperado**:

```
⚙️  TESTANDO ROTAÇÕES:
  Rotação 0: 240x320
  Rotação 1: 320x240  ← LANDSCAPE!
  Rotação 2: 240x320
  Rotação 3: 320x240  ← LANDSCAPE!

✅ Usando rotação 1 (LANDSCAPE): 320x240

  ↓ Teste 1: Vermelho...
  ↓ Teste 2: Verde...
  ↓ Teste 3: Azul...
  ↓ Teste 4: Tela preta...
  ↓ Teste 5: Grade de orientação...
  
✅ TFT Display inicializado!
  ├─ Resolução: 320x240
  ├─ Rotação: 1
  └─ Heap livre: ~240000 bytes

👀 Iniciando animação de teste (sem sprite)...

👀 Posições dos olhos (tela 320x240):
  ├─ Olho esquerdo: (80, 120)
  ├─ Olho direito: (240, 120)
  └─ Tamanho: 80 px

  └─ Desenhando olhos...
  └─ Olhos desenhados! Piscarão a cada 3s

✅ Sistema pronto!
```

---

### 3. **Visual Final Esperado**:

```
Display CYD (320x240 landscape):
┌─────────────────────────────────┐
│                                 │
│                                 │
│      ⚪           ⚪             │
│   (x=80)       (x=240)          │
│    y=120        y=120           │
│                                 │
│   SEPARADOS!                    │
│   SEM sobreposição              │
│   SEM faixa branca              │
└─────────────────────────────────┘
```

**Distância entre olhos**: 160 pixels (80 → 240)

---

## 🔍 Como Verificar se Está Correto

### ✅ Grade de Orientação (2 segundos):

**Se a grade aparece assim**:
```
🔴──────🟢
│   +   │
🔵──────🟡
```
→ **LANDSCAPE CORRETO!** (horizontal)

**Se aparece assim**:
```
🔴─🔵
│ │
+ │
│ │
🟢─🟡
```
→ **PORTRAIT!** (vertical) - Algo errado!

---

### ✅ Olhos:

**Correto** (landscape 320x240):
```
│      ⚪           ⚪             │
│   Esquerdo     Direito          │
```
- Dois círculos SEPARADOS
- Na mesma linha horizontal

**Errado** (portrait 240x320):
```
│        ⚪                        │
│        ⚪                        │
│    Sobrepostos!                 │
```
- Círculos muito próximos ou sobrepostos

---

## 📊 Debug via Serial Monitor

### Abra o monitor:
```bash
pio device monitor -e display-cyd
```

### Verifique:

1. **Teste de rotações**: Deve mostrar 4 combinações
2. **Rotação escolhida**: Deve ser 1 ou 3 (ambos são 320x240)
3. **Resolução final**: Deve ser `320x240`
4. **Posições dos olhos**: Devem estar em `(80, 120)` e `(240, 120)`

---

## 🐛 Troubleshooting

### Se AINDA aparecem sobrepostos:

**Cenário 1**: Serial mostra `240x320`

```
Problema: Código não conseguiu encontrar rotação landscape
Solução: 
  1. Verificar User_Setup.h do TFT_eSPI
  2. Forçar rotação manual:
     tft.setRotation(1); // ou 3
```

**Cenário 2**: Serial mostra `320x240` mas olhos sobrepostos

```
Problema: Função updateEyePositions() não foi chamada
Solução: Verificar se aparece no Serial:
  "👀 Posições dos olhos (tela 320x240):"
```

**Cenário 3**: Grade aparece distorcida

```
Problema: Hardware ou pinout errado
Solução: Verificar pinos SPI no User_Setup.h:
  TFT_MOSI = 13
  TFT_MISO = 12
  TFT_SCLK = 14
  TFT_CS   = 15
  TFT_DC   = 2
```

---

### Se grade aparece PORTRAIT (vertical):

1. **Adicionar força manual**:
```cpp
// Após o loop de teste, forçar:
tft.setRotation(3);  // Tentar rotação 3
if (tft.width() == 240) {
  tft.setRotation(1);  // Ou rotação 1
}
```

2. **Verificar cabo USB**: Conexão instável pode causar reset

3. **Limpar e recompilar**:
```bash
pio run -t clean
pio run -e display-cyd --target upload
```

---

## 🎯 Checklist de Verificação

Monitor Serial:
- ✅ "TESTANDO ROTAÇÕES" aparece?
- ✅ "Usando rotação X (LANDSCAPE): 320x240" aparece?
- ✅ "Resolução: 320x240" (não 240x320)?
- ✅ "Posições dos olhos" aparecem?
- ✅ Olho esquerdo em (80, 120)?
- ✅ Olho direito em (240, 120)?

Display:
- ✅ Cores de teste preenchem TELA TODA?
- ✅ Grade tem 4 cores nos cantos?
- ✅ Cruz branca centralizada?
- ✅ Olhos SEPARADOS (não sobrepostos)?
- ✅ Olhos na mesma linha horizontal?
- ✅ Sem faixa branca lateral?

---

## 📝 Arquivos Modificados

- **`src/display/main.cpp`**:
  - Linha 68-80: Função `updateEyePositions()`
  - Linha 466-480: Teste automático de rotações
  - Linha 498-515: Grade de orientação
  - Linha 526: Chamada `updateEyePositions()`

---

## 🔧 Próximos Passos

### Se funcionar perfeitamente:

1. ✅ TFT OK
2. ✅ Orientação OK
3. → Próximo: Reativar RoboEyes com sprite corrigido

### Se NÃO funcionar:

1. Enviar log completo do Serial Monitor
2. Tirar foto da tela mostrando:
   - Grade de orientação
   - Olhos
3. Informar: "Resolução: ?" que aparece no Serial

---

**Status**: ✅ **UPLOAD COMPLETO**  
**Mudança Principal**: Detecção automática de rotação landscape  
**Cálculo Dinâmico**: Posições adaptam à resolução real  
**Teste Visual**: Grade de orientação para validação  
**Data**: 15 de Dezembro de 2024

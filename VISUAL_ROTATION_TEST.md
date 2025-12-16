# 🎯 Teste Visual de Rotação - Identificação Manual

## ❌ Problema Persistente

Usuário reporta:
- Quadrado azul no canto inferior esquerdo
- Quadrado vermelho no meio da tela
- Faixa branca lateral
- Orientação ainda errada

**Diagnóstico**: A rotação automática não está funcionando corretamente.

---

## ✅ Nova Abordagem: Teste Visual de TODAS as Rotações

O código agora **mostra cada uma das 4 rotações** por 2 segundos, com texto identificando:

### Sequência de Testes (8 segundos):

```
ROT 0                  ROT 1
240x320                320x240
🔴──🟢                 🔴──────────🟢
│  │                   │          │
└──┘                   └──────────┘
(2s)          →        (2s)


ROT 2                  ROT 3
240x320                320x240
──┐                    ──────────┐
  │                              │
🔴🟢                   🔴────────🟢
(2s)          →        (2s)
```

**O que observar**:
- **Texto "ROT X"** legível → Orientação correta para aquela rotação
- **Dimensões** mostradas
- **Quadrados vermelho e verde** nos cantos

---

## 📺 Teste FINAL (5 segundos)

Após testar as 4 rotações, o código **força rotação 1** e mostra:

```
┌──────────────────────────────────┐
│ LANDSCAPE  🔴                🟢  │ ← Texto + cantos
│ 320x240                          │
│                                  │
│                                  │
│──────────────+───────────────────│ ← Cruz branca
│                                  │
│                                  │
│ 🔵                          🟡   │
└──────────────────────────────────┘

🔴 = RED (superior esquerdo)
🟢 = GREEN (superior direito)
🔵 = BLUE (inferior esquerdo)
🟡 = YELLOW (inferior direito)
```

---

## ✅ Como Interpretar

### **Se você vê "LANDSCAPE" LEGÍVEL no topo esquerdo**:

✅ **ORIENTAÇÃO CORRETA!**

Deve ver:
- Texto "LANDSCAPE" horizontal e legível
- "320x240" embaixo
- 4 quadrados coloridos nos cantos
- Cruz branca centralizada

---

### **Se você NÃO vê "LANDSCAPE" legível**:

❌ **ORIENTAÇÃO ERRADA!**

Possíveis casos:

**Caso 1: Texto de lado (vertical)**
```
L
A
N
D
S
C
A
P
E
```
→ Rotação 0 ou 2 (portrait)
→ Precisa usar rotação 1 ou 3

**Caso 2: Texto de cabeça para baixo**
```
EPACSDNAL
```
→ Rotação invertida
→ Tentar rotação oposta

**Caso 3: Texto cortado ou fora da tela**
```
LANDS...
```
→ Problema de dimensões
→ Verificar User_Setup.h

---

## 🎯 Instrução ao Usuário

### **Durante os primeiros 8 segundos** (teste de rotações):

Observe QUAL das 4 telas mostra:
1. ✅ Texto "ROT X" **LEGÍVEL** (horizontal)
2. ✅ Quadrados vermelho (esquerda) e verde (direita) **SEPARADOS**
3. ✅ Dimensões "320x240" (não 240x320)

**Anote o número da rotação que parecer correta!**

---

### **Nos 5 segundos finais** (teste LANDSCAPE):

Verifique se:
1. ✅ "LANDSCAPE" está legível horizontalmente?
2. ✅ "320x240" aparece embaixo?
3. ✅ RED superior esquerdo, GREEN superior direito?
4. ✅ BLUE inferior esquerdo, YELLOW inferior direito?
5. ✅ Cruz branca centralizada?

**Se SIM para todas**: ✅ Rotação correta!  
**Se NÃO**: ❌ Me diga o que você vê exatamente!

---

## 📊 Possíveis Resultados

### Resultado A: "LANDSCAPE" legível, grade OK

```
✅ SUCESSO!
Rotação 1 está correta (320x240)
Problema anterior era configuração
Próximo: Ativar olhos
```

### Resultado B: Texto vertical ou ilegível

```
❌ PROBLEMA de rotação
Rotação 1 não é landscape neste hardware
Solução: Testar rotação 3:
  tft.setRotation(3);
```

### Resultado C: Tudo distorcido/cortado

```
❌ PROBLEMA de User_Setup.h
Configuração errada do TFT_eSPI
Solução: Verificar:
  - TFT_WIDTH e TFT_HEIGHT
  - Driver (ILI9341)
  - Pinos SPI
```

---

## 🔍 Debug Detalhado

### Monitor Serial mostrará:

```
⚙️  TESTANDO TODAS AS 4 ROTAÇÕES:
  Rotação 0: 240x320  ← Portrait
  Rotação 1: 320x240  ← Landscape ✅
  Rotação 2: 240x320  ← Portrait
  Rotação 3: 320x240  ← Landscape ✅

✅ FORÇANDO rotação 1: 320x240

📺 OLHE PARA A TELA:
  Se você vê 'LANDSCAPE' no topo esquerdo legível,
  a orientação está CORRETA!
  RED no canto superior esquerdo
  GREEN no canto superior direito
  BLUE no canto inferior esquerdo
  YELLOW no canto inferior direito
```

---

## 🐛 Troubleshooting

### Se NENHUMA das 4 rotações mostrar texto legível:

**Problema**: Configuração básica do TFT errada

**Verificar**:
1. `User_Setup.h` ou `User_Setup_Select.h`
2. Driver selecionado: `#define ILI9341_DRIVER`
3. Pinos corretos:
   ```cpp
   #define TFT_MOSI 13
   #define TFT_MISO 12
   #define TFT_SCLK 14
   #define TFT_CS   15
   #define TFT_DC   2
   #define TFT_RST  -1
   ```

### Se rotação 1 mostra 240x320 (portrait):

**Problema**: TFT_eSPI configurado com dimensões invertidas

**Solução temporária**:
```cpp
// Trocar para rotação 3:
tft.setRotation(3);
```

### Se grade aparece, mas olhos não:

**Problema**: Posições calculadas erradas

**Debug**:
```cpp
Serial.printf("Olho L: (%d, %d)\n", eyeLeftX, eyeY);
Serial.printf("Olho R: (%d, %d)\n", eyeRightX, eyeY);
// Devem ser valores dentro da tela
```

---

## 📝 O Que Reportar

Por favor, me informe:

1. **Durante teste de rotações (0-3)**:
   - Qual rotação mostrou texto legível?
   - Qual mostrou "320x240"?

2. **No teste FINAL (5s)**:
   - Você vê "LANDSCAPE" legível?
   - Onde estão os quadrados coloridos?
   - O texto está horizontal, vertical ou invertido?

3. **Monitor Serial**:
   - Copie a saída das rotações
   - Especialmente: "FORÇANDO rotação X: WxH"

4. **Foto** (se possível):
   - Da tela durante o teste FINAL de 5s

---

## 🎯 Próximos Passos

### Se funcionar (LANDSCAPE legível):

1. ✅ Confirmar rotação correta
2. → Ativar desenho dos olhos
3. → Verificar animação
4. → Reativar RoboEyes com sprite

### Se NÃO funcionar:

1. Identificar qual rotação funciona (0, 1, 2 ou 3)
2. Forçar essa rotação manualmente
3. Verificar User_Setup.h se nenhuma funcionar
4. Considerar testar com exemplo básico do TFT_eSPI

---

**Status**: ✅ **TESTE VISUAL IMPLEMENTADO**  
**Objetivo**: Identificar manualmente a rotação correta  
**Duração**: 8s (rotações) + 5s (grade final) = 13s  
**Data**: 15 de Dezembro de 2024

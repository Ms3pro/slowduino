/**
 * @file diag.cpp
 * @brief Instrumentação de diagnóstico (ver diag.h)
 */

#include "diag.h"

#if defined(DIAG_BUILD)

#define DIAG_LED PC13  // Blue Pill: LED onboard, ativo em nível baixo

// Definida no core STM32duino (usbd_cdc_if.c): só vira true quando o host
// manda SET_CONTROL_LINE_STATE com DTR (ou sempre, após Serial.dtr(false),
// mas apenas NO MOMENTO desse request).
extern __IO bool dtrState;

static uint8_t diagLastStage = 0;
static bool diagDtrSeen = false;
static bool diagRxSeen = false;
static uint32_t diagLastPrint = 0;

static inline void ledOn()  { digitalWrite(DIAG_LED, LOW); }
static inline void ledOff() { digitalWrite(DIAG_LED, HIGH); }

void diagInit() {
  pinMode(DIAG_LED, OUTPUT);
  ledOn();
  delay(1000);
  ledOff();
  delay(500);
  Serial2.begin(115200);
}

void diagStage(uint8_t stage) {
  diagLastStage = stage;
  for (uint8_t i = 0; i < stage; i++) {
    ledOn();
    delay(80);
    ledOff();
    delay(120);
  }
  delay(300);
}

void diagLoop() {
  uint32_t now = millis();

  if (dtrState) diagDtrSeen = true;
  if (Serial.available()) diagRxSeen = true;

  // Contorno: depois de 5 s no ar, força a linha DTR como "presente". Sem
  // isso, se o host nunca mandar o request de linha de controle, o core
  // descarta TODAS as escritas no Serial USB (CDC_connected() == false).
  // diagDtrSeen guarda o valor REAL visto antes do contorno.
  if (now > 5000) dtrState = true;

  // LED: ciclo de 2 s
  uint16_t t = (uint16_t)(now % 2000);
  bool on = (t < 400)
         || (diagDtrSeen && t >= 600 && t < 700)
         || (diagRxSeen  && t >= 900 && t < 1000);
  if (on) ledOn(); else ledOff();

  // Texto de vida, só até o host falar conosco
  if (!diagRxSeen && (now - diagLastPrint) >= 1000) {
    diagLastPrint = now;
    for (uint8_t port = 0; port < 2; port++) {
      Print& out = (port == 0) ? (Print&)Serial : (Print&)Serial2;
      out.print(F("DIAG stage="));
      out.print(diagLastStage);
      out.print(F(" up="));
      out.print(now / 1000);
      out.print(F("s dtrSeen="));
      out.print(diagDtrSeen ? 1 : 0);
      out.print(F(" rx="));
      out.println(diagRxSeen ? 1 : 0);
    }
  }
}

#endif // DIAG_BUILD

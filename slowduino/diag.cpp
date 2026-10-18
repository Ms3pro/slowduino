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
static bool diagRtsSeen = false;
static bool diagUsbRxSeen = false;
static uint32_t diagUartRxCount = 0;
static uint32_t diagUsbRxCount = 0;
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

static void diagPrintLine(Print& out, const __FlashStringHelper* label, uint32_t now) {
  out.print(F("DIAG["));
  out.print(label);
  out.print(F("] stage="));
  out.print(diagLastStage);
  out.print(F(" up="));
  out.print(now / 1000);
  out.print(F("s dtrSeen="));
  out.print(diagDtrSeen ? 1 : 0);
  out.print(F(" rtsSeen="));
  out.print(diagRtsSeen ? 1 : 0);
  out.print(F(" hostBaud="));
  out.print(Serial.baud());
  out.print(F(" rxUSB="));
  out.print(diagUsbRxCount);
  out.print(F(" rxUART2="));
  out.println(diagUartRxCount);
}

void diagLoop() {
  uint32_t now = millis();

  if (dtrState) diagDtrSeen = true;
  if (Serial.rts()) diagRtsSeen = true;

  // A USART2 não é usada pelo protocolo: consumimos aqui só para contar e
  // avisar que o firmware está ouvindo nela.
  while (Serial2.available()) {
    Serial2.read();
    diagUartRxCount++;
  }

  // USB: só olha o buffer (não consome), quem consome é o commsProcess().
  int usbAvail = Serial.available();
  if (usbAvail > 0) {
    diagUsbRxSeen = true;
    if (usbAvail > (int)diagUsbRxCount) diagUsbRxCount = usbAvail;
    if (diagUsbRxCount == 0) diagUsbRxCount = 1;
  }

  // Contorno: depois de 5 s no ar, força a linha DTR como "presente". Sem
  // isso, se o host nunca mandar o request de linha de controle, o core
  // descarta TODAS as escritas no Serial USB (CDC_connected() == false).
  // diagDtrSeen guarda o valor REAL visto antes do contorno.
  if (now > 5000) dtrState = true;

  // LED: ciclo de 2 s
  uint16_t t = (uint16_t)(now % 2000);
  bool on = (t < 400)
         || (diagDtrSeen      && t >=  600 && t <  700)
         || (diagUsbRxSeen    && t >=  900 && t < 1000)
         || (diagUartRxCount  && t >= 1200 && t < 1300);
  if (on) ledOn(); else ledOff();

  // Texto de vida, só até o host falar conosco pela USB
  if (!diagUsbRxSeen && (now - diagLastPrint) >= 1000) {
    diagLastPrint = now;
    diagPrintLine(Serial,  F("USB"),   now);
    diagPrintLine(Serial2, F("UART2"), now);
  }
}

#endif // DIAG_BUILD

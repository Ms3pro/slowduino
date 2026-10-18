/**
 * @file diag.h
 * @brief Instrumentação de diagnóstico (só existe com -DDIAG_BUILD)
 *
 * Usada pelo env `bluepill_diag` para investigar a issue #4 (STM32 não
 * responde no TunerStudio). Nos builds normais todas as macros viram no-op e
 * diag.cpp não gera código.
 *
 * LED PC13 (Blue Pill, aceso = nível baixo):
 *  - Boot: 1 s aceso = setup() foi alcançado. Depois, N piscadas rápidas por
 *    etapa concluída: 1=Serial 2=tabelas 3=storage 4=sensores 5=scheduler
 *    6=trigger 7=comms 8=auxiliares 9=setup completo. Se parar em N, a
 *    etapa N+1 travou.
 *  - Loop vivo, ciclo de 2 s: 1 piscada longa (loop rodando), +1 curta se o
 *    host já abriu a porta USB (DTR visto), +1 curta se o firmware já
 *    recebeu algum byte do host.
 *  - Texto "DIAG ..." a cada 1 s na USB e na USART2 (PA2=TX, PA3=RX, 115200)
 *    até o primeiro byte chegar do host; depois disso fica quieto para não
 *    atrapalhar o protocolo do TunerStudio.
 */

#ifndef DIAG_H
#define DIAG_H

#include <Arduino.h>

#if defined(DIAG_BUILD)
void diagInit();
void diagStage(uint8_t stage);
void diagLoop();
#define DIAG_INIT()      diagInit()
#define DIAG_STAGE(n)    diagStage(n)
#define DIAG_LOOP()      diagLoop()
#else
#define DIAG_INIT()      ((void)0)
#define DIAG_STAGE(n)    ((void)0)
#define DIAG_LOOP()      ((void)0)
#endif

#endif // DIAG_H

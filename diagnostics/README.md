# Diagnóstico STM32 Blue Pill (issue #4)

`slowduino-bluepill-diag.bin` é o firmware do `env:bluepill_diag`: o mesmo
firmware do Blue Pill, mais um LED de progresso (PC13) e mensagens "DIAG"
na USB e na USART2. Foi compilado do código desta branch.

Gravar em 0x08000000 (ST-Link / STM32CubeProgrammer). O boot leva ~12 s.

## LED PC13

1. **1 s aceso** logo após ligar: `setup()` foi alcançado.
2. **Piscadas rápidas em grupos**: grupo N = etapa N concluída
   (1 Serial, 2 tabelas, 3 storage, 4 sensores, 5 scheduler, 6 trigger,
   7 comms, 8 auxiliares, 9 setup completo). Se parar no grupo N, a etapa
   N+1 travou.
3. **Depois do boot**, ciclo de 2 s: 1 piscada longa (loop vivo), +1 curta
   se o PC já abriu a porta USB, +1 curta se o firmware recebeu algum byte.

## Texto

A cada 1 s, até chegar o primeiro byte do PC, sai pela USB (e pela USART2,
PA2=TX, 115200):

    DIAG stage=9 up=23s dtrSeen=1 rx=0

Depois do primeiro byte o firmware fica quieto e responde normalmente ao
protocolo (`Q` -> `speeduino 202402(s)`).

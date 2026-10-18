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
   se o PC já abriu a porta USB, +1 curta se o firmware recebeu algum byte
   pela USB, +1 curta (mais ao fim do ciclo) se recebeu byte pela USART2.

## Texto

A cada 1 s, até chegar o primeiro byte pela USB, saem DUAS linhas: uma pela
USB e outra pela USART2 (PA2=TX, 115200), identificadas no começo:

    DIAG[USB] stage=9 up=23s dtrSeen=1 rtsSeen=1 hostBaud=115200 rxUSB=0 rxUART2=0
    DIAG[UART2] stage=9 up=23s ...

`hostBaud` é a velocidade que o PC configurou na porta USB (prova que o PC
falou com o dispositivo). `rxUSB`/`rxUART2` contam bytes recebidos.

Depois do primeiro byte o firmware fica quieto e responde normalmente ao
protocolo (`Q` -> `speeduino 202402(s)`).

## slowduino-bluepill-uart.bin (`env:bluepill_uart`)

Firmware normal, SEM USB CDC: o `Serial` fica na **USART2** (PA2=TX, PA3=RX,
115200). Use com um adaptador USB-TTL de 3,3 V (TX do adaptador no PA3, RX do
adaptador no PA2, GND comum) e conecte o TunerStudio à porta do adaptador.
Não use PA9/PA10 (são saídas de ignição). Não depende da USB do Blue Pill.

.origin 0 // offset of the start of the code in PRU memory
.entrypoint START // program entry point, used by debugger only

#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3 // corresponds to PRU_EVTOUT_0

#define CLOCK 200000000 // PRU is always clocked at 200MHz

#define MS (CLOCK / 1000)
#define US (MS / 1000)

#define DELAYCTR r10
#define TXCTR r11

.macro BOARDON
    SET r30,r30, 15
.endm

.macro BOARDOFF
    CLR r30,r30, 15
.endm

.macro BOARDRESET
    BOARDOFF
    DELAY_CNST 5*MS
    BOARDON
    DELAY_CNST 60*MS
.endm

.macro DELAY_CNST
.mparam delaycnt 
    MOV DELAYCTR, ((delaycnt/2)-1)
LOOP:
    SUB DELAYCTR, DELAYCTR, 1
    QBNE LOOP, DELAYCTR, 0
.endm

.macro DOHIGH
    SET r30, r30, 14
.endm
.macro DOLOW
    CLR r30, r30, 14
.endm


.macro PULSE_CNST
.mparam duration
    DOHIGH
    DELAY_CNST (duration-2)
    DOLOW
.endm


.macro RESETITER
.mparam iter=r0
    MOV iter, 0

    SUB TXCTR, TXCTR, 1
    QBEQ BAIL, TXCTR, 0
.endm

.macro READNEXTDWORD
.mparam dst=r1,src=r0
    LBBO &dst, src, 0, 4
    ADD src, src, 4
.endm

.macro GETNEXTBURST
LOOP:
    READNEXTDWORD
    QBEQ TMPDELAY, r1, 8
    QBNE RET, r1, 0    
    RESETITER
    JMP LOOP
TMPDELAY:
    DELAY_CNST (85*1000*US)
    JMP LOOP
RET:
.endm

START:
    DOLOW
    MOV TXCTR, 2
    BOARDRESET
    RESETITER

ITER_VALUE:
    GETNEXTBURST
    PULSE_CNST 500*US
ITER_TIME: 
    DELAY_CNST ((1*US)-2)
    SUB r1, r1, 1
    QBNE ITER_TIME, r1, 0

    JMP ITER_VALUE


BAIL:
    DOLOW
    // tell host were done, then halt
    MOV R31.b0, PRU0_R31_VEC_VALID | SIGNUM
    HALT

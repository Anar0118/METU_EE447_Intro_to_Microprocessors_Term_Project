; mic_read.s
; Reads one audio sample from ADC0 Sample Sequencer 3 (SS3)
; Assumes ADC0 is already initialized in C (channel, SS3 enabled, etc.)

        THUMB
        AREA    |.text|, CODE, READONLY, ALIGN=2

; -------------------------------
; ADC0 register addresses
; -------------------------------
ADC0_BASE       EQU     0x40038000
ADC_PSSI        EQU     (ADC0_BASE + 0x028)   ; Processor Sample Seq Initiate
ADC_RIS         EQU     (ADC0_BASE + 0x004)   ; Raw Interrupt Status
ADC_ISC         EQU     (ADC0_BASE + 0x00C)   ; Interrupt Status & Clear
ADC_SSFIFO3     EQU     (ADC0_BASE + 0x0A8)   ; SS3 FIFO

SS3_BIT         EQU     0x08                  ; bit3

        EXPORT  ADC0_SS3_ReadRaw
        EXPORT  ADC0_SS3_ReadCentered

; -----------------------------------------------------------
; uint16_t ADC0_SS3_ReadRaw(void)
; Returns: R0 = 12-bit raw sample [0..4095]
; -----------------------------------------------------------
ADC0_SS3_ReadRaw  PROC
        PUSH    {R1-R3, LR}

        ; Trigger SS3
        LDR     R1, =ADC_PSSI
        MOVS    R2, #SS3_BIT
        STR     R2, [R1]

wait_done
        ; Poll RIS bit3 until set
        LDR     R1, =ADC_RIS
        LDR     R3, [R1]
        TST     R3, #SS3_BIT
        BEQ     wait_done

        ; Read FIFO3
        LDR     R1, =ADC_SSFIFO3
        LDR     R0, [R1]
        ; Mask to 12 bits: R0 = R0 & 0x0FFF
        LSLS    R0, R0, #20
        LSRS    R0, R0, #20

        ; Clear completion flag (ISC bit3)
        LDR     R1, =ADC_ISC
        MOVS    R2, #SS3_BIT
        STR     R2, [R1]

        POP     {R1-R3, LR}
        BX      LR
        ENDP

; -----------------------------------------------------------
; int16_t ADC0_SS3_ReadCentered(void)
; Returns: R0 = centered sample in signed range approx [-2048..+2047]
; (raw - 2048). This removes 1.25V DC offset.
; -----------------------------------------------------------
ADC0_SS3_ReadCentered  PROC
        PUSH    {R1-R3, LR}

        ; Trigger SS3
        LDR     R1, =ADC_PSSI
        MOVS    R2, #SS3_BIT
        STR     R2, [R1]

wait_done_c
        LDR     R1, =ADC_RIS
        LDR     R3, [R1]
        TST     R3, #SS3_BIT
        BEQ     wait_done_c

        ; Read FIFO3
        LDR     R1, =ADC_SSFIFO3
        LDR     R0, [R1]
        ; Mask to 12 bits: R0 = R0 & 0x0FFF  (shift trick)
        LSLS    R0, R0, #20
        LSRS    R0, R0, #20

        ; Clear completion flag
        LDR     R1, =ADC_ISC
        MOVS    R2, #SS3_BIT
        STR     R2, [R1]

        ; Center: R0 = raw - 2048
        ; (2048 = 0x800)
        SUB     R0, R0, #0x800

        POP     {R1-R3, LR}
        BX      LR
        ENDP

        END

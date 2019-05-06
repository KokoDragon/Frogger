; Print.s
; Student names: Stephen Chang and Mayank Shouche
; Last modification date: 3/31/19
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

quotient EQU 0
remainder EQU 4
number EQU 1000000000
	
;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
	PUSH{R0, LR}
	  ;MOV R4, #0
	  SUB SP, #8
	  MOV R12, #10
	  LDR R1, =number
	  CMP R0, #0
	  BEQ diveq1
divloop
	  UDIV R1, R12
	  CMP R1, R0
	  BHI divloop

recursion
	  MOV R3, #0  ;counter
	  CMP R1, #1
	  BEQ diveq1  ;return number once div equals 1
	  
L1
	  CMP R0, R1
	  BLO div10
	  SUB R0, R0, R1
	  ADD R3, #1       ;R3 is counter
	  B L1
div10
	  UDIV R1, R12
	  B done
diveq1
	  ADD R0, #0x30
	  BL ST7735_OutChar
	  ADD SP, #8
	  POP {R0, PC}
	  BX LR
done
	  ;ADD R4, R4, #1   ;R4 is recursive counter
	  ;PUSH {R3, R4}
	  MOV R2, R0
	  MOV R0, R3
	  ADD R0, #0x30
	  PUSH{R1,R2,R6,R7}
	  BL ST7735_OutChar
	  POP{R1,R2,R6,R7}
	  MOV R0, R2
	  B recursion
      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11

counter EQU 0
number1 EQU 9999
LCD_OutFix
     PUSH {R0, LR}
	 SUB SP, #8 ;allocate space for local variables
	 
	 MOV R12, #0
	 STR R12, [SP, #counter] ;initialize counter local variable with 0
	 
	 LDR R12, =number1
	 CMP R0, R12
     BHI tooLarge     ;number is too large, print *.***

	MOV R1, #1000	 
	MOV R3, #10
	
div
	LDR R12, [SP, #counter]
	ADD R12, #1
	STR R12, [SP, #counter]

	CMP R12, #2
	BEQ period

	CMP R12, #6
	BEQ endFix

	UDIV R2, R0, R1
	CMP R2, #0
	BNE case1

	PUSH {R0, R1, R2, R3}
	MOV R0, #0x30  ;prints first zero
	BL ST7735_OutChar
	POP {R0, R1, R2, R3}	

	UDIV R1, R3
	B noperiod

period
	PUSH {R0, R1, R2, R3}
	MOV R0, #0x2E  ;prints decimal point
	BL ST7735_OutChar
	POP {R0, R1, R2, R3}

noperiod	
	B div

case1
	PUSH {R0, R1, R2, R3}
	MOV R0, R2  ;prints character
	ADD R0, #0x30
	BL ST7735_OutChar
	POP {R0, R1, R2, R3}
	
	MUL R12, R1, R2
	SUB R0, R0, R12

	CMP R1, #1
	BEQ endFix

	UDIV R1, R3
	B div

tooLarge
	MOV R0, #0x2A
	BL ST7735_OutChar
	MOV R0, #0x2E
	BL ST7735_OutChar
	MOV R0, #0X2A
	BL ST7735_OutChar
	MOV R0, #0X2A
	BL ST7735_OutChar
	MOV R0, #0X2A
	BL ST7735_OutChar

endFix
	 ADD SP, #8 ;de-allocate local variable space
	 POP {R0, LR}
     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file

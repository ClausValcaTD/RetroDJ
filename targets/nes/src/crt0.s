.export _init, _exit, __STARTUP__:absolute = 1
.exportzp _vblank_flag

.import _main
.import zerobss, copydata, initlib, donelib
.importzp sp

PPUCTRL   = $2000
PPUMASK   = $2001
PPUSTATUS = $2002
OAMADDR   = $2003
PPUSCROLL = $2005
PPUADDR   = $2006
PPUDATA   = $2007
APU_STATUS= $4015
APU_FRAME = $4017

.segment "ZEROPAGE"
_vblank_flag: .res 1

.segment "HEADER"
    .byte "NES", $1A ; iNES magic
    .byte 2          ; 2x 16KB PRG = 32KB
    .byte 1          ; 1x 8KB CHR = 8KB
    .byte $01        ; NROM, Vertical Mirroring
    .byte $00        ; Mapper 0
    .byte 0,0,0,0,0,0,0,0

.segment "CODE"

_init:
_exit:
reset:
    sei
    cld
    ldx #$FF
    txs
    inx
    stx PPUCTRL
    stx PPUMASK
    stx APU_STATUS

    ; Disable frame IRQ
    lda #$40
    sta APU_FRAME

:   lda PPUSTATUS
    bpl :-

    ; Clear RAM
    txa
:   sta $000, x
    sta $100, x
    sta $200, x
    sta $300, x
    sta $400, x
    sta $500, x
    sta $600, x
    sta $700, x
    inx
    bne :-

:   lda PPUSTATUS
    bpl :-

    ; Initialize cc65 stack pointer at $0800
    lda #<$0800
    sta sp
    lda #>$0800
    sta sp+1

    jsr zerobss
    jsr copydata
    jsr initlib

    ; Enable NMI
    lda #$80
    sta PPUCTRL

    jsr _main
    jsr donelib

nmi:
    pha
    txa
    pha
    tya
    pha

    inc _vblank_flag

    pla
    tay
    pla
    tax
    pla
    rti

irq:
    rti

.segment "VECTORS"
    .word nmi
    .word reset
    .word irq

.segment "CHARS"
    ; Character tile graphics generator for ASCII table (0..127)
    ; Standard 8x8 font pattern data for tracker UI text display
    .res $2000, $00

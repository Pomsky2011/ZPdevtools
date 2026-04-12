; Play smooth sine wave sample - 440 Hz sine wave
; Loads and plays on left channel 0

start:
    ; Write SST data to $9000
    SDP $90
    SDB $00

    ; Block 0
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68

    ; Block 1
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75

    ; Block 2
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F

    ; Block 3
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A

    ; Block 4
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89

    ; Block 5
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC

    ; Block 6
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63

    ; Block 7
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78

    ; Block 8
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16

    ; Block 9
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F

    ; Block 10
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86

    ; Block 11
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5

    ; Block 12
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D

    ; Block 13
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A

    ; Block 14
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E

    ; Block 15
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4

    ; Block 16
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84

    ; Block 17
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD

    ; Block 18
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58

    ; Block 19
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C

    ; Block 20
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26

    ; Block 21
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA

    ; Block 22
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82

    ; Block 23
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5

    ; Block 24
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52

    ; Block 25
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 26
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E

    ; Block 27
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0

    ; Block 28
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81

    ; Block 29
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE

    ; Block 30
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C

    ; Block 31
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 32
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35

    ; Block 33
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7

    ; Block 34
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80

    ; Block 35
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6

    ; Block 36
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45

    ; Block 37
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 38
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C

    ; Block 39
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD

    ; Block 40
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80

    ; Block 41
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF

    ; Block 42
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E

    ; Block 43
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F

    ; Block 44
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43

    ; Block 45
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5

    ; Block 46
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80

    ; Block 47
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8

    ; Block 48
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37

    ; Block 49
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F

    ; Block 50
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A

    ; Block 51
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC

    ; Block 52
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80

    ; Block 53
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2

    ; Block 54
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30

    ; Block 55
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E

    ; Block 56
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50

    ; Block 57
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3

    ; Block 58
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82

    ; Block 59
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC

    ; Block 60
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28

    ; Block 61
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D

    ; Block 62
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56

    ; Block 63
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB

    ; Block 64
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83

    ; Block 65
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6

    ; Block 66
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20

    ; Block 67
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B

    ; Block 68
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C

    ; Block 69
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3

    ; Block 70
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85

    ; Block 71
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0

    ; Block 72
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18

    ; Block 73
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78

    ; Block 74
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61

    ; Block 75
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA

    ; Block 76
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88

    ; Block 77
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B

    ; Block 78
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11

    ; Block 79
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75

    ; Block 80
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66

    ; Block 81
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2

    ; Block 82
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B

    ; Block 83
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96

    ; Block 84
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09

    ; Block 85
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72

    ; Block 86
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B

    ; Block 87
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA

    ; Block 88
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F

    ; Block 89
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92

    ; Block 90
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01

    ; Block 91
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E

    ; Block 92
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F

    ; Block 93
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03

    ; Block 94
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93

    ; Block 95
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E

    ; Block 96
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8

    ; Block 97
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A

    ; Block 98
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73

    ; Block 99
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B

    ; Block 100
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97

    ; Block 101
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A

    ; Block 102
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0

    ; Block 103
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65

    ; Block 104
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76

    ; Block 105
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13

    ; Block 106
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C

    ; Block 107
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87

    ; Block 108
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9

    ; Block 109
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60

    ; Block 110
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79

    ; Block 111
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A

    ; Block 112
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2

    ; Block 113
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85

    ; Block 114
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1

    ; Block 115
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B

    ; Block 116
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B

    ; Block 117
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22

    ; Block 118
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7

    ; Block 119
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83

    ; Block 120
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9

    ; Block 121
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55

    ; Block 122
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D

    ; Block 123
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A

    ; Block 124
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD

    ; Block 125
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81

    ; Block 126
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1

    ; Block 127
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F

    ; Block 128
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 129
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31

    ; Block 130
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3

    ; Block 131
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80

    ; Block 132
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA

    ; Block 133
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48

    ; Block 134
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 135
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39

    ; Block 136
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA

    ; Block 137
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80

    ; Block 138
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3

    ; Block 139
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42

    ; Block 140
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 141
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40

    ; Block 142
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1

    ; Block 143
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80

    ; Block 144
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC

    ; Block 145
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B

    ; Block 146
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F

    ; Block 147
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47

    ; Block 148
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8

    ; Block 149
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80

    ; Block 150
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5

    ; Block 151
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33

    ; Block 152
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F

    ; Block 153
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D

    ; Block 154
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF

    ; Block 155
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81

    ; Block 156
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF

    ; Block 157
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C

    ; Block 158
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D

    ; Block 159
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53

    ; Block 160
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7

    ; Block 161
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82

    ; Block 162
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9

    ; Block 163
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24

    ; Block 164
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C

    ; Block 165
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59

    ; Block 166
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF

    ; Block 167
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84

    ; Block 168
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3

    ; Block 169
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C

    ; Block 170
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A

    ; Block 171
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F

    ; Block 172
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7

    ; Block 173
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87

    ; Block 174
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E

    ; Block 175
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15

    ; Block 176
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77

    ; Block 177
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64

    ; Block 178
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE

    ; Block 179
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A

    ; Block 180
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99

    ; Block 181
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D

    ; Block 182
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74

    ; Block 183
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69

    ; Block 184
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6

    ; Block 185
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D

    ; Block 186
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94

    ; Block 187
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05

    ; Block 188
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70

    ; Block 189
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D

    ; Block 190
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE

    ; Block 191
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91

    ; Block 192
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90

    ; Block 193
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC

    ; Block 194
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C

    ; Block 195
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71

    ; Block 196
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07

    ; Block 197
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95

    ; Block 198
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C

    ; Block 199
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4

    ; Block 200
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68

    ; Block 201
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75

    ; Block 202
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F

    ; Block 203
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A

    ; Block 204
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89

    ; Block 205
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC

    ; Block 206
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63

    ; Block 207
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78

    ; Block 208
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16

    ; Block 209
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F

    ; Block 210
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86

    ; Block 211
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5

    ; Block 212
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D

    ; Block 213
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A

    ; Block 214
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E

    ; Block 215
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4

    ; Block 216
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84

    ; Block 217
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD

    ; Block 218
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58

    ; Block 219
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C

    ; Block 220
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26

    ; Block 221
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA

    ; Block 222
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82

    ; Block 223
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5

    ; Block 224
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52

    ; Block 225
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 226
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E

    ; Block 227
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0

    ; Block 228
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81

    ; Block 229
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE

    ; Block 230
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C

    ; Block 231
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 232
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35

    ; Block 233
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7

    ; Block 234
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80

    ; Block 235
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6

    ; Block 236
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45

    ; Block 237
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 238
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C

    ; Block 239
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD

    ; Block 240
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80

    ; Block 241
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF

    ; Block 242
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E

    ; Block 243
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F

    ; Block 244
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43

    ; Block 245
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5

    ; Block 246
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80

    ; Block 247
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8

    ; Block 248
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37

    ; Block 249
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F

    ; Block 250
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A

    ; Block 251
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC

    ; Block 252
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80

    ; Block 253
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2

    ; Block 254
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30

    ; Block 255
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E

    ; Block 256
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50

    ; Block 257
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3

    ; Block 258
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82

    ; Block 259
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC

    ; Block 260
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28

    ; Block 261
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D

    ; Block 262
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56

    ; Block 263
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB

    ; Block 264
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83

    ; Block 265
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6

    ; Block 266
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20

    ; Block 267
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B

    ; Block 268
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C

    ; Block 269
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3

    ; Block 270
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85

    ; Block 271
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0

    ; Block 272
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18

    ; Block 273
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78

    ; Block 274
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61

    ; Block 275
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA

    ; Block 276
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88

    ; Block 277
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B

    ; Block 278
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11

    ; Block 279
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75

    ; Block 280
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66

    ; Block 281
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2

    ; Block 282
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B

    ; Block 283
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96

    ; Block 284
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09

    ; Block 285
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72

    ; Block 286
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B

    ; Block 287
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA

    ; Block 288
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F

    ; Block 289
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92

    ; Block 290
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01

    ; Block 291
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E

    ; Block 292
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F

    ; Block 293
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03

    ; Block 294
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93

    ; Block 295
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E

    ; Block 296
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8

    ; Block 297
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A

    ; Block 298
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73

    ; Block 299
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B

    ; Block 300
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97

    ; Block 301
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A

    ; Block 302
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0

    ; Block 303
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65

    ; Block 304
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76

    ; Block 305
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13

    ; Block 306
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C

    ; Block 307
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87

    ; Block 308
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9

    ; Block 309
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60

    ; Block 310
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79

    ; Block 311
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A

    ; Block 312
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2

    ; Block 313
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85

    ; Block 314
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1

    ; Block 315
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B

    ; Block 316
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B

    ; Block 317
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22

    ; Block 318
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7

    ; Block 319
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83

    ; Block 320
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9

    ; Block 321
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55

    ; Block 322
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D

    ; Block 323
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A

    ; Block 324
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD

    ; Block 325
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81

    ; Block 326
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1

    ; Block 327
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F

    ; Block 328
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 329
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31

    ; Block 330
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3

    ; Block 331
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80

    ; Block 332
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA

    ; Block 333
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48

    ; Block 334
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 335
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39

    ; Block 336
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA

    ; Block 337
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80

    ; Block 338
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3

    ; Block 339
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42

    ; Block 340
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 341
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40

    ; Block 342
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1

    ; Block 343
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80

    ; Block 344
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC

    ; Block 345
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B

    ; Block 346
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F

    ; Block 347
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47

    ; Block 348
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8

    ; Block 349
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80

    ; Block 350
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5

    ; Block 351
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33

    ; Block 352
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F

    ; Block 353
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D

    ; Block 354
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF

    ; Block 355
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81

    ; Block 356
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF

    ; Block 357
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C

    ; Block 358
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D

    ; Block 359
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53

    ; Block 360
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7

    ; Block 361
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82

    ; Block 362
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9

    ; Block 363
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24

    ; Block 364
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C

    ; Block 365
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59

    ; Block 366
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF

    ; Block 367
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84

    ; Block 368
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3

    ; Block 369
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C

    ; Block 370
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A

    ; Block 371
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F

    ; Block 372
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7

    ; Block 373
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87

    ; Block 374
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E

    ; Block 375
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15

    ; Block 376
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77

    ; Block 377
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64

    ; Block 378
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE

    ; Block 379
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A

    ; Block 380
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99

    ; Block 381
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D

    ; Block 382
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74

    ; Block 383
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69

    ; Block 384
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6

    ; Block 385
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D

    ; Block 386
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94

    ; Block 387
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05

    ; Block 388
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70

    ; Block 389
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D

    ; Block 390
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE

    ; Block 391
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91

    ; Block 392
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90

    ; Block 393
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC

    ; Block 394
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C

    ; Block 395
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71

    ; Block 396
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07

    ; Block 397
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95

    ; Block 398
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C

    ; Block 399
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4

    ; Block 400
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68

    ; Block 401
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75

    ; Block 402
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F

    ; Block 403
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A

    ; Block 404
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89

    ; Block 405
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC

    ; Block 406
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63

    ; Block 407
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78

    ; Block 408
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16

    ; Block 409
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F

    ; Block 410
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86

    ; Block 411
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5

    ; Block 412
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D

    ; Block 413
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A

    ; Block 414
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E

    ; Block 415
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4

    ; Block 416
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84

    ; Block 417
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD

    ; Block 418
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58

    ; Block 419
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C

    ; Block 420
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26

    ; Block 421
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA

    ; Block 422
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82

    ; Block 423
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5

    ; Block 424
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52

    ; Block 425
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 426
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E

    ; Block 427
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0

    ; Block 428
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81

    ; Block 429
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE

    ; Block 430
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C

    ; Block 431
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 432
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35

    ; Block 433
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7

    ; Block 434
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80

    ; Block 435
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6

    ; Block 436
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45

    ; Block 437
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 438
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C

    ; Block 439
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD

    ; Block 440
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80

    ; Block 441
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF

    ; Block 442
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E

    ; Block 443
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F

    ; Block 444
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43

    ; Block 445
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5

    ; Block 446
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80

    ; Block 447
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8

    ; Block 448
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37

    ; Block 449
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F

    ; Block 450
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A

    ; Block 451
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC

    ; Block 452
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80

    ; Block 453
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2

    ; Block 454
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30

    ; Block 455
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E

    ; Block 456
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50

    ; Block 457
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3

    ; Block 458
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82

    ; Block 459
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC

    ; Block 460
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28

    ; Block 461
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D

    ; Block 462
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56

    ; Block 463
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB

    ; Block 464
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83

    ; Block 465
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6

    ; Block 466
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20

    ; Block 467
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B

    ; Block 468
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C

    ; Block 469
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3

    ; Block 470
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85

    ; Block 471
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0

    ; Block 472
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18

    ; Block 473
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78

    ; Block 474
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61

    ; Block 475
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA

    ; Block 476
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88

    ; Block 477
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B

    ; Block 478
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11

    ; Block 479
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75

    ; Block 480
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66

    ; Block 481
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2

    ; Block 482
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B

    ; Block 483
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96

    ; Block 484
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09

    ; Block 485
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72

    ; Block 486
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B

    ; Block 487
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA

    ; Block 488
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F

    ; Block 489
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92

    ; Block 490
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01

    ; Block 491
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E

    ; Block 492
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F

    ; Block 493
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03

    ; Block 494
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93

    ; Block 495
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E

    ; Block 496
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8

    ; Block 497
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A

    ; Block 498
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73

    ; Block 499
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B

    ; Block 500
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97

    ; Block 501
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A

    ; Block 502
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0

    ; Block 503
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65

    ; Block 504
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76

    ; Block 505
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13

    ; Block 506
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C

    ; Block 507
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87

    ; Block 508
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9

    ; Block 509
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60

    ; Block 510
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79

    ; Block 511
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A

    ; Block 512
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2

    ; Block 513
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85

    ; Block 514
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1

    ; Block 515
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B

    ; Block 516
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B

    ; Block 517
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22

    ; Block 518
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7

    ; Block 519
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83

    ; Block 520
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9

    ; Block 521
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55

    ; Block 522
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D

    ; Block 523
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A

    ; Block 524
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD

    ; Block 525
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81

    ; Block 526
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1

    ; Block 527
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F

    ; Block 528
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 529
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31

    ; Block 530
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3

    ; Block 531
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80

    ; Block 532
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA

    ; Block 533
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48

    ; Block 534
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 535
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39

    ; Block 536
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA

    ; Block 537
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80

    ; Block 538
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3

    ; Block 539
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42

    ; Block 540
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 541
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40

    ; Block 542
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1

    ; Block 543
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80

    ; Block 544
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC

    ; Block 545
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B

    ; Block 546
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F

    ; Block 547
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47

    ; Block 548
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8

    ; Block 549
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80

    ; Block 550
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5

    ; Block 551
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33

    ; Block 552
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F

    ; Block 553
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D

    ; Block 554
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF

    ; Block 555
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81

    ; Block 556
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF

    ; Block 557
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C

    ; Block 558
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D

    ; Block 559
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53

    ; Block 560
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7

    ; Block 561
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82

    ; Block 562
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9

    ; Block 563
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24

    ; Block 564
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C

    ; Block 565
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59

    ; Block 566
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF

    ; Block 567
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84

    ; Block 568
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3

    ; Block 569
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C

    ; Block 570
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A

    ; Block 571
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F

    ; Block 572
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7

    ; Block 573
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87

    ; Block 574
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E

    ; Block 575
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15

    ; Block 576
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77

    ; Block 577
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64

    ; Block 578
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE

    ; Block 579
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A

    ; Block 580
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99

    ; Block 581
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D

    ; Block 582
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74

    ; Block 583
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69

    ; Block 584
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6

    ; Block 585
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D

    ; Block 586
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94

    ; Block 587
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05

    ; Block 588
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70

    ; Block 589
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D

    ; Block 590
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE

    ; Block 591
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91

    ; Block 592
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90

    ; Block 593
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC

    ; Block 594
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C

    ; Block 595
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71

    ; Block 596
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07

    ; Block 597
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95

    ; Block 598
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C

    ; Block 599
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4

    ; Block 600
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68

    ; Block 601
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75

    ; Block 602
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F

    ; Block 603
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A

    ; Block 604
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89

    ; Block 605
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC

    ; Block 606
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63

    ; Block 607
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78

    ; Block 608
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16

    ; Block 609
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F

    ; Block 610
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86

    ; Block 611
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5

    ; Block 612
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D

    ; Block 613
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A

    ; Block 614
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E

    ; Block 615
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4

    ; Block 616
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84

    ; Block 617
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD

    ; Block 618
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58

    ; Block 619
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C

    ; Block 620
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26

    ; Block 621
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA

    ; Block 622
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82

    ; Block 623
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5

    ; Block 624
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52

    ; Block 625
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 626
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E

    ; Block 627
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0

    ; Block 628
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81

    ; Block 629
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE

    ; Block 630
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C

    ; Block 631
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 632
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35

    ; Block 633
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7

    ; Block 634
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80

    ; Block 635
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6

    ; Block 636
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45

    ; Block 637
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 638
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C

    ; Block 639
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD

    ; Block 640
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80

    ; Block 641
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF

    ; Block 642
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E

    ; Block 643
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F

    ; Block 644
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43

    ; Block 645
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5

    ; Block 646
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80

    ; Block 647
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8

    ; Block 648
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37

    ; Block 649
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F

    ; Block 650
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A

    ; Block 651
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC

    ; Block 652
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80

    ; Block 653
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2

    ; Block 654
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30

    ; Block 655
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E

    ; Block 656
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50

    ; Block 657
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3

    ; Block 658
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82

    ; Block 659
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC

    ; Block 660
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28

    ; Block 661
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D

    ; Block 662
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56

    ; Block 663
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB

    ; Block 664
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83

    ; Block 665
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6

    ; Block 666
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20

    ; Block 667
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B

    ; Block 668
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C

    ; Block 669
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3

    ; Block 670
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85

    ; Block 671
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0

    ; Block 672
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18

    ; Block 673
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78

    ; Block 674
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61

    ; Block 675
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA

    ; Block 676
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88

    ; Block 677
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B

    ; Block 678
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11

    ; Block 679
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75

    ; Block 680
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66

    ; Block 681
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2

    ; Block 682
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B

    ; Block 683
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96

    ; Block 684
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09

    ; Block 685
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72

    ; Block 686
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B

    ; Block 687
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA

    ; Block 688
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F

    ; Block 689
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92

    ; Block 690
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01

    ; Block 691
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E

    ; Block 692
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F

    ; Block 693
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03

    ; Block 694
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93

    ; Block 695
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E

    ; Block 696
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8

    ; Block 697
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A

    ; Block 698
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73

    ; Block 699
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B

    ; Block 700
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97

    ; Block 701
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A

    ; Block 702
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0

    ; Block 703
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65

    ; Block 704
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76

    ; Block 705
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13

    ; Block 706
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C

    ; Block 707
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87

    ; Block 708
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9

    ; Block 709
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60

    ; Block 710
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79

    ; Block 711
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A

    ; Block 712
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2

    ; Block 713
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85

    ; Block 714
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1

    ; Block 715
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B

    ; Block 716
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B

    ; Block 717
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22

    ; Block 718
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7

    ; Block 719
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83

    ; Block 720
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9

    ; Block 721
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55

    ; Block 722
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D

    ; Block 723
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A

    ; Block 724
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD

    ; Block 725
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81

    ; Block 726
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1

    ; Block 727
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F

    ; Block 728
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 729
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31

    ; Block 730
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3

    ; Block 731
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80

    ; Block 732
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA

    ; Block 733
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48

    ; Block 734
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 735
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39

    ; Block 736
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA

    ; Block 737
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80

    ; Block 738
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3

    ; Block 739
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42

    ; Block 740
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 741
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40

    ; Block 742
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1

    ; Block 743
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80

    ; Block 744
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC

    ; Block 745
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B

    ; Block 746
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F

    ; Block 747
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47

    ; Block 748
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8

    ; Block 749
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80

    ; Block 750
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5

    ; Block 751
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33

    ; Block 752
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F

    ; Block 753
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D

    ; Block 754
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF

    ; Block 755
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81

    ; Block 756
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF

    ; Block 757
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C

    ; Block 758
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D

    ; Block 759
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53

    ; Block 760
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7

    ; Block 761
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82

    ; Block 762
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9

    ; Block 763
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24

    ; Block 764
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C

    ; Block 765
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59

    ; Block 766
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF

    ; Block 767
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84

    ; Block 768
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3

    ; Block 769
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C

    ; Block 770
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A

    ; Block 771
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F

    ; Block 772
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7

    ; Block 773
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87

    ; Block 774
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E

    ; Block 775
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15

    ; Block 776
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77

    ; Block 777
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64

    ; Block 778
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE

    ; Block 779
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A

    ; Block 780
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99

    ; Block 781
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D

    ; Block 782
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74

    ; Block 783
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69

    ; Block 784
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6

    ; Block 785
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D

    ; Block 786
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94

    ; Block 787
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05

    ; Block 788
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70

    ; Block 789
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D

    ; Block 790
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE

    ; Block 791
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91

    ; Block 792
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90

    ; Block 793
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC

    ; Block 794
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C

    ; Block 795
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71

    ; Block 796
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07

    ; Block 797
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95

    ; Block 798
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C

    ; Block 799
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4

    ; Block 800
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68

    ; Block 801
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75

    ; Block 802
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F

    ; Block 803
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A

    ; Block 804
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89

    ; Block 805
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC

    ; Block 806
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63

    ; Block 807
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78

    ; Block 808
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16

    ; Block 809
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F

    ; Block 810
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86

    ; Block 811
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5

    ; Block 812
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D

    ; Block 813
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A

    ; Block 814
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E

    ; Block 815
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4

    ; Block 816
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84

    ; Block 817
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD

    ; Block 818
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58

    ; Block 819
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C

    ; Block 820
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26

    ; Block 821
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA

    ; Block 822
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82

    ; Block 823
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5

    ; Block 824
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52

    ; Block 825
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 826
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E

    ; Block 827
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0

    ; Block 828
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81

    ; Block 829
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE

    ; Block 830
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C

    ; Block 831
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 832
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35

    ; Block 833
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7

    ; Block 834
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80

    ; Block 835
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6

    ; Block 836
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45

    ; Block 837
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 838
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C

    ; Block 839
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD

    ; Block 840
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80

    ; Block 841
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF

    ; Block 842
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E

    ; Block 843
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F

    ; Block 844
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43

    ; Block 845
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5

    ; Block 846
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80

    ; Block 847
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8

    ; Block 848
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37

    ; Block 849
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F

    ; Block 850
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A

    ; Block 851
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC

    ; Block 852
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80

    ; Block 853
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2

    ; Block 854
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30

    ; Block 855
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E

    ; Block 856
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50

    ; Block 857
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3

    ; Block 858
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82

    ; Block 859
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC

    ; Block 860
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28

    ; Block 861
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D

    ; Block 862
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56

    ; Block 863
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB

    ; Block 864
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83

    ; Block 865
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6

    ; Block 866
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20

    ; Block 867
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B

    ; Block 868
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C

    ; Block 869
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3

    ; Block 870
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85

    ; Block 871
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0

    ; Block 872
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18

    ; Block 873
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78

    ; Block 874
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61

    ; Block 875
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA

    ; Block 876
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88

    ; Block 877
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B

    ; Block 878
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11

    ; Block 879
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75

    ; Block 880
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66

    ; Block 881
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2

    ; Block 882
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B

    ; Block 883
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96

    ; Block 884
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09

    ; Block 885
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72

    ; Block 886
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B

    ; Block 887
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA

    ; Block 888
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F

    ; Block 889
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92

    ; Block 890
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01

    ; Block 891
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E

    ; Block 892
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F

    ; Block 893
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03

    ; Block 894
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93

    ; Block 895
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E

    ; Block 896
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8

    ; Block 897
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A

    ; Block 898
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73

    ; Block 899
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B

    ; Block 900
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97

    ; Block 901
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A

    ; Block 902
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0

    ; Block 903
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65

    ; Block 904
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76

    ; Block 905
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13

    ; Block 906
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C

    ; Block 907
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87

    ; Block 908
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9

    ; Block 909
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60

    ; Block 910
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79

    ; Block 911
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A

    ; Block 912
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2

    ; Block 913
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85

    ; Block 914
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1

    ; Block 915
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B

    ; Block 916
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B

    ; Block 917
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22

    ; Block 918
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7

    ; Block 919
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83

    ; Block 920
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9

    ; Block 921
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55

    ; Block 922
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D

    ; Block 923
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A

    ; Block 924
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD

    ; Block 925
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81

    ; Block 926
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1

    ; Block 927
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F

    ; Block 928
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 929
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31

    ; Block 930
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3

    ; Block 931
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80

    ; Block 932
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA

    ; Block 933
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48

    ; Block 934
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 935
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39

    ; Block 936
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA

    ; Block 937
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80

    ; Block 938
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3

    ; Block 939
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42

    ; Block 940
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 941
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40

    ; Block 942
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1

    ; Block 943
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80

    ; Block 944
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC

    ; Block 945
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B

    ; Block 946
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F

    ; Block 947
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47

    ; Block 948
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8

    ; Block 949
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80

    ; Block 950
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5

    ; Block 951
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33

    ; Block 952
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F

    ; Block 953
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D

    ; Block 954
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF

    ; Block 955
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81

    ; Block 956
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF

    ; Block 957
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C

    ; Block 958
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D

    ; Block 959
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53

    ; Block 960
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7

    ; Block 961
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82

    ; Block 962
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9

    ; Block 963
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24

    ; Block 964
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C

    ; Block 965
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59

    ; Block 966
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF

    ; Block 967
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84

    ; Block 968
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3

    ; Block 969
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C

    ; Block 970
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A

    ; Block 971
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F

    ; Block 972
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7

    ; Block 973
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87

    ; Block 974
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E

    ; Block 975
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15

    ; Block 976
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77

    ; Block 977
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64

    ; Block 978
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE

    ; Block 979
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A

    ; Block 980
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99

    ; Block 981
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D

    ; Block 982
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74

    ; Block 983
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69

    ; Block 984
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6

    ; Block 985
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D

    ; Block 986
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94

    ; Block 987
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05

    ; Block 988
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70

    ; Block 989
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D

    ; Block 990
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE

    ; Block 991
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91

    ; Block 992
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90

    ; Block 993
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC

    ; Block 994
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C

    ; Block 995
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71

    ; Block 996
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07

    ; Block 997
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95

    ; Block 998
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C

    ; Block 999
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4

    ; Block 1000
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68

    ; Block 1001
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75

    ; Block 1002
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F

    ; Block 1003
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A

    ; Block 1004
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89

    ; Block 1005
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC

    ; Block 1006
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63

    ; Block 1007
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78

    ; Block 1008
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16

    ; Block 1009
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F

    ; Block 1010
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86

    ; Block 1011
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5

    ; Block 1012
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D

    ; Block 1013
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A

    ; Block 1014
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E

    ; Block 1015
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4

    ; Block 1016
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84

    ; Block 1017
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD

    ; Block 1018
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58

    ; Block 1019
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C

    ; Block 1020
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26

    ; Block 1021
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA

    ; Block 1022
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82

    ; Block 1023
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5

    ; Block 1024
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52

    ; Block 1025
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 1026
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E

    ; Block 1027
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0

    ; Block 1028
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81

    ; Block 1029
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE

    ; Block 1030
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C

    ; Block 1031
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 1032
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35

    ; Block 1033
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7

    ; Block 1034
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80

    ; Block 1035
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6

    ; Block 1036
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45

    ; Block 1037
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 1038
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C

    ; Block 1039
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD

    ; Block 1040
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80

    ; Block 1041
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF

    ; Block 1042
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E

    ; Block 1043
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F

    ; Block 1044
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43

    ; Block 1045
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5

    ; Block 1046
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80

    ; Block 1047
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8

    ; Block 1048
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37

    ; Block 1049
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F

    ; Block 1050
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A

    ; Block 1051
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC

    ; Block 1052
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80

    ; Block 1053
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2

    ; Block 1054
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30

    ; Block 1055
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E

    ; Block 1056
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50

    ; Block 1057
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3

    ; Block 1058
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82

    ; Block 1059
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC

    ; Block 1060
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28

    ; Block 1061
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D

    ; Block 1062
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56

    ; Block 1063
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB

    ; Block 1064
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83

    ; Block 1065
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6

    ; Block 1066
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20

    ; Block 1067
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B

    ; Block 1068
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C

    ; Block 1069
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3

    ; Block 1070
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85

    ; Block 1071
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0

    ; Block 1072
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18

    ; Block 1073
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78

    ; Block 1074
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61

    ; Block 1075
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA

    ; Block 1076
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88

    ; Block 1077
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B

    ; Block 1078
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11

    ; Block 1079
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75

    ; Block 1080
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66

    ; Block 1081
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2

    ; Block 1082
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B

    ; Block 1083
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96

    ; Block 1084
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09

    ; Block 1085
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72

    ; Block 1086
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B

    ; Block 1087
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA

    ; Block 1088
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F

    ; Block 1089
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92

    ; Block 1090
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01

    ; Block 1091
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E

    ; Block 1092
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F

    ; Block 1093
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03

    ; Block 1094
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93

    ; Block 1095
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E

    ; Block 1096
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8

    ; Block 1097
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A

    ; Block 1098
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73

    ; Block 1099
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B

    ; Block 1100
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97

    ; Block 1101
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A

    ; Block 1102
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0

    ; Block 1103
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65

    ; Block 1104
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76

    ; Block 1105
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13

    ; Block 1106
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C

    ; Block 1107
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87

    ; Block 1108
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9

    ; Block 1109
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60

    ; Block 1110
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79

    ; Block 1111
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A

    ; Block 1112
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2

    ; Block 1113
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85

    ; Block 1114
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1

    ; Block 1115
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B

    ; Block 1116
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B

    ; Block 1117
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22

    ; Block 1118
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7

    ; Block 1119
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83

    ; Block 1120
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9

    ; Block 1121
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55

    ; Block 1122
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D

    ; Block 1123
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A

    ; Block 1124
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD

    ; Block 1125
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81

    ; Block 1126
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1

    ; Block 1127
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F

    ; Block 1128
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 1129
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31

    ; Block 1130
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3

    ; Block 1131
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80

    ; Block 1132
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA

    ; Block 1133
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48

    ; Block 1134
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 1135
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39

    ; Block 1136
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA

    ; Block 1137
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80

    ; Block 1138
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3

    ; Block 1139
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42

    ; Block 1140
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 1141
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40

    ; Block 1142
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1

    ; Block 1143
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80

    ; Block 1144
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC

    ; Block 1145
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B

    ; Block 1146
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F

    ; Block 1147
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47

    ; Block 1148
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8

    ; Block 1149
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80

    ; Block 1150
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5

    ; Block 1151
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33

    ; Block 1152
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F

    ; Block 1153
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D

    ; Block 1154
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF

    ; Block 1155
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81

    ; Block 1156
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF

    ; Block 1157
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C

    ; Block 1158
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D

    ; Block 1159
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53

    ; Block 1160
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7

    ; Block 1161
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82

    ; Block 1162
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9

    ; Block 1163
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24

    ; Block 1164
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C

    ; Block 1165
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59

    ; Block 1166
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF

    ; Block 1167
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84

    ; Block 1168
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3

    ; Block 1169
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C

    ; Block 1170
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A

    ; Block 1171
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F

    ; Block 1172
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7

    ; Block 1173
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87

    ; Block 1174
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E

    ; Block 1175
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15

    ; Block 1176
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77

    ; Block 1177
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64

    ; Block 1178
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE

    ; Block 1179
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A

    ; Block 1180
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99

    ; Block 1181
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D

    ; Block 1182
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74

    ; Block 1183
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69

    ; Block 1184
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6

    ; Block 1185
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D

    ; Block 1186
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94

    ; Block 1187
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05

    ; Block 1188
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70

    ; Block 1189
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D

    ; Block 1190
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE

    ; Block 1191
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91

    ; Block 1192
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90

    ; Block 1193
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC

    ; Block 1194
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C

    ; Block 1195
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71

    ; Block 1196
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07

    ; Block 1197
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95

    ; Block 1198
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C

    ; Block 1199
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4

    ; Block 1200
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $0B
    WRH $16
    WRL $20
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68

    ; Block 1201
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75

    ; Block 1202
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F

    ; Block 1203
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A

    ; Block 1204
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $8E
    WRH $89
    WRL $85
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89

    ; Block 1205
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC

    ; Block 1206
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63

    ; Block 1207
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $6F
    WRH $74
    WRL $78
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78

    ; Block 1208
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $6E
    WRH $68
    WRL $61
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16

    ; Block 1209
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F

    ; Block 1210
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $92
    WRH $8D
    WRL $88
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86

    ; Block 1211
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5

    ; Block 1212
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D

    ; Block 1213
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $6B
    WRH $71
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A

    ; Block 1214
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $72
    WRH $6D
    WRL $66
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E

    ; Block 1215
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4

    ; Block 1216
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84

    ; Block 1217
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $8B
    WRH $90
    WRL $96
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD

    ; Block 1218
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58

    ; Block 1219
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $66
    WRH $6D
    WRL $72
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C

    ; Block 1220
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $75
    WRH $71
    WRL $6B
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26

    ; Block 1221
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA

    ; Block 1222
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82

    ; Block 1223
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $88
    WRH $8D
    WRL $92
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5

    ; Block 1224
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52

    ; Block 1225
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 1226
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E

    ; Block 1227
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $18
    WRH $0E
    WRL $03
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0

    ; Block 1228
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81

    ; Block 1229
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $85
    WRH $89
    WRL $8E
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE

    ; Block 1230
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C

    ; Block 1231
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F

    ; Block 1232
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35

    ; Block 1233
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7

    ; Block 1234
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80

    ; Block 1235
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6

    ; Block 1236
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45

    ; Block 1237
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F

    ; Block 1238
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C

    ; Block 1239
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $28
    WRH $1D
    WRL $13
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD

    ; Block 1240
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80

    ; Block 1241
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $82
    WRH $84
    WRL $87
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF

    ; Block 1242
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E

    ; Block 1243
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $50
    WRH $59
    WRL $60
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F

    ; Block 1244
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43

    ; Block 1245
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5

    ; Block 1246
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80

    ; Block 1247
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $82
    WRL $85
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8

    ; Block 1248
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37

    ; Block 1249
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F

    ; Block 1250
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A

    ; Block 1251
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $41
    WRL $37
    WRH $2D
    WRL $22
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC

    ; Block 1252
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80

    ; Block 1253
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2

    ; Block 1254
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30

    ; Block 1255
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E

    ; Block 1256
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50

    ; Block 1257
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3

    ; Block 1258
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82

    ; Block 1259
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC

    ; Block 1260
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28

    ; Block 1261
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D

    ; Block 1262
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56

    ; Block 1263
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB

    ; Block 1264
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83

    ; Block 1265
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6

    ; Block 1266
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $0B
    WRH $16
    WRL $20

    ; Block 1267
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $2B
    WRL $35
    WRH $3F
    WRL $48
    WRH $51
    WRL $59
    WRH $61
    WRL $68
    WRH $6E
    WRL $73
    WRH $77
    WRL $7B

    ; Block 1268
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $70
    WRL $6A
    WRH $63
    WRL $5C

    ; Block 1269
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $54
    WRL $4C
    WRH $42
    WRL $39
    WRH $2F
    WRL $24
    WRH $19
    WRL $0F
    WRH $04
    WRL $F8
    WRH $ED
    WRL $E3

    ; Block 1270
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D8
    WRL $CE
    WRH $C4
    WRL $BA
    WRH $B1
    WRL $A9
    WRH $A1
    WRL $9A
    WRH $93
    WRL $8E
    WRH $89
    WRL $85

    ; Block 1271
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $81
    WRL $82
    WRH $85
    WRL $89
    WRH $8D
    WRL $93
    WRH $99
    WRL $A0

    ; Block 1272
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A8
    WRL $B0
    WRH $B9
    WRL $C3
    WRH $CD
    WRL $D7
    WRH $E2
    WRL $EC
    WRH $F7
    WRL $03
    WRH $0E
    WRL $18

    ; Block 1273
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $23
    WRL $2E
    WRH $38
    WRL $42
    WRH $4B
    WRL $53
    WRH $5B
    WRL $63
    WRH $69
    WRL $6F
    WRH $74
    WRL $78

    ; Block 1274
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7B
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D
    WRH $7B
    WRL $78
    WRH $73
    WRL $6E
    WRH $68
    WRL $61

    ; Block 1275
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5A
    WRL $52
    WRH $49
    WRL $40
    WRH $36
    WRL $2C
    WRH $21
    WRL $16
    WRH $0C
    WRL $01
    WRH $F5
    WRL $EA

    ; Block 1276
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E0
    WRL $D5
    WRH $CB
    WRL $C1
    WRH $B8
    WRL $AF
    WRH $A6
    WRL $9F
    WRH $98
    WRL $92
    WRH $8D
    WRL $88

    ; Block 1277
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $85
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $86
    WRH $8A
    WRL $8F
    WRH $95
    WRL $9B

    ; Block 1278
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A2
    WRL $AA
    WRH $B3
    WRL $BC
    WRH $C5
    WRL $CF
    WRH $DA
    WRL $E5
    WRH $EF
    WRL $FA
    WRH $06
    WRL $11

    ; Block 1279
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1B
    WRL $26
    WRH $30
    WRL $3B
    WRH $44
    WRL $4D
    WRH $56
    WRL $5D
    WRH $65
    WRL $6B
    WRH $71
    WRL $75

    ; Block 1280
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7A
    WRH $77
    WRL $72
    WRH $6D
    WRL $66

    ; Block 1281
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $60
    WRL $58
    WRH $50
    WRL $47
    WRH $3D
    WRL $33
    WRH $29
    WRL $1E
    WRH $14
    WRL $09
    WRH $FD
    WRL $F2

    ; Block 1282
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E8
    WRL $DD
    WRH $D2
    WRL $C8
    WRH $BE
    WRL $B5
    WRH $AC
    WRL $A4
    WRH $9D
    WRL $96
    WRH $90
    WRL $8B

    ; Block 1283
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $87
    WRL $84
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $84
    WRH $87
    WRL $8B
    WRH $90
    WRL $96

    ; Block 1284
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9D
    WRL $A4
    WRH $AC
    WRL $B5
    WRH $BE
    WRL $C8
    WRH $D2
    WRL $DD
    WRH $E8
    WRL $F2
    WRH $FD
    WRL $09

    ; Block 1285
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $14
    WRL $1E
    WRH $29
    WRL $33
    WRH $3D
    WRL $47
    WRH $50
    WRL $58
    WRH $60
    WRL $66
    WRH $6D
    WRL $72

    ; Block 1286
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $77
    WRL $7A
    WRH $7D
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7C
    WRH $79
    WRL $75
    WRH $71
    WRL $6B

    ; Block 1287
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $65
    WRL $5D
    WRH $56
    WRL $4D
    WRH $44
    WRL $3B
    WRH $30
    WRL $26
    WRH $1B
    WRL $11
    WRH $06
    WRL $FA

    ; Block 1288
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EF
    WRL $E5
    WRH $DA
    WRL $CF
    WRH $C5
    WRL $BC
    WRH $B3
    WRL $AA
    WRH $A2
    WRL $9B
    WRH $95
    WRL $8F

    ; Block 1289
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8A
    WRL $86
    WRH $83
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $85
    WRL $88
    WRH $8D
    WRL $92

    ; Block 1290
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $98
    WRL $9F
    WRH $A6
    WRL $AF
    WRH $B8
    WRL $C1
    WRH $CB
    WRL $D5
    WRH $E0
    WRL $EA
    WRH $F5
    WRL $01

    ; Block 1291
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $0C
    WRL $16
    WRH $21
    WRL $2C
    WRH $36
    WRL $40
    WRH $49
    WRL $52
    WRH $5A
    WRL $61
    WRH $68
    WRL $6E

    ; Block 1292
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $73
    WRL $78
    WRH $7B
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7B
    WRL $78
    WRH $74
    WRL $6F

    ; Block 1293
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $69
    WRL $63
    WRH $5B
    WRL $53
    WRH $4B
    WRL $42
    WRH $38
    WRL $2E
    WRH $23
    WRL $18
    WRH $0E
    WRL $03

    ; Block 1294
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F7
    WRL $EC
    WRH $E2
    WRL $D7
    WRH $CD
    WRL $C3
    WRH $B9
    WRL $B0
    WRH $A8
    WRL $A0
    WRH $99
    WRL $93

    ; Block 1295
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8D
    WRL $89
    WRH $85
    WRL $82
    WRH $81
    WRL $80
    WRH $80
    WRL $81
    WRH $83
    WRL $85
    WRH $89
    WRL $8E

    ; Block 1296
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $93
    WRL $9A
    WRH $A1
    WRL $A9
    WRH $B1
    WRL $BA
    WRH $C4
    WRL $CE
    WRH $D8
    WRL $E3
    WRH $ED
    WRL $F8

    ; Block 1297
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $04
    WRL $0F
    WRH $19
    WRL $24
    WRH $2F
    WRL $39
    WRH $42
    WRL $4C
    WRH $54
    WRL $5C
    WRH $63
    WRL $6A

    ; Block 1298
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $70
    WRL $75
    WRH $79
    WRL $7C
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7D
    WRL $7B
    WRH $77
    WRL $73

    ; Block 1299
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6E
    WRL $68
    WRH $61
    WRL $59
    WRH $51
    WRL $48
    WRH $3F
    WRL $35
    WRH $2B
    WRL $20
    WRH $16
    WRL $0B

    ; Block 1300
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $00
    WRL $F4
    WRH $E9
    WRL $DF
    WRH $D4
    WRL $CA
    WRH $C0
    WRL $B7
    WRH $AE
    WRL $A6
    WRH $9E
    WRL $97

    ; Block 1301
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $91
    WRL $8C
    WRH $88
    WRL $84
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83
    WRH $86
    WRL $8A

    ; Block 1302
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8F
    WRL $95
    WRH $9C
    WRL $A3
    WRH $AB
    WRL $B3
    WRH $BD
    WRL $C6
    WRH $D0
    WRL $DB
    WRH $E6
    WRL $F0

    ; Block 1303
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $FB
    WRL $07
    WRH $12
    WRL $1C
    WRH $27
    WRL $31
    WRH $3B
    WRL $45
    WRH $4E
    WRL $56
    WRH $5E
    WRL $65

    ; Block 1304
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $6C
    WRL $71
    WRH $76
    WRL $7A
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7D
    WRH $7A
    WRL $76

    ; Block 1305
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $72
    WRL $6C
    WRH $66
    WRL $5F
    WRH $57
    WRL $4F
    WRH $46
    WRL $3C
    WRH $32
    WRL $28
    WRH $1D
    WRL $13

    ; Block 1306
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $08
    WRL $FC
    WRH $F1
    WRL $E7
    WRH $DC
    WRL $D1
    WRH $C7
    WRL $BD
    WRH $B4
    WRL $AC
    WRH $A4
    WRL $9C

    ; Block 1307
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $96
    WRL $90
    WRH $8B
    WRL $87
    WRH $84
    WRL $81
    WRH $80
    WRL $80
    WRH $80
    WRL $82
    WRH $84
    WRL $87

    ; Block 1308
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $8C
    WRL $91
    WRH $97
    WRL $9E
    WRH $A5
    WRL $AD
    WRH $B6
    WRL $BF
    WRH $C9
    WRL $D3
    WRH $DE
    WRL $E9

    ; Block 1309
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $F3
    WRL $FE
    WRH $0A
    WRL $15
    WRH $1F
    WRL $2A
    WRH $34
    WRL $3E
    WRH $47
    WRL $50
    WRH $59
    WRL $60

    ; Block 1310
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $67
    WRL $6D
    WRH $72
    WRL $77
    WRH $7A
    WRL $7D
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7E
    WRH $7C
    WRL $79

    ; Block 1311
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $75
    WRL $70
    WRH $6A
    WRL $64
    WRH $5D
    WRL $55
    WRH $4C
    WRL $43
    WRH $3A
    WRL $30
    WRH $25
    WRL $1A

    ; Block 1312
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $10
    WRL $05
    WRH $F9
    WRL $EE
    WRH $E4
    WRL $D9
    WRH $CF
    WRL $C5
    WRH $BB
    WRL $B2
    WRH $A9
    WRL $A2

    ; Block 1313
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9A
    WRL $94
    WRH $8E
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80
    WRH $82
    WRL $85

    ; Block 1314
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $88
    WRL $8D
    WRH $92
    WRL $99
    WRH $9F
    WRL $A7
    WRH $AF
    WRL $B8
    WRH $C2
    WRL $CC
    WRH $D6
    WRL $E1

    ; Block 1315
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $EB
    WRL $F6
    WRH $02
    WRL $0D
    WRH $17
    WRL $22
    WRH $2D
    WRL $37
    WRH $41
    WRL $4A
    WRH $53
    WRL $5B

    ; Block 1316
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $62
    WRL $69
    WRH $6F
    WRL $74
    WRH $78
    WRL $7B
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7F
    WRH $7E
    WRL $7B

    ; Block 1317
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $78
    WRL $74
    WRH $6F
    WRL $69
    WRH $62
    WRL $5B
    WRH $53
    WRL $4A
    WRH $41
    WRL $37
    WRH $2D
    WRL $22

    ; Block 1318
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $17
    WRL $0D
    WRH $02
    WRL $F6
    WRH $EB
    WRL $E1
    WRH $D6
    WRL $CC
    WRH $C2
    WRL $B8
    WRH $AF
    WRL $A7

    ; Block 1319
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $9F
    WRL $99
    WRH $92
    WRL $8D
    WRH $88
    WRL $85
    WRH $82
    WRL $80
    WRH $80
    WRL $80
    WRH $81
    WRL $83

    ; Block 1320
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $86
    WRL $8A
    WRH $8E
    WRL $94
    WRH $9A
    WRL $A2
    WRH $A9
    WRL $B2
    WRH $BB
    WRL $C5
    WRH $CF
    WRL $D9

    ; Block 1321
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $E4
    WRL $EE
    WRH $F9
    WRL $05
    WRH $10
    WRL $1A
    WRH $25
    WRL $30
    WRH $3A
    WRL $43
    WRH $4C
    WRL $55

    ; Block 1322
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $5D
    WRL $64
    WRH $6A
    WRL $70
    WRH $75
    WRL $79
    WRH $7C
    WRL $7E
    WRH $7F
    WRL $7F
    WRH $7F
    WRL $7D

    ; Block 1323
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7A
    WRL $77
    WRH $72
    WRL $6D
    WRH $67
    WRL $60
    WRH $59
    WRL $50
    WRH $47
    WRL $3E
    WRH $34
    WRL $2A

    ; Block 1324
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $1F
    WRL $15
    WRH $0A
    WRL $FE
    WRH $F3
    WRL $E9
    WRH $DE
    WRL $D3
    WRH $C9
    WRL $BF
    WRH $B6
    WRL $AD

    ; Block 1325
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $A5
    WRL $9E
    WRH $97
    WRL $91
    WRH $8C
    WRL $87
    WRH $84
    WRL $82
    WRH $80
    WRL $80
    WRH $80
    WRL $81

    ; Block 1326
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $84
    WRL $87
    WRH $8B
    WRL $90
    WRH $96
    WRL $9C
    WRH $A4
    WRL $AC
    WRH $B4
    WRL $BD
    WRH $C7
    WRL $D1

    ; Block 1327
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $DC
    WRL $E7
    WRH $F1
    WRL $FC
    WRH $08
    WRL $13
    WRH $1D
    WRL $28
    WRH $32
    WRL $3C
    WRH $46
    WRL $4F

    ; Block 1328
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $57
    WRL $5F
    WRH $66
    WRL $6C
    WRH $72
    WRL $76
    WRH $7A
    WRL $7D
    WRH $7E
    WRL $7F
    WRH $7F
    WRL $7E

    ; Block 1329
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $7C
    WRL $7A
    WRH $76
    WRL $71
    WRH $6C
    WRL $65
    WRH $5E
    WRL $56
    WRH $4E
    WRL $45
    WRH $3B
    WRL $31

    ; Block 1330
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $27
    WRL $1C
    WRH $12
    WRL $07
    WRH $FB
    WRL $F0
    WRH $E6
    WRL $DB
    WRH $D0
    WRL $C6
    WRH $BD
    WRL $B3

    ; Block 1331
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $AB
    WRL $A3
    WRH $9C
    WRL $95
    WRH $8F
    WRL $8A
    WRH $86
    WRL $83
    WRH $81
    WRL $80
    WRH $80
    WRL $80

    ; Block 1332
    ; Header: loops=$FF, Y/L=$00, V/U=$00, T/S=$00
    WRH $FF
    WRL $00
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $82
    WRL $84
    WRH $88
    WRL $8C
    WRH $91
    WRL $97
    WRH $9E
    WRL $A6
    WRH $AE
    WRL $B7
    WRH $C0
    WRL $CA

    ; Block 1333
    ; Header: loops=$FF, Y/L=$04, V/U=$00, T/S=$00
    WRH $FF
    WRL $04
    WRH $00
    WRL $00
    ; Samples (12 bytes)
    WRH $D4
    WRL $DF
    WRH $E9
    WRL $F4
    WRH $00
    WRL $00
    WRH $00
    WRL $00
    WRH $00
    WRL $00
    WRH $00
    WRL $00


    ; Write STL entry to $7000
    SDP $70
    SDB $00

    ; Sample data address = $9000
    WRH $90
    WRL $00
    ; Loop address = 0 (loop entire sample)
    WRH $00
    WRL $00


    ; Configure MMP channel 0 (left ear)
    SDP $00              ; MMP registers
    SDB $00
    WRH $10              ; Pitch = 0x1000 (1.0× speed)
    WRL $00

    ; Set volume to 128 (1.5× gain)
    SDB $20
    SBF 0
    SCR X, 128
    STA X, $20

    ; Set STL address to $7000 (starts playback!)
    SDB $54
    WRH $70
    WRL $00

    ; Set RP for loop address
    SRP $80

    ; Infinite loop
loop:
    NOP 1000
    BEQ X, X, loop

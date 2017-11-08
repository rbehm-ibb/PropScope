{{
    Copyright 2015, Reinhardt Behm rbehm@hushmail.com
    I2C interface in PropScope
}}

OBJ
    i2c     : "i2c32"
    time    : "timer"

CON
    EEPROM  = $a0    'address of system eeprom
    I2C_D   = 29
    I2C_C   = 28

' addr EEPROM on ext card
CON
    EEP_CARD = $a2

' PCA9538
CON
    PCA_ADDR = $e0
    'regs
    PCA_IN  = 0
    PCA_OUT = 1
    PCA_INV = 2
    PCA_CONF = 3   
    'bits
    DC_0    = |< 0    'channel 0 DC/~AC
    DC_1    = |< 1    'channel 1 DC/~AC
    DIV2_0  = |< 2    'channel 0 div2/~div10
    DIV2_1  = |< 3    'channel 1 div2/~div10
    REF1_0  = |< 4    'channel 0 ref 1V/~2V
    REF1_1  = |< 5    'channel 1 ref 1V/~2V
    VOLT2   = |< 6    '-2V on
    CARD    = |< 7    'expansion card active

CON
    versionsize = 17
    NCALIB = 23
    versionSizeFull = NCALIB*4 + 4*2 + versionsize*2

VAR
    byte pca, lastPca

VAR 
    long calib[NCALIB]
    word hwversion, testversion
    word cardhwversion, cardtestversion
    byte version[versionsize]
    byte cardversion[versionsize]

PUB start | ok
    i2c.init(I2C_D, I2C_C, true)
    i2c.Start
    ok := i2c.devicePresent(PCA_ADDR)
    pca := VOLT2
    lastPca := $ff
    writePcaReg(PCA_OUT, pca)
    writePcaReg(PCA_INV, 0)  'polarity inversion
    writePcaReg(PCA_CONF, 0)
    pca := VOLT2
    ok := i2c.devicePresent(EEP_CARD)
    if ok
        pca |= CARD
    else
        pca &= !CARD
        resCard
    writePca
'] start

PUB getPca
    return pca
'] getPca

PRI setPcaBit(on, mask)
    if on
        pca |= mask
    else
        pca &= !mask
    '] if
    writePca
'] setPcaBit

PUB set0DcAc(ac)
    setPcaBit(not ac, DC_0)
'] set0DcAc

PUB set1DcAc(ac)
    setPcaBit(not ac, DC_1)
'] set1DcAdc

PUB set0div(v2)
    setPcaBit(v2, DIV2_0)
'] set0div

PUB set1div(v2)
    setPcaBit(v2, DIV2_1)
'] set1div

PUB set0range(r1)
    setPcaBit(r1, REF1_0)
'] set0range

PUB set1range(r1)
    setPcaBit(r1, REF1_1)
'] set1range

PRI set2V
    pca |= VOLT2
    writePca
'] set2V

PRI res2V
    pca &= !VOLT2
    writePca
'] res2V

PRI setCard
    pca |= CARD
    writePca
']  setCard

PRI resCard
    pca &= !CARD
    writePca
'] resCard

PRI writePcaReg(reg, data) | ok
    ok := i2c.write(PCA_ADDR, reg, data, 8)
'] writPcaReg

PRI writePca | ok
    pca |= VOLT2
    ok := i2c.write(PCA_ADDR, PCA_OUT, pca, 8)
    if (lastPca ^ pca) & $0f
        time.pauseMSec(100)
    '] if
    lastPca := pca
'] writePca

PUB getVersion | i, d
    bytefill(@calib, 0, versionSizeFull)
    repeat i from 0 to 15
        version[i] := i2c.read(EEPROM, $ff7f + i, 16, 8)
    '] repeat
    hwversion := i2c.read(EEPROM, $ff8f, 16, 16)
    testversion := i2c.read(EEPROM, $ff91, 16, 16)
    d := i2c.devicePresent(EEP_CARD)
    if d
        repeat i from 0 to 15
            cardversion[i] := i2c.read(EEP_CARD, i, 8, 8)
        '] repeat
        cardhwversion := cardversion[14]
        cardtestversion := cardversion[15]
        cardversion[14] := 0
    '] if
    repeat i from 0 to NCALIB-1
        calib[i] := i2c.read(EEPROM, $ff93 + i*4, 16, 32)
    '] repeat
    return @calib
'] getVersion

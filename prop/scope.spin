{{
    Copyright 2015, Reinhardt Behm rbehm@hushmail.com
}}

OBJ
    i2c     : "i2c-scope"
    ipc     : "ipccomm-pasm"
    timer   : "timer"

CON
    P_MUX = 27
    P_CLK = 26

VAR 
    long cog

CON
    BUFSIZE = 4096

VAR
    long addrVersion
    long depth
    long seq  
    long trigPos
    long trigEnd
    long buffer[BUFSIZE]

CON 'trigger mode
    #0, TrigAuto, TrigNorm, TrigSingle
CON 'trigger edge
    #0, TrigRising, TrigFalling

VAR
    byte pending
CON
    PEND_ERR    = |<0
    PEND_STOP   = |<1
    PEND_START  = |<2
    PEND_VER    = |<3
    PEND_PING   = |<4
    
PUB start(aversion) | ok
    addrVersion := aversion
    stop
    timer.startTimer(timer#PING, timer#TICK_MS*1000)
    ipc.start
    ok := i2c.start
    pending~

PRI startCog
    cog := cognew(@runScopeAsm, 0) + 1
    pending |= PEND_START

PUB stop
    if cog                                 'Did we start a cog?
        cogstop(cog~ - 1)                    '  If so, stop it
    pending |= PEND_STOP

PRI version | addr
    pending &= ! PEND_VER
    addr := i2c.getVersion
    ipc.startTx("v")
    ipc.addBytes(addr, i2c#versionSizeFull)
    ipc.addBytes(addrVersion, strsize(addrVersion)+1)
    ipc.send

{
	enum Mode { TrigOff, TrigAuto, TrigNorm, TrigSingle };
	enum Edge { Rising, Falling };
	ds << quint8('G')                          0
	   << quint8(m_config.ch(0).res())         1
	   << quint8(m_config.ch(0).dc())          2
	   << quint8(m_config.ch(1).res())         3
	   << quint8(m_config.ch(1).dc())          4
	   << quint32(frqa)                        5
	   << quint32(m_config.time().depth())     9
	      ;
	ds << quint8(m_config.trigger().channel()) 13
	   << quint8(m_config.trigger().mode())    14
	   << quint8(m_config.trigger().edge())    15
	   << quint16(level)                       16
	   << quint16(m_config.trigger().pos())    18
}
    
PUB go | reso, dc0, dc1, r0, r1, d0, d1
    stop
    
    'ch 0
    reso := ipc.getByte
    dc0 := ipc.getByte
    r0 := (reso & 1) <> 0
    d0 := (reso & 2) <> 0
    'ch 1
    reso := ipc.getByte
    dc1 := ipc.getByte
    r1 := (reso & 1) <> 0
    d1 := (reso & 2) <> 0

    adcFreq := ipc.getLong
    depth := ipc.getWord
    if depth > BUFSIZE
        depth := BUFSIZE
    if depth > ipc#MAX_TX_BUF/4 - 3*4
        depth := ipc#MAX_TX_BUF/4 - 3*4

    trigShift := 0
    if ipc.getByte <> 0
        trigShift := 10
    trigMode := ipc.getByte

    trigEdge := ipc.getByte
    trigLevel := ipc.getWord
    trigPost := ipc.getWord
    if trigPost =< 1
        trigPost := 1
    
    i2c.set0DcAc(dc0)
    i2c.set0div(d0)
    i2c.set0Range(r0)

    i2c.set1DcAc(dc1)
    i2c.set1div(d1)
    i2c.set1Range(r1)

    addrseq := @seq
    addrBuf := @buffer
    addrTpos := @trigPos
    addrTend := @trigEnd
    trigPre := depth - trigPost
    if (trigPre < 10)
        trigPre := 10
'    trigPre := BUFSIZE
    longfill(@buffer, $10000002, BUFSIZE)
    startCog
    
PUB poll | cmd, i
    cmd := ipc.getCmd
    if (cmd >= 0)
        case ipc.getCmd
            "G":
                go
            "S":
                stop
            "V":
                pending |= PEND_VER
            other:
                pending |= PEND_ERR
        ipc.rxDone
    if timer.timeout(timer#PING)
        pending |= PEND_PING
        timer.startTimer(timer#PING, timer#TICK_S)

    if ipc.txRdy
        if pending & PEND_ERR
            pending &= ! PEND_ERR
            ipc.startTx("?")
            ipc.send
        elseif pending & PEND_VER
            version
        elseif pending & PEND_STOP
            pending &= ! PEND_STOP
            ipc.startTx("s")
            ipc.send
        elseif pending & PEND_START
            pending &= ! PEND_START
            ipc.startTx("g")
            ipc.send
        elseif seq and cog
            ipc.startTx("d")
            i := trigEnd - depth
            ipc.addByte(i2c.getPca)
            ipc.addWord(trigPost)
            ipc.addWord(depth)
            repeat depth
                i &= BUFSIZE - 1
                ipc.addLong(buffer[i++])
            ipc.send
            longfill(@buffer, 0*1024+100, BUFSIZE)
            seq := 0
        elseif pending & PEND_PING
            pending &= ! PEND_PING
            ipc.startTx("p")
            ipc.addByte(i2c.getPca)
            ipc.addLong(depth)
            ipc.send
 
DAT

'***********************************
'* Assembly language  driver       *
'***********************************

                org     0
runScopeAsm
                or      dira,clkMask
                or      dira,muxMask
                or      outa,muxMask
                mov     ctra,ctraInit
                mov     frqa,adcFreq
                mov     t1,#0
                wrlong  t1,addrSeq
                
                'setup trigger
{
                cmp     trigMode,#TrigOff
    if_z        mov     tproc0,#tGoWcnt
    if_z        jmp     #trigSetDone
}
                cmp     trigEdge,#TrigRising wz
    if_z        jmp     #:rising
                cmp     trigMode,#TrigAuto wz
    if_z        mov     tproc0,#tpFe0   'auto
    if_nz       mov     tproc0,#tpFeN0  'norm
                jmp     #trigSetDone

:rising         cmp     trigMode,#TrigAuto wz
    if_z        mov     tproc0,#tpRe0   'auto
    if_nz       mov     tproc0,#tpReN0  'norm

trigSetDone                

forever
                mov     tcount,trigPre
                mov     tCountX,maxCountX
                mov     tEnd,#0
                mov     tproc,tproc0
                mov     bufIdx,#0
                waitpne null,clkMask
                waitpeq null,clkMask
                waitpne null,clkMask
oneCycle
                'leading edge
                waitpeq null,clkMask
                mov     adcval,ina
                and     adcval,adcMask
                
                'store data
                mov     t1,addrBuf
                add     t1,bufIdx
                wrlong  adcval,t1
                add     bufIdx,#4
                and     bufIdx,idxMask
                
                'trailing edge
                waitpne null,clkMask
                'trigger
                shr     adcval,trigShift
                and     adcval,adcmask1     'this is the relevant channel

                jmp     tproc
trigRdy
                shr     tPos,#2
                wrlong  tPos,addrTpos
                shr     tEnd,#2
                wrlong  tEnd,addrTend
                mov     t1,#1
                wrlong  t1,addrSeq
waitSeq         rdlong  t1,addrSeq  wz
    if_nz       jmp     #waitSeq
                jmp     #forever

'*************** trigger procs
'--- rising egde, norm

tpReN0           'wait pre trigger
                djnz    tcount,#oneCycle
                mov     tproc,#tpReN1
                mov     tEnd,bufIdx
tpReN1           'wait low
                cmp     adcval,trigLevel    wz,wc
    if_c        mov     tproc,#tpReN2   'was low
                jmp     #oneCycle
tpReN2           'wait high
                cmp     adcval,trigLevel    wz,wc
    if_c        jmp     #oneCycle
                mov     tpos,bufIdx
                mov     tCount,trigPost
                mov     tproc,#tpReN3
                jmp     #oneCycle
tpReN3           'wait post trigger                
                djnz    tcount,#oneCycle
                mov     tEnd,bufIdx
                jmp     #trigRdy

'--- falling egde, norm

tpFeN0           'wait pre trigger
                djnz    tcount,#oneCycle
                mov     tproc,#tpFeN1
                mov     tEnd,bufIdx
tpFeN1           'wait low
                cmp     adcval,trigLevel    wz,wc
    if_nc       mov     tproc,#tpFeN2
                jmp     #oneCycle
tpFeN2           'wait high
                cmp     adcval,trigLevel    wz,wc
    if_nc        jmp     #oneCycle
                mov     tpos,bufIdx
                mov     tCount,trigPost
                mov     tproc,#tpFeN3
                jmp     #oneCycle
tpFeN3           'wait post trigger                
                djnz    tcount,#oneCycle
                mov     tEnd,bufIdx
                jmp     #trigRdy

'--- rising egde, auto

tpRe0           'wait pre trigger
                djnz    tcount,#oneCycle
                mov     tproc,#tpRe1
                mov     tEnd,bufIdx
tpRe1           'wait low
                cmp     adcval,trigLevel    wz,wc
    if_b        mov     tproc,#tpRe2
                djnz    tCountX,#oneCycle
                jmp     #trigRdy
tpRe2           'wait high
                djnz    tCountX,#:noTo
                jmp     #trigRdy
:noTo
                cmp     adcval,trigLevel    wz,wc
    if_b        jmp     #oneCycle
                mov     tpos,bufIdx
                mov     tCount,trigPost
                mov     tproc,#tpRe3
                jmp     #oneCycle
tpRe3           'wait post trigger                
                djnz    tcount,#oneCycle
                mov     tEnd,bufIdx
                jmp     #trigRdy

'--- falling egde, auto

tpFe0           'wait pre trigger
                djnz    tcount,#oneCycle
                mov     tproc,#tpFe1
                mov     tEnd,bufIdx
tpFe1           'wait low
                cmp     adcval,trigLevel    wz,wc
    if_a        mov     tproc,#tpFe2
                djnz    tCountX,#oneCycle
                jmp     #trigRdy
tpFe2           'wait high
                djnz    tCountX,#:noTo
                jmp     #trigRdy
:noTo
                cmp     adcval,trigLevel    wz,wc
    if_a        jmp     #oneCycle
                mov     tpos,bufIdx
                mov     tCount,trigPost
                mov     tproc,#tpFe3
                jmp     #oneCycle
tpFe3           'wait post trigger                
                djnz    tcount,#oneCycle
                mov     tEnd,bufIdx
                jmp     #trigRdy

'--------------- vars initialized
clkMask         long    |< P_CLK
muxMask         long    |< P_MUX
ctraInit        long    $10_00_00_1A
null            long    0
adcMask         long    (|<20)-1    '20 bits
adcMask1        long    (|<10)-1    '10bits
idxMask         long    (BUFSIZE*4)-1   'assumes BUFSIZE is power of 2
maxCountX       long    BUFSIZE*10
adcFreq         long    0
addrSeq         long    0
addrBuf         long    0

trigShift       long    0       'will select channel
trigLevel       long    0
trigPre         long    0
trigPost        long    0
trigEdge        long    0       'enum Edge { Rising, Falling }
trigMode        long    0
addrTpos        long    0-0
addrTend        long    0-0
'--------------- vars not initialized
bufIdx          res     1
'--- trigger
tProc0          res     1
tProc           res     1
tCount          res     1
tCountX         res     1
tPos            res     1
tEnd            res     1

adcval          res     1
t1              res     1
t2              res     1


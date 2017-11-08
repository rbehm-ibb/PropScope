CON
    BaudRate      = 230400  '115200
    RxPin         = |< 31         'For RS232
    TxPin         = |< 30         'For RS232

OBJ
    uart        : "JDCogSerial"
    timer       : "timer"

CON
    STX = "S"   '$01
    ETX = "E"   '$03
    DLE = "D"   '$10
    ACK = "A"   '$06
    NAK = "N"   '$15
    WACK = 1    'internal only
    MAX_TX_BUF = 2*1024*4+20
    MAX_RX_BUF = 100
    MAX_REPEAT = 5+1

DAT
_rxLen      LONG 0
_txLen      LONG 0
cog         long 0
rxBuffer    BYTE 0[MAX_RX_BUF]
txBuffer    BYTE 0[MAX_TX_BUF]
_txIdx      LONG 0
_rxPtr      LONG 0

PUB start | addr
    addr := uart.start(RxPin, TxPin, BaudRate)
    if addr == 0
       return

    aRxBuf := addr
    aTxBuf := addr + 4
    _rxLen := 0
    _txLen := 0
    aRxLen := @_rxLen
    aTxLen := @_txLen
    aRxBuffer := @rxBuffer
    aTxBuffer := @txBuffer
    TO_RX_CHAR := CLKFREQ / 1000 * 10   ' ms
    TO_TX_RESP := CLKFREQ / 1000 * 100   ' ms
    cog := cognew(@ipccomm, 0) + 1

{
    tests if something rxd, returns true if yes
}
PUB hasRx
    return _rxLen <> 0
'] hasRx

{
    get received data, copies it to buffer
    buffer must be large enough
    return length of rxd data or 0 if none
}
PUB get(buffer) : len
    if _rxLen == 0
        return 0
    ']if
    len := _rxLen
    bytemove(buffer, @rxbuffer, len)
    _rxLen := 0
    return len
'] get

{
    high level rx functions
}
PUB getCmd
    if _rxLen <> 0
        _rxPtr := @rxbuffer[1]
        return rxbuffer[0]
    return -1

PUB getByte
    return byte[_rxPtr++]

PUB getWord : v
    bytemove(@v, _rxPtr, 2)
    _rxPtr += 2
    return v
    
PUB getLong : v
    bytemove(@v, _rxPtr, 4)
    _rxPtr += 4
    return v

PUB rxDone
    _rxLen := 0

{
    can accept new data to send
}
PUB txRdy
    return _txLen == 0
'] txRdy

{{
    "high level" tx functions.
    startTx, appendValue, appendBuffer, tx should be called in one batch
    These build up the tx message in txBuffer and finally send it
}}
{
    starts a transmission sequence, resets txLen, appends cmd
    return false if buffer still busy
}
PUB startTx(cmd)
    if _txLen > 0
        return false
    _txIdx := 1
    txBuffer[0] := cmd
    return true

PUB addByte(d)
    txBuffer[_txIdx] := d
    ++_txIdx

PUB addWord(d)   
    bytemove(@txBuffer[_txIdx], @d, 2)
    _txIdx += 2


PUB addLong(d)   
    bytemove(@txBuffer[_txIdx], @d, 4)
    _txIdx += 4

PUB addBytes(src, nbytes)
    bytemove(@txBuffer[_txIdx], src, nbytes)
    _txIdx += nbytes

PUB put(src, nbytes)
    bytemove(@txBuffer[_txIdx], src, nbytes)
    _txIdx += nbytes

PUB send
    _txLen := _txIdx
    _txIdx := 0


{
--------------------
---- IpcComm in PASM
--------------------
}
DAT org

ipccomm
    'parameters
    ' none,all set up by callee
            mov     rxStAddr,#pRxIdle
            mov     rxResp,#0
            mov     rxSum,#0
            mov     txStAddr,#pTxIdle
            mov     txResp,#0
            mov     txRepeat,#0

loop
            jmp     rxStAddr    '-- rx state machine
rxReturn    'all rx procs will "return" here
            jmp     txStAddr    '-- tx state machine
txReturn    'all tx procs will "return" here
            jmp     #loop

'-------------------------------- TX
'-- tx procs

uartRdy     'check if tx is ready
            rdlong  t1,aTxBuf
            cmp     t1,cMinus1 WZ
uartRdy_ret ret

pTxIdle     'idle state, check if any response to send
            'check if any msg to send -> start msg
            call    #uartRdy
    if_nz   jmp     #txReturn       'not ready

            cmp     txResp,#0 WZ
    if_nz   jmp     #sendResp

            rdlong  txLen,aTxLen WZ    'anything to tx
    if_z    jmp     #txReturn       'no
            mov     txIdx,#0        'init sending
            mov     rxResp,#0
            mov     txStAddr,#pTxData
            mov     txSum,#STX
            wrlong  txSum,aTxBuf    'give it to sender
            jmp     #txReturn

sendRespW   'if tx is ready send a resp in txResp and reset 
            call    #uartRdy
    if_nz   jmp     #txReturn       'not ready
sendResp    'send a resp in txResp and reset
            wrlong  txResp,aTxBuf   'send it
            mov     txResp,#0      'and clear it
            jmp     #txReturn

pTxData     call    #uartRdy
    if_nz   jmp     #txReturn       'not ready

            cmp     txIdx,txLen WZ    'at end?
    if_z    jmp     #:txEnd
            mov     t1,txIdx
            add     txIdx,#1
            add     t1,aTxBuffer
            rdbyte  char,t1         'next byte to send
            wrlong  char,aTxBuf     'send it
            add     txSum,char
            cmp     char,#DLE WZ
    if_z    mov     txStAddr,#pTxDle
            jmp     #txReturn
:txEnd      mov     char,#DLE
            wrlong  char,aTxBuf     'send DLE
            mov     txStAddr,#pTxEtx
            jmp     #txReturn

pTxDle      call    #uartRdy
    if_nz   jmp     #txReturn       'not ready

            mov     char,#DLE
            wrlong  char,aTxBuf     'send DLE
            mov     txStAddr,#pTxData
            jmp     #txReturn

pTxEtx      call    #uartRdy
    if_nz   jmp     #txReturn       'not ready

            mov     char,#ETX
            wrlong  char,aTxBuf     'send ETX
            mov     txStAddr,#pTxCrc
            add     txSum,char
            mov     rxResp,#0
            jmp     #txReturn

pTxCrc      call    #uartRdy
    if_nz   jmp     #txReturn       'not ready

            wrlong  txSum,aTxBuf    'send chksum
            mov     txStAddr,#pTxWait
            mov     txTime0,cnt
            jmp     #txReturn

pTxWait
            cmp     txResp,#0 WZ
    if_nz   jmp     #sendRespW
            cmp     rxResp,#ACK WZ
    if_z    jmp     #:hadAck
            cmp     rxResp,#WACK WZ
    if_z    jmp     #:hadWack
            cmp     rxResp,#NAK WZ
    if_z    jmp     #:hadNak
            'timeout?
            mov     t1,cnt
            sub     t1,txTime0  'elapsed time
            cmp     t1,TO_TX_RESP WC
    if_nc   jmp     #:hadNak     'timeout, same as NAK
            jmp     #txReturn

:hadAck     wrlong  null,aTxLen     'done reset
            mov     txStAddr,#pTxIdle
            mov     txRepeat,#0     'reset
            jmp     #txReturn

:hadWack    mov     txTime0,cnt     'restart timer
            mov     rxResp,#0
            jmp     #txReturn

:hadNak     add     txRepeat,#1
            cmp     txRepeat,#MAX_REPEAT WC
    if_nc   jmp     #:hadAck        'too many repeats,abort,same as ACK
            mov     txStAddr,#pTxIdle 'will repeat
            jmp     #txReturn

'------------------- RX
'
'-- some helper functions
'
getUart     'any char available?        ret with NZ set if anything
            rdlong  char,aRxBuf         'Check the receiver
            cmp     char,cMinus1  WZ
    if_nz   wrlong  cMinus1,aRxBuf      'clear receiver, if anything rxd
'    if_nz   wrlong  char,aTxBuf    'give it to sender
'    if_nz   wrlong  char,aTxState
getUart_ret ret

chkRxTime   'check if we had a timeout (>= TO_RX_CHAR since rxTime0 was set to cnt)
            'will reset any rx
            'and jump to rxReturn
            'return normal if ok
            mov     t1,cnt
            sub     t1,rxTime0
            cmp     t1,TO_RX_CHAR WC
    if_c    jmp     #chkRxTime_ret
            mov     rxStAddr,#pRxIdle   'reset
            mov     txResp,#NAK         'and send a NAK
            jmp     #rxReturn           'unclean, breaks out of call
chkRxTime_ret ret


'-- rx procs
pRxIdle     'accept responses (ACK/NAK) and store them
            'accept STX and start a new rx msg
            'anything elsye is ignored
            call    #getUart
    if_z    jmp     #rxReturn           'nothing rxd
            cmp     char,#ACK   WZ
    if_z    jmp     #:isResp
            cmp     char,#NAK   WZ
    if_nz   jmp     #:noResp
:isResp     mov     rxResp,char
            jmp     #rxReturn
:noResp     cmp     char,#STX   WZ
    if_nz   jmp     #rxReturn           'nothing interesting
            'we have rxd STX
            mov     rxIdx,#0
            mov     rxSum,#STX
            mov     rxStAddr,#pRxData
            mov     rxTime0,cnt
            jmp     #rxReturn

pRxData     'accept any data, handle DLE (DLE-DLE,DLE-ETX)
            'keep tx happy by simulating WACK
            'if msg too long abort and send a NAK
            call    #chkRxTime
            call    #getUart
    if_z    jmp     #rxReturn           'nothing rxd
            mov     rxResp,#WACK
            mov     rxTime0,cnt
            cmp     char,#DLE   WZ
    if_z    jmp     #isDle

addRx       cmp     rxIdx,cMaxRx WC
    if_nc   jmp     #:overflow
            'normal data byte, no OV
            mov     t1,aRxBuffer
            add     t1,rxIdx
            wrbyte  char,t1
            add     rxIdx,#1
            add     rxSum,char
            jmp     #rxReturn

:overflow   mov     txResp,#NAK
            mov     rxStAddr,#pRxIdle   'reset
            jmp     #rxReturn

isDle       mov     rxStAddr,#pRxDle
            jmp     #rxreturn

pRxDle      'handle second part of DLE-DLE or DLE-ETX
            call    #chkRxTime
            call    #getUart
    if_z    jmp     #rxReturn           'nothing rxd
            mov     rxResp,#WACK
            mov     rxTime0,cnt
            cmp     char,#ETX   WZ
            mov     rxStAddr,#pRxData   'assume not ETX, reset to normal data
    if_nz   jmp     #addRx              'then just store it, should be DLE (from DLE-DLE)
            mov     rxStAddr,#pRxCrc    'was DLE-ETX so expect crc
            add     rxSum,#ETX
            and     rxSum,#$0ff
            jmp     #rxReturn

pRxCrc      'wait for final crc
            'check, if ok send ACK, set global rxLen and enter RxBusy
            'if bad crc send NAK and enter RxIdle
            call    #chkRxTime
            call    #getUart
    if_z    jmp     #rxReturn           'nothing rxd
            mov     rxResp,#WACK
            cmp     char,rxSum  WZ
    if_z    jmp     #:crcOk
            mov     txResp,#NAK         'bad crc
            mov     rxStAddr,#pRxIdle
            jmp     #rxReturn

:crcOk      mov     txResp,#ACK
            wrlong  rxIdx,aRxLen
            mov     rxStAddr,#pRxBusy
            jmp     #rxReturn

pRxBusy     'if rxd msg process -> RxIdle
            'store rxd ACK/NAK -> rxResp
            'if STX -> rxResp=NAK
            'ignore any other
            rdlong  t1,aRxLen   WZ              'check if rxd msg has been processed?
     if_z   mov     rxStAddr,#pRxIdle             'reset state
     if_z   jmp     #rxReturn
            'store any ACK/NAK into rxResp, answer STX wih NAK
            call    #getUart
    if_z    jmp     #rxReturn   'no, done
            '---------
            cmp     char,#ACK   WZ
    if_z    jmp     #:isResp
            cmp     char,#NAK   WZ
    if_z    jmp     #:isResp
            cmp     char,#STX   WZ
    if_z    mov     txResp,NAK
            jmp     #rxReturn
:isResp     mov     rxResp,char
            jmp     #rxReturn

'--- init vars, init is done by SPIN callee

'-- interface to JDCogSerial
aRxBuf      long    0
aTxBuf      long    0

'-- interface for SPIN
aRxLen      long    0
aTxLen      long    0
aRxBuffer   long    0
cMaxRx      long    MAX_RX_BUF
aTxBuffer   long    0
TO_RX_CHAR  long    0
TO_TX_RESP  long    0

'-- internal vars
cMinus1     long    -1  'for JDCogSerial as "empty" value
null        long    0

'--- uninit vars
t1          res     1
char        res     1
'-- RX
rxStAddr    res     1
rxIdx       res     1
rxSum       res     1
rxResp      res     1
rxTime0     res     1
'-- TX
txStAddr    res     1
txLen       res     1
txIdx       res     1
txSum       res     1
txResp      res     1
txRepeat    res     1
txTime0     res     1

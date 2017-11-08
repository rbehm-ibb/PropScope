{{
 ******************************************************
 * copyright (C) 2015 by Becker Electronics Taiwan Ltd.
 * All Rights reserved
 * created 2015/06/17 by behm
 ******************************************************

timer.spin

It also manages some monoflop timers, counting down at 1ms.
The real counting is done in module buttons. This module only holds the data
with access functions.
The data is held in the DAT section to have only one instance.

This module can be used by many other modules. 
Care should be taken, not to use the same timer in different modules.
Semaphore locking is not needed then.
}}

CON
    'timer idx to be used outside
    #0, 
    IPC_TX,     'timer for ipccomm tx response timeout
    IPC_RX,     'timer for ipccomm rx char timeout
    PING,
    NTIMER      'always have this as last

CON
    TICK_MS = 10
    TICK_S = TICK_MS * 1000

CON 'taken from Clock.spin and merged here
    WMin  = 381                                                                                     'WAITCNT-expression-overhead Minimum

DAT 'used in spin and asm
    tickCount long 0
    timers long 0[NTIMER]

DAT ' used in spin code
    period long 0[NTIMER]
    active byte 0[NTIMER]

DAT
    stack long 0[32]

{
    Start this module. 
    Only returns the address of the vars in DAT for use by module button.
}
PUB start
    bytefill(@active, false, NTIMER)
    longfill(@timers, 0, NTIMER)   'make each timer fire once
    longfill(@period, 10, NTIMER)   'give it just some meaningful value
    delay := clkfreq / TICK_S
    addrTick := @tickCount
    cognew(@timerTick, 0)

{
    return the number of 1ms ticks since started
}
PUB tick
    return tickCount

PUB timerAddr(n)
    if (n => 0 and n < NTIMER)
        return @timers[n]
    return 0

{
    start timer n to count timev ms
}
PUB startTimer(n, timev)
    if (n => 0 and n < NTIMER)
        timers[n] := timev
        active[n] := true

{
    set period of timer n to time
    used by testAndSet
}
PUB setPeriod(n, time)
    if (n => 0 and n < NTIMER)
        period[n] := time

{
    stop timer n
}
PUB stopTimer(n)
    if (n => 0 and n < NTIMER)
        timers[n] := 0
        active[n] := false

{
    return the remaining time of timer n
}        
PUB timer(n)
    if (n => 0 and n < NTIMER)
        return timers[n]
    return -1

{
    Test if timer n has run out. This state will be erased.
}        
PUB timeOut(n)
    if (n => 0 and n < NTIMER)
        if (active[n] and (timers[n] == 0)) 'timed out?
            active[n] := false  'reset
            return true
    return false

{
    test if timer has expired and restart it
}
PUB testAndSet(n)
    if (n => 0 and n < NTIMER)
        if (active[n] and (timers[n] == 0)) 'timed out?
            timers[n] := period[n]  'don't reset but restart
            return true
        elseif(not active[n])
            timers[n] := period[n]      'was not running, start it
            active[n] := true
    return false

{
PRI runTimer | i, time, delay
    delay := clkfreq / 10000
    time := cnt
    repeat
        waitcnt(time += delay)
        ++tickCount
        repeat i from 0 to NTIMER-1
            if timers[i] <> 0
                --timers[i]
}         
{
    taken from Clock.spin and merged here
}
{{Pause execution in microseconds.
  PARAMETERS: Duration = number of microseconds to delay.
}}
PUB pauseUSec(Duration) 
  waitcnt(((clkfreq / 1_000_000 * Duration - 3928) #> WMin) + cnt)                                 
  

{{Pause execution in milliseconds.
  PARAMETERS: Duration = number of milliseconds to delay.
}}
PUB pauseMSec(Duration)
  waitcnt(((clkfreq / 1_000 * Duration - 3932) #> WMin) + cnt)                                     
  

{{Pause execution in seconds.
  PARAMETERS: Duration = number of seconds to delay.
}}
PUB pauseSec(Duration)
  waitcnt(((clkfreq * Duration - 3016) #> WMin) + cnt)                                             

'================================
'   ASM
'================================
 
DAT         org

timerTick
                    'parameters: none
'            mov     dira,mask
            mov     time,cnt
            add     time,delay
loop
            waitcnt time,delay
'            xor     outa,mask 
            rdlong  tmpTimer,addrTick
            add     tmpTimer,#1
            wrlong  tmpTimer,addrTick
            mov     timCounter,#NTIMER
            mov     timAddr,addrTick
:loop       add     timAddr,#4
            rdlong  tmpTimer,timAddr
            cmpsub  tmpTimer,#1   wc    'dec if <>0
    if_c    wrlong  tmpTimer,timAddr    'only write back if changed
            djnz    timCounter,#:loop
            jmp     #loop

'----------------------------------------------
'first 2 values are parameters
'these are set be callee in the dat section
'and copied to the cog during start
'----------------------------------------------

'--------------------- parameters data
addrTick    long    0   'addr of tickCounter, timers follow directly
delay       long    0   'calculated by callee o give 1ms
'mask        long    |<14
'--------------------- work data
t1          res     1
time        res     1   'for delay
    'regs for doTick
timAddr     res     1
tmpTimer    res     1
timCounter  res     1

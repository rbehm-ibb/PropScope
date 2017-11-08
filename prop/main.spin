{{
    Copyright 2016, Reinhardt Behm rbehm@hushmail.com
}}

CON
    _clkmode      = xtal1 + pll16x
    _xinfreq      = 6_250_000

DAT
    VERSION byte "PScope V02.01",0

OBJ
    scope       : "scope"
    timer       : "timer"

PUB start
    timer.start
    scope.start(@VERSION)
    repeat
        scope.poll

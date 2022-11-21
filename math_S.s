|  int16_t mul_div_round(int16_t mult1,int16_t mult2,int16_t divisor)
|
|  returns (mult1| mult2 / divisor), rounded away from zero
|
|  this is actually calculated as:
|      ((mult1| mult2| 2 / divisor) +/- 1) / 2
|  where mult1 & divisor are signed 16-bit integers
|        mult2 is a signed 15-bit integer
|
|  this version of the code is derived (with some tweaks) from the
|  original imported AES code (function mul_div() in aes/gsx2.S)


        .globl  _mul_div_round
_mul_div_round:
        move.w  6(sp),d0        | d0 = mult2
        add.w   d0,d0           | d0 = mult2| 2
        muls.w  4(sp),d0        | d0 = mult1| mult2| 2
        divs.w  8(sp),d0        | d0 = (mult1| mult2| 2) / divisor
        jmi     mdrminus
        addq.w  #1,d0           | d0 = (mult1| mult2| 2) / divisor + 1
        asr.w   #1,d0           | d0 = ((mult1| mult2| 2) / divisor + 1) / 2
        rts
mdrminus:
        subq.w  #1,d0           | d0 = (mult1| mult2| 2) / divisor - 1
        asr.w   #1,d0           | d0 = ((mult1| mult2| 2) / divisor - 1) / 2
        rts

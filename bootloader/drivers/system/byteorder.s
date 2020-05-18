    .syntax unified
    .cpu cortex-m4
    .thumb
    .align
    .global big2little32
    .global big2little16
    .thumb
    .thumb_func
 big2little32:
    rev r0, r0
    bx  lr
 big2little16:
    rev16   r0, r0
    bx  lr

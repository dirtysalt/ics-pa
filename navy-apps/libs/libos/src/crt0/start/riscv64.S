.globl  _start
_start:
  move s0, zero
  # context_uload
  # user stack pointer has been initialized to a0
  # and move to sp.
  move sp, a0
  jal call_main

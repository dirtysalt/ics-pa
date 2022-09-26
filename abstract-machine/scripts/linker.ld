ENTRY(_start)

SECTIONS {
  /* _pmem_start and _entry_offset are defined in LDFLAGS */
  . = _pmem_start + _entry_offset;
  .text : {
    *(entry)
    *(.text*)
  }
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }
  .data : {
    *(.data)
  }
  edata = .;
  _data = .;
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  }
  /* TODO(yan): 这个安排是把stack放在了heap之前 */
  _stack_top = ALIGN(0x1000);
  . = _stack_top + 0x8000;
  _stack_pointer = .;
  end = .;  
  _end = .;

  /* TODO(yan): 从这个地方开始进行sbrk */
  _brk_begin = .;
  . = _brk_begin + 0x8000;
  _brk_end = .;

  /* TODO(yan): 从这个地方开始做page allocation */  
  _heap_start = ALIGN(0x1000);
}

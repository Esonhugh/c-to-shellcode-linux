BASE = 0x00;
SECTIONS {
  . = BASE;
  .text : {
    . = BASE; 
    *(.text);
    *(.func);
  }
}
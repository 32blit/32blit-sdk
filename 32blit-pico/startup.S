.cpu cortex-m0
.thumb

.section .header, "a"

.word 0x54494C42
.word 0 // render
.word 0 // tick
.word 0 // init
.word __flash_binary_size
.word __flash_binary_start // unused

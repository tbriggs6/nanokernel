# built-in executables to the kernel
# these run in a different, virtual, address space from the kernel

.section .builtin
.global _idletask_start
.incbin "idle.bin"
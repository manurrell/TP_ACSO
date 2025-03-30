.text
mov X1, 0x100
cbz X1, error
cbnz X2, error
cbz X2, next

HLT 0

error:
mov X10, 0x100
HLT 0

next:
cbnz X1, end
mov X11, 0x100
HLT 0

end:

mov X12, 0x100
HLT 0


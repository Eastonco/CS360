        .global getebp
# getebp: returns the value of the EBP (Base Pointer) register.
#
# In x86 32-bit calling convention:
#   - EBP (Base/Frame Pointer) always points to the start of the current
#     stack frame. The word at [EBP+0] is the caller's saved EBP, and
#     [EBP+4] is the return address back to the caller.
#
# By moving EBP into EAX (the return-value register) and returning,
# this tiny assembly stub lets C code read the raw EBP value — something
# the C language itself has no syntax for.
getebp:
        movl %ebp, %eax   # copy EBP -> EAX (EAX is the integer return register)
        ret               # return to caller; caller sees EBP value as return value

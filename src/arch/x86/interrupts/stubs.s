.section .text

# Macro to define an interrupt stub
.macro ISR_NOERRCODE, num
.global isr\num
isr\num:
    cli                         # Disable interrupts
    push $0                     # Push dummy error code
    push $\num                  # Push interrupt number
    jmp isr_common_stub
.endm

.macro ISR_ERRCODE, num
.global isr\num
isr\num:
    cli                         # Disable interrupts
    push $\num                  # Push interrupt number
    jmp isr_common_stub
.endm

.macro IRQ, num, int_num
.global irq\num
irq\num:
    cli                         # Disable interrupts
    push $0                     # Push dummy error code
    push $\int_num              # Push interrupt number
    jmp irq_common_stub
.endm

# Define the actual interrupt stubs
ISR_NOERRCODE 0                 # Divide by zero
ISR_NOERRCODE 1                 # Debug
ISR_NOERRCODE 2                 # Non-maskable interrupt
ISR_NOERRCODE 3                 # Breakpoint
ISR_NOERRCODE 4                 # Overflow
ISR_NOERRCODE 5                 # Bound range exceeded
ISR_NOERRCODE 6                 # Invalid opcode
ISR_NOERRCODE 7                 # Device not available
ISR_ERRCODE   8                 # Double fault
ISR_NOERRCODE 9                 # Coprocessor segment overrun
ISR_ERRCODE   10                # Invalid TSS
ISR_ERRCODE   11                # Segment not present
ISR_ERRCODE   12                # Stack segment fault
ISR_ERRCODE   13                # General protection fault
ISR_ERRCODE   14                # Page fault
ISR_NOERRCODE 15                # Reserved
ISR_NOERRCODE 16                # x87 floating point exception
ISR_ERRCODE   17                # Alignment check
ISR_NOERRCODE 18                # Machine check
ISR_NOERRCODE 19                # SIMD floating point exception
ISR_NOERRCODE 20                # Virtualization exception
ISR_NOERRCODE 21                # Control protection exception
# Add more if needed...

# IRQ stubs (remapped to 32-47)
IRQ 0, 32                       # Timer
IRQ 1, 33                       # Keyboard
IRQ 2, 34                       # Cascade (if PIC is used in cascade mode)
IRQ 3, 35                       # COM2
IRQ 4, 36                       # COM1
IRQ 5, 37                       # LPT2
IRQ 6, 38                       # Floppy disk
IRQ 7, 39                       # LPT1 / Spurious interrupt
IRQ 8, 40                       # CMOS real-time clock
IRQ 9, 41                       # Free for peripherals / legacy SCSI / NIC
IRQ 10, 42                      # Free for peripherals / SCSI / NIC
IRQ 11, 43                      # Free for peripherals / SCSI / NIC
IRQ 12, 44                      # PS2 Mouse
IRQ 13, 45                      # FPU / Coprocessor / Inter-processor
IRQ 14, 46                      # Primary ATA Hard Disk
IRQ 15, 47                      # Secondary ATA Hard Disk

# Common ISR stub
isr_common_stub:
    pusha                       # Push all general purpose registers

    mov %ds, %ax                # Save data segment
    push %eax

    mov $0x10, %ax              # Load kernel data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    call default_interrupt_handler # Call the C handler

    pop %eax                    # Restore data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa                        # Restore general purpose registers
    add $8, %esp                # Clean up error code and interrupt number
    sti                         # Re-enable interrupts
    iret                        # Return from interrupt

# Common IRQ stub
irq_common_stub:
    pusha                       # Push all general purpose registers

    mov %ds, %ax                # Save data segment
    push %eax

    mov $0x10, %ax              # Load kernel data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    call default_interrupt_handler # Call the C handler

    # Send EOI to PIC
    movb $0x20, %al
    outb %al, $0x20             # Send EOI to master PIC
    cmpb $40, 24(%esp)          # Check if IRQ >= 40 (slave PIC)
    jl .irq_master_only
    outb %al, $0xA0             # Send EOI to slave PIC if needed
.irq_master_only:

    pop %eax                    # Restore data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa                        # Restore general purpose registers
    add $8, %esp                # Clean up error code and interrupt number
    sti                         # Re-enable interrupts
    iret                        # Return from interrupt

# Function to load IDT
.global load_idt
load_idt:
    mov 4(%esp), %eax           # Get pointer to IDT descriptor
    lidt (%eax)                 # Load IDT
    ret

# Function to enable interrupts
.global enable_interrupts_asm
enable_interrupts_asm:
    sti
    ret

# Function to disable interrupts
.global disable_interrupts_asm
disable_interrupts_asm:
    cli
    ret
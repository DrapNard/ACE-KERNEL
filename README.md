# ACE Micro-Kernel

ACE is a teaching-oriented x86 kernel that boots through a Multiboot entry point,
initialises a small set of core subsystems, and exposes a minimal shell for
interaction. The repository has been reorganised to make the codebase easier to
extend: sources live in `src/`, headers in `include/`, and a tiny freestanding
standard library sits in `src/libk`.

## Features

- Multiboot-compliant entry stub for QEMU and legacy BIOS.
- VGA text-mode console with a lightweight `vga_printf`.
- Keyboard interrupt driver and circular input buffer.
- Heap allocator (`kmalloc`/`kfree`) with block coalescing.
- Cooperative scheduler scaffold with an idle task.
- Toy VFS layer with in-memory nodes and simple open/read/write helpers.
- Basic syscall dispatcher piping to the kernel subsystems.
- Interactive shell (`shell_run`) with commands backed by the VFS and heap.
- Custom `libk` primitives (`k_memcpy`, `k_memset`, `k_memmove`, `k_memcmp`).

Enable in-kernel self-tests (syscalls + VFS smoke checks) with `SELFTEST=1` at
build time.

## Build & Run

### Prerequisites

- Cross GCC toolchain (`i686-elf-*` or `x86_64-elf-*`).
- GNU `ld`/`objcopy` matching the cross toolchain.
- `qemu-system-i386` (or `qemu-system-x86_64` for 64-bit builds).

### Commands

```bash
# Build a 32-bit kernel (default)
make

# Run inside QEMU (serial output redirected to the terminal)
make run

# Run with built-in self tests enabled
make SELFTEST=1 run

# Cross-compile for x86_64 (experimental)
make BITS=64
```

Build artefacts are placed under `build/` (`kernel.elf`, `kernel.bin`,
`kernel.map`).

## Repository Layout

```
include/
  ace/        # Shared types & macros
  arch/x86/   # Port I/O helpers
  core/       # Kernel-facing headers (scheduler, interrupts, ...)
  drivers/    # Device interfaces (VGA, keyboard)
  fs/         # VFS structures
  libk/       # Kernel libc replacements
  mm/         # Heap interface
  shell/      # Shell API
  sys/        # Syscall table and numbers
  tests/      # Self-test declarations

src/
  arch/x86/boot/   # Multiboot entry (assembly)
  core/            # Kernel subsystems (init, scheduler, shell, syscalls,...)
  drivers/         # Driver implementations
  fs/              # Virtual filesystem
  libk/            # Freestanding libc helpers
  mm/              # Heap allocator
  tests/           # Optional runtime tests

linker.ld          # Kernel linker script (loaded at 0x00100000)
Makefile           # Recursive build over src/
```

## Shell Commands

| Command | Description |
|---------|-------------|
| `help`  | List available commands |
| `clear` | Clear VGA text buffer |
| `echo`  | Echo arguments |
| `ls [path]` | Enumerate VFS entries |
| `cat <path>` | Dump a file |
| `mkdir <path>` | Create directory |
| `touch <path>` | Create empty file |
| `rm <path>` | Unlink file or empty directory |
| `mem` | Print heap statistics |
| `ps`, `kill`, `uptime`, `exit` | Placeholders / future work |

## Kernel Standard Library (`libk`)

`libk` replaces the usual C library facilities that are unavailable in kernel
space. The implementation is intentionally small:

- `k_memcpy`, `k_memmove`, `k_memset`, `k_memcmp`
- Optional helpers can be added here as the kernel grows (string handling, math
  primitives, etc.).

Include `<libk/mem.h>` and rely on the `ace/types.h` definitions instead of
`<stdint.h>`/`<stddef.h>`.

## Self Tests

The `src/tests` directory contains runtime smoke tests for syscalls and the
VFS. Compile them into the image by building with `SELFTEST=1`. During boot the
kernel will output the test progress before dropping into the shell.

```bash
make SELFTEST=1 run
```

## Next Steps

- Flesh out the scheduler (context switching + process table integration).
- Back VFS nodes with real storage.
- Replace polling shell input with event-driven tasks.
- Expand `libk` with string/format helpers as required.
- Reintroduce ISO generation (GRUB) as a dedicated target.

---

ACE is designed as a learning playground. Explore, instrument, and extend the
subsystems freely!

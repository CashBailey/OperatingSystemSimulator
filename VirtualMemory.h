#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include <cstddef>
#include <cstdint>

// Configuration macros (you can override these before including this header)
#ifndef VM_PAGE_SIZE
#define VM_PAGE_SIZE 256
#endif

#ifndef VM_PHYS_FRAMES
#define VM_PHYS_FRAMES 8
#endif

#ifndef VM_MAX_PROCESSES
#define VM_MAX_PROCESSES 128
#endif

// Initialize the virtual memory system.
// total_frames is kept for API clarity; current implementation uses VM_PHYS_FRAMES.
void vm_init(std::size_t total_frames = VM_PHYS_FRAMES);

// Create a new process's address space.
// pid: process identifier (0 <= pid < VM_MAX_PROCESSES recommended).
// memory_bytes: size of virtual address space in bytes for this process.
void vm_create_process(int pid, std::size_t memory_bytes);

// Read a single byte from a process's virtual address.
// pid: process identifier.
// address: virtual address in the process's address space [0, memory_bytes).
// Returns the byte value read.
std::uint8_t vm_read(int pid, std::size_t address);

// Write a single byte to a process's virtual address.
// pid: process identifier.
// address: virtual address in the process's address space [0, memory_bytes).
// value: byte value to write.
void vm_write(int pid, std::size_t address, std::uint8_t value);

// Free all memory associated with a process.
// Writes back dirty pages, frees frames, and releases page table and backing store.
void vm_free_process(int pid);

// Optional: dump internal VM state (frames and page tables) to stdout for debugging.
void vm_dump_state();

#endif // VIRTUAL_MEMORY_H

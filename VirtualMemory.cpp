#include "VirtualMemory.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// Internal configuration constants derived from macros
static constexpr std::size_t PAGE_SIZE     = VM_PAGE_SIZE;
static constexpr std::size_t PHYS_FRAMES   = VM_PHYS_FRAMES;
static constexpr std::size_t MAX_PROCESSES = VM_MAX_PROCESSES;

// Page table entry
struct PageTableEntry {
    int  frame_number;  // Index of the physical frame, or -1 if not in memory
    bool valid;         // Present bit
    bool dirty;         // Dirty bit
};

// Per-process page table
struct PageTable {
    int              num_pages; // Number of virtual pages
    PageTableEntry*  pages;     // Array of entries (size = num_pages)
};

// Backing store for a process (simulated disk image of its virtual memory)
struct BackingStore {
    std::uint8_t* data; // Size = num_pages * PAGE_SIZE
};

// Global physical memory and frame tracking
static std::uint8_t physical_memory[VM_PHYS_FRAMES * VM_PAGE_SIZE];
static bool         frame_in_use[VM_PHYS_FRAMES];
static int          frame_owner[VM_PHYS_FRAMES]; // pid owning this frame, or -1
static int          frame_page[VM_PHYS_FRAMES];  // virtual page index in this frame, or -1

// Per-process page tables and backing stores (indexed by pid)
static PageTable*    process_page_table[VM_MAX_PROCESSES];
static BackingStore  process_backing_store[VM_MAX_PROCESSES];

// FIFO replacement pointer
static int next_victim_frame = 0;

// Internal helpers
static void vm_check_pid(int pid) {
    if (pid < 0 || pid >= static_cast<int>(MAX_PROCESSES)) {
        std::fprintf(stderr, "VM error: pid %d out of range [0, %zu)\n",
                     pid, MAX_PROCESSES);
        std::exit(1);
    }
}

static PageTable* vm_get_page_table(int pid) {
    vm_check_pid(pid);
    PageTable* pt = process_page_table[pid];
    if (!pt) {
        std::fprintf(stderr, "VM error: process %d has no page table (not created?)\n", pid);
        std::exit(1);
    }
    return pt;
}

// Handle a page fault by loading the requested page into some frame.
// Returns the frame number used.
static int vm_handle_page_fault(int pid, int page_number) {
    PageTable* pt = vm_get_page_table(pid);

    if (page_number < 0 || page_number >= pt->num_pages) {
        std::fprintf(stderr,
                     "Segmentation fault: process %d tried to access invalid page %d\n",
                     pid, page_number);
        std::exit(1);
    }

    // 1. Find a free frame, if available
    int frame = -1;
    for (std::size_t f = 0; f < PHYS_FRAMES; ++f) {
        if (!frame_in_use[f]) {
            frame = static_cast<int>(f);
            break;
        }
    }

    // 2. If none free, evict a victim using FIFO
    if (frame == -1) {
        frame = next_victim_frame;
        next_victim_frame = (next_victim_frame + 1) % static_cast<int>(PHYS_FRAMES);

        int victim_pid  = frame_owner[frame];
        int victim_page = frame_page[frame];

        if (victim_pid < 0 || victim_pid >= static_cast<int>(MAX_PROCESSES) ||
            victim_page < 0) {
            std::fprintf(stderr,
                         "VM internal error: victim frame %d has invalid metadata\n",
                         frame);
            std::exit(1);
        }

        PageTable* victim_pt = vm_get_page_table(victim_pid);
        PageTableEntry& victim_pte = victim_pt->pages[victim_page];

        std::printf("VM: Evicting page %d of process %d from frame %d\n",
                    victim_page, victim_pid, frame);

        if (victim_pte.dirty) {
            // Write back frame contents to victim's backing store
            std::uint8_t* dst = process_backing_store[victim_pid].data +
                                static_cast<std::size_t>(victim_page) * PAGE_SIZE;
            std::uint8_t* src = physical_memory +
                                static_cast<std::size_t>(frame) * PAGE_SIZE;
            std::memcpy(dst, src, PAGE_SIZE);
            victim_pte.dirty = false;

            std::printf("VM: ... page was dirty, written back to backing store\n");
        }

        // Mark victim page as not present
        victim_pte.valid        = false;
        victim_pte.frame_number = -1;
        // Frame remains "in use" but will now belong to the new page.
    }

    // 3. Load requested page from backing store into the chosen frame
    std::printf("VM: Loading page %d of process %d into frame %d\n",
                page_number, pid, frame);

    std::uint8_t* frame_ptr = physical_memory +
                              static_cast<std::size_t>(frame) * PAGE_SIZE;
    std::uint8_t* src       = process_backing_store[pid].data +
                              static_cast<std::size_t>(page_number) * PAGE_SIZE;

    std::memcpy(frame_ptr, src, PAGE_SIZE);

    // 4. Update page table and frame metadata
    PageTableEntry& pte = pt->pages[page_number];
    pte.frame_number = frame;
    pte.valid        = true;
    pte.dirty        = false;

    frame_in_use[frame] = true;
    frame_owner[frame]  = pid;
    frame_page[frame]   = page_number;

    return frame;
}

// Public API implementations

void vm_init(std::size_t /*total_frames*/) {
    // Initialize physical memory and frame metadata
    std::memset(physical_memory, 0, sizeof(physical_memory));

    for (std::size_t f = 0; f < PHYS_FRAMES; ++f) {
        frame_in_use[f] = false;
        frame_owner[f]  = -1;
        frame_page[f]   = -1;
    }

    // Initialize per-process structures
    for (std::size_t pid = 0; pid < MAX_PROCESSES; ++pid) {
        process_page_table[pid]        = nullptr;
        process_backing_store[pid].data = nullptr;
    }

    next_victim_frame = 0;

    std::printf("VM: Initialized with %zu frames, page size %zu bytes\n",
                PHYS_FRAMES, PAGE_SIZE);
}

void vm_create_process(int pid, std::size_t memory_bytes) {
    vm_check_pid(pid);

    if (process_page_table[pid] != nullptr ||
        process_backing_store[pid].data != nullptr) {
        std::fprintf(stderr,
                     "VM error: process %d already has a page table/backing store\n",
                     pid);
        std::exit(1);
    }

    // Calculate number of pages (ceil division)
    int num_pages = static_cast<int>(
        (memory_bytes + PAGE_SIZE - 1) / PAGE_SIZE
    );

    if (num_pages <= 0) {
        num_pages = 1; // at least one page
    }

    // Allocate page table
    PageTable* pt = new PageTable;
    pt->num_pages = num_pages;
    pt->pages     = new PageTableEntry[num_pages];

    for (int i = 0; i < num_pages; ++i) {
        pt->pages[i].frame_number = -1;
        pt->pages[i].valid        = false;
        pt->pages[i].dirty        = false;
    }

    process_page_table[pid] = pt;

    // Allocate backing store (zero-initialized)
    std::size_t backing_size = static_cast<std::size_t>(num_pages) * PAGE_SIZE;
    std::uint8_t* backing = static_cast<std::uint8_t*>(
        std::calloc(backing_size, 1)
    );

    if (!backing) {
        std::fprintf(stderr,
                     "VM error: failed to allocate backing store for process %d\n",
                     pid);
        std::exit(1);
    }

    process_backing_store[pid].data = backing;

    std::printf("VM: Created process %d with %d pages (%zu bytes requested)\n",
                pid, num_pages, memory_bytes);
}

std::uint8_t vm_read(int pid, std::size_t address) {
    PageTable* pt = vm_get_page_table(pid);

    int page_number = static_cast<int>(address / PAGE_SIZE);
    int offset      = static_cast<int>(address % PAGE_SIZE);

    if (page_number < 0 || page_number >= pt->num_pages) {
        std::fprintf(stderr,
                     "VM error: process %d read address %zu out of range "
                     "(pages: %d, page_size: %zu)\n",
                     pid, address, pt->num_pages, PAGE_SIZE);
        std::exit(1);
    }

    PageTableEntry& pte = pt->pages[page_number];

    if (!pte.valid) {
        std::printf("VM: Page fault on read by process %d, page %d\n",
                    pid, page_number);
        vm_handle_page_fault(pid, page_number);
    }

    int frame = pte.frame_number;
    std::size_t phys_index = static_cast<std::size_t>(frame) * PAGE_SIZE +
                             static_cast<std::size_t>(offset);

    std::uint8_t value = physical_memory[phys_index];

    std::printf("VM: Read P%d[addr=%zu] -> page %d, frame %d, offset %d = %u\n",
                pid, address, page_number, frame, offset,
                static_cast<unsigned>(value));

    return value;
}

void vm_write(int pid, std::size_t address, std::uint8_t value) {
    PageTable* pt = vm_get_page_table(pid);

    int page_number = static_cast<int>(address / PAGE_SIZE);
    int offset      = static_cast<int>(address % PAGE_SIZE);

    if (page_number < 0 || page_number >= pt->num_pages) {
        std::fprintf(stderr,
                     "VM error: process %d write address %zu out of range "
                     "(pages: %d, page_size: %zu)\n",
                     pid, address, pt->num_pages, PAGE_SIZE);
        std::exit(1);
    }

    PageTableEntry& pte = pt->pages[page_number];

    if (!pte.valid) {
        std::printf("VM: Page fault on write by process %d, page %d\n",
                    pid, page_number);
        vm_handle_page_fault(pid, page_number);
    }

    int frame = pte.frame_number;
    std::size_t phys_index = static_cast<std::size_t>(frame) * PAGE_SIZE +
                             static_cast<std::size_t>(offset);

    physical_memory[phys_index] = value;
    pte.dirty = true;

    std::printf("VM: Wrote P%d[addr=%zu] -> page %d, frame %d, offset %d = %u\n",
                pid, address, page_number, frame, offset,
                static_cast<unsigned>(value));
}

void vm_free_process(int pid) {
    vm_check_pid(pid);

    PageTable* pt = process_page_table[pid];
    if (!pt && !process_backing_store[pid].data) {
        // Already freed or never created; nothing to do
        return;
    }

    if (pt) {
        // Write back any dirty pages in memory and free frames
        for (int i = 0; i < pt->num_pages; ++i) {
            PageTableEntry& pte = pt->pages[i];
            if (pte.valid && pte.frame_number >= 0) {
                int frame = pte.frame_number;

                if (pte.dirty) {
                    std::uint8_t* dst = process_backing_store[pid].data +
                                        static_cast<std::size_t>(i) * PAGE_SIZE;
                    std::uint8_t* src = physical_memory +
                                        static_cast<std::size_t>(frame) * PAGE_SIZE;
                    std::memcpy(dst, src, PAGE_SIZE);
                }

                // Free frame
                frame_in_use[frame] = false;
                frame_owner[frame]  = -1;
                frame_page[frame]   = -1;
            }
        }

        // Free page table
        delete[] pt->pages;
        delete pt;
        process_page_table[pid] = nullptr;
    }

    if (process_backing_store[pid].data) {
        std::free(process_backing_store[pid].data);
        process_backing_store[pid].data = nullptr;
    }

    std::printf("VM: Freed all memory for process %d\n", pid);
}

void vm_dump_state() {
    std::printf("===== VM STATE DUMP =====\n");
    std::printf("Physical memory: %zu frames, page size %zu bytes\n",
                PHYS_FRAMES, PAGE_SIZE);

    for (std::size_t f = 0; f < PHYS_FRAMES; ++f) {
        if (frame_in_use[f]) {
            std::printf("Frame %zu: used by PID %d, page %d\n",
                        f, frame_owner[f], frame_page[f]);
        } else {
            std::printf("Frame %zu: free\n", f);
        }
    }

    std::printf("\nPer-process page tables:\n");
    for (std::size_t pid = 0; pid < MAX_PROCESSES; ++pid) {
        PageTable* pt = process_page_table[pid];
        if (!pt) {
            continue;
        }
        std::printf("PID %zu: %d pages\n", pid, pt->num_pages);
        for (int i = 0; i < pt->num_pages; ++i) {
            const PageTableEntry& pte = pt->pages[i];
            std::printf("  page %d: valid=%d, dirty=%d, frame=%d\n",
                        i,
                        pte.valid ? 1 : 0,
                        pte.dirty ? 1 : 0,
                        pte.frame_number);
        }
    }
    std::printf("===== END VM STATE DUMP =====\n");
}

// Boot loader.
//
// Part of the boot block, along with bootasm.S, which calls bootmain().
// bootasm.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.

#include <types.h>
#include <elf.h>
#include <x86.h>

#define SECTSIZE  512

void readseg(u8*, u32, u32);

void bootmain(void) {
	struct elf_t *elf;
	struct prog_t *ph, *eph;
	void (*entry)(void);
	u8* pa;

	elf = (struct elf_t*) 0x10000;  // scratch space

	// Read 1st page off disk
	readseg((u8*) elf, 4096*20, 0);		//为了使用我的elf，强行读取20个页才行。因为光是elf头，就大概有（一页4kb）20页了......也就是80kb......

	// Is this an ELF executable?
	if (elf->e_magic != ELF_MAGIC)
		return;  // let bootasm.S handle error

	// Load each program segment (ignores ph flags).
	ph = (struct prog_t*) ((u8*) elf + elf->e_phoff);
	eph = ph + elf->e_phnum;
	for (; ph < eph; ph++) {
		pa = (u8*) ph->ph_paddr;
		readseg(pa, ph->ph_filesz, ph->ph_off);
		if (ph->ph_memsz > ph->ph_filesz)
			stosb(pa + ph->ph_filesz, 0, ph->ph_memsz - ph->ph_filesz);
	}

	// Call the entry point from the ELF header.
	// Does not return!
	entry = (void (*)(void)) (elf->e_entry);
	entry();
}

void waitdisk(void) {
	// Wait for disk ready.
	while ((inb(0x1F7) & 0xC0) != 0x40)
		;
}

// Read a single sector at offset into dst.
void readsect(void *dst, u32 offset) {
	// Issue command.
	waitdisk();
	outb(0x1F2, 1);   // count = 1
	outb(0x1F3, offset);
	outb(0x1F4, offset >> 8);
	outb(0x1F5, offset >> 16);
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

	// Read data.
	waitdisk();
	insl(0x1F0, dst, SECTSIZE / 4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void readseg(u8* pa, u32 count, u32 offset) {
	u8* epa;

	epa = pa + count;

	// Round down to sector boundary.
	pa -= offset % SECTSIZE;

	// Translate from bytes to sectors; kernel starts at sector 1.
	offset = (offset / SECTSIZE) + 1;

	// If this is too slow, we could read lots of sectors at a time.
	// We'd write more to memory than asked, but it doesn't matter --
	// we load in increasing order.
	for (; pa < epa; pa += SECTSIZE, offset++)
		readsect(pa, offset);
}

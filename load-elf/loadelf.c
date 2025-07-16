
#define __PRINT
#include "../common/common.h"
#define __X64
#include "include/elf_struct.h"
#include "./loadtype.c"

#ifdef __PRINT
#define LOG(str) print(str)
#else
#define LOG(str) (do {} while (true))
#endif

#define BADADDR ((void *) -1)

#define MMAP_LOAD_BASE ((void*) 0x0000000)


FUNC int check_header(elf_header* header) {
	if (*(uint*) header->e_ident != 0x464c457f) {
		LOG("elf magic header not detected.\n");
		return 0;
	}
	if (header->e_ident[4] != (sizeof(void*) / 4)) { // ei_class, 1: ELFCLASS32, 2: ELFCLASS64
		LOG("elf class mismatch.\n");
		return 0;
	}
	if (header->e_ident[5] != 1) {
		LOG("LSB expected.\n");
		return 0;
	}
	if (header->e_type != 2 && header->e_type != 3) {
		LOG("Dynamic library or executable expected.\n");
		return 0;
	}
	if (header->e_ehsize != sizeof(elf_header)) {
		LOG("Unexpected header size.\n");
		return 0;
	}
	return 1;
}


FUNC void* MY_DLOPEN(const char* buf, size_t buf_size) {
	elf_header* header = (elf_header*)((void*)buf);
	LOG("checking elf header...\n");
	if (!check_header(header)) {
		return BADADDR;
	}

	elf_program_header* pheader = NULL;
	elf_dyn* dyn = NULL;

	int e_phentsize = header->e_phentsize;
	int e_phnum = header->e_phnum;

	if (e_phentsize != sizeof(elf_program_header)) {
		LOG("unexpected program header size.\n");
		return BADADDR;
	}

	int is_pie; // simple detection, not exact
	LOG("determine pie:\n");
	
	
	// lseek(fd, header->e_phoff, SEEK_SET);
	for (int i = 0; i < e_phnum; i++) {
		// Load Pheader 
		// 		LOGV("scanning phdr %d...\n", i);
		// if (read(fd, &pheader, sizeof(pheader)) != sizeof(pheader)) {
		// 	LOGE("read pheader error\n");
		//	close(fd);
		//	return BADADDR;
		// }
		pheader = (elf_program_header*) ((size_t) buf + header-> e_phoff + i * sizeof(elf_program_header));
		if (pheader->p_type != 1 || pheader->p_memsz == 0) { // not PT_LOAD or nothing to load
			LOG("not load or p_memsz is 0\n");
			continue;
		}
		if (pheader->p_offset != 0) { // not header
			LOG("p_offset is not 0, not header\n");
			continue;
		}
		if (pheader->p_vaddr == 0) {
			LOG("p_vaddr is 0, is pie\n");
			is_pie = 1; // load 0 to 0 (pie)
		} else {
			LOG("p_vaddr is not 0, not pie\n");
			is_pie = 0; // load 0 to 0x??? (maybe not pie)
		}
		break;
	}
	
	void* base;
	if (is_pie) {
		LOG("pie\n");
		base = MMAP_LOAD_BASE;
		LOG("determine LOAD_BASE...\n");
		// try to find a free address

		while (base != mmap(base, 0x1000, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)) {
			base = (void*) ((size_t) base + 0x1000000);
		}
		munmap(base, 0x1000);
	} else {
		LOG("not pie\n");
		base = NULL;
	}
	LOG("trying loading at ");
	print_hex(base);
	LOG("\n");

	// lseek(fd, header.e_phoff, SEEK_SET);
	for (int i = 0; i < e_phnum; i++) {
		LOG("processing phdr ...\n");
		// if (read(fd, &pheader, sizeof(pheader)) != sizeof(pheader)) {
		//	LOG("read pheader error\n");
		//	close(fd);
		//	return BADADDR;
		//}
		pheader = (elf_program_header*) ((size_t) buf + header->e_phoff + i * sizeof(elf_program_header));
		if (pheader->p_type != 1 || pheader->p_memsz == 0) { // not PT_LOAD or nothing to load
			if (pheader->p_type == 2) { // DYNAMIC
				LOG("DYNAMIC PHT detected.\n");
				if (dyn != NULL) {
					LOG("duplicated DYNAMIC PHT detected.\n");
					return BADADDR;
				} else {
					dyn = (elf_dyn*) ((size_t) base + pheader->p_vaddr);
					LOG("DYN: ");
					print_hex(dyn);
					LOG("\n");
				}
			}
			LOG("ptype ");
			print_hex(pheader->p_type);
			LOG(" memsz is ");
			print_hex(pheader->p_memsz);
			LOG(" vaddr is ");
			print_hex(pheader->p_vaddr);
			LOG(" skipping...\n");
			continue;
		} 
		// PT_LOAD
		LOG("ptype ");
		print_hex(pheader->p_type);
		LOG("\n[PT_LOAD] start loading ");
		print_hex(pheader->p_offset);
		LOG(" to ");
		print_hex(pheader->p_vaddr + (size_t) base);
		LOG(".\n");

		void* addr = (void*) (((size_t) base + pheader->p_vaddr) & ~0xfff);
		int offset = pheader->p_vaddr & 0xfff;
		size_t size = (offset + pheader->p_filesz + 0xfff) & ~0xfff;
		if (addr != mmap(addr, size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, pheader->p_offset - offset)) {
			LOG("failed to mmap ");
			print_hex(pheader->p_offset); // 0x0000000000000000
			LOG(" to ");
			print_hex(pheader->p_vaddr + (size_t)base); // 0x0000000001000000
			LOG(" get ");
			print_hex(addr);
			LOG(".\n");
			return BADADDR;
		} else {
			LOG("succeed to mmap pheader ");
			print_hex(pheader->p_offset);
			LOG(" to ");
			print_hex(pheader->p_vaddr + (size_t) base);
			LOG(" get ");
			print_hex(addr);
			LOG(".\n");
			// try copy
			memcpy(addr, buf + pheader->p_offset - offset, size);
		}
		if (offset) {
			memset(addr, 0, offset); // not exactly needed
		}
		if (pheader->p_memsz != pheader->p_filesz) {
			if (pheader->p_memsz < pheader->p_filesz) {
				LOG("unexpected: filesz bigger than memsz.\n");
				return BADADDR;
			}
			if (pheader->p_memsz + offset > size) {
				LOG("mmap extra pages in memory\n");
				addr = (void*) (addr + size);
				if (addr != mmap(addr, pheader->p_memsz + offset - size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_ANONYMOUS| MAP_SHARED, -1, 0)) {
					// LOG("failed to mmap 0x%lx to 0x%lx.\n", pheader->p_offset, pheader->p_vaddr + (size_t) base);
					LOG("failed to mmap extra memory for pheader.\n");
					print_hex(pheader->p_offset);
					LOG(" to ");
					print_hex(pheader->p_vaddr + (size_t) base);
					LOG(".\n");
					return BADADDR;
				} else {
					LOG("succeed to mmap program memory ");
					print_hex(addr);
					LOG("\n");
				}
			}
			memset((void*) ((size_t) base + pheader->p_vaddr + pheader->p_filesz), 0, pheader->p_memsz - pheader->p_filesz);
		}
		{
			LOG("testing memory...\n");
			char c = *(unsigned char*) (pheader->p_vaddr + (size_t) base);
			c = *(unsigned char*) (pheader->p_vaddr + (size_t) base + pheader->p_filesz - 1);
			c = *(unsigned char*) (pheader->p_vaddr + (size_t) base + pheader->p_memsz - 1);
			c++; // to avoid warning: c not used
		}
		// LOG("mmaped 0x%lx to 0x%lx, filesz 0x%lx, memsz 0x%lx\n", pheader->p_offset, pheader->p_vaddr + (size_t) base, pheader->p_filesz, pheader->p_memsz);
		LOG("mmaped ");
		print_hex(pheader->p_offset);
		LOG(" to ");
		print_hex(pheader->p_vaddr + (size_t) base);
		LOG(", filesz ");
		print_hex(pheader->p_filesz);
		LOG(", memsz ");
		print_hex(pheader->p_memsz);
		LOG("\n");
	}
	LOG("mmap done\n");
	int fd = open("/proc/self/maps", 00);
	if (fd < 0) {
		LOG("failed to open /proc/self/maps\n");
		return BADADDR;
	}
	char file_buf [2048];
	read(fd, file_buf, 2048);
	write(1, file_buf, 2048);


	if (dyn) {
		LOG("DYNAMIC detected, loading...\n");
		if (!load_dynamic(base, dyn)) {
			return BADADDR;
		}
	} else {
		LOG("No DYNAMIC, checking static symbols...\n");
		if (!load_static(base, buf, header)) {
			return BADADDR;
		}
	}
	LOG("done, loaded at ");
	print_hex(base);
	LOG("\n");
	return base;
}

FUNC const elf_dyn* get_dyn(void* base) {
	elf_header* header = (elf_header*) base;
	int e_phnum = header->e_phnum;
	elf_program_header* pheader = (elf_program_header*) ((size_t) base + header->e_phoff);
	for (int i = 0; i < e_phnum; i++, pheader++) {
		if (pheader->p_type == 2) {
			return (elf_dyn*) ((size_t) base + pheader->p_vaddr);
		}
	}
	return NULL;
}

FUNC void* get_symbol_by_name(void* base, const char* symbol) {
	const elf_dyn* dyn = get_dyn(base);
	const char* strtab = (const char*) (find_dyn_entry(dyn, 5)->d_un); // DT_STRTAB

	if (strtab < (const char*) base)
		strtab = (const char*) strtab + (size_t) base;
	size_t strsz = find_dyn_entry(dyn, 0xa)->d_un; // DT_STRSZ
	const elf_sym* symtab = (const elf_sym*) (find_dyn_entry(dyn, 6)->d_un); // DT_SYMTAB
	if ((const char*) symtab < (const char*) base)
		symtab = (const elf_sym*) ((const char*) symtab + (size_t) base);

	for (; ; symtab++) {
		if (symtab->st_name == 0) continue;
		if (symtab->st_name >= strsz) {
			// LOGE("failed to resolve symbol `%s' from library (%p): not found.\n", symbol, base);
			return NULL;
		}
		if (strcmp(strtab + symtab->st_name, symbol) == 0) {
			if (symtab->st_value == 0) {
				// LOGE("failed to resolve symbol `%s' from library (%p): value is NULL.\n", symbol, base);
				return NULL;
			}
			if (elf_st_type(symtab->st_info) != 10) { // STT_GNU_IFUNC
				return (void*) ((size_t) base + symtab->st_value);
			}
			return ((void* (*)()) ((size_t) base + symtab->st_value))();
		}
	}
}

FUNC void* get_symbol_by_offset(void* base, size_t offset) {
	return (void*) ((size_t) base + offset);
}
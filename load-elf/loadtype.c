// don't use this function directly, use load_dynamic instead
FUNC int load_dynamic(void* base, const elf_dyn* dyn) {
	const elf_dyn* res = find_dyn_entry(dyn, 5); // DT_STRTAB
	if (res == NULL) {
		LOGE("string table not found.\n");
		return 0;
	}
	const char* strtab = (const char*) ((size_t) base + res->d_un);

	const elf_sym* symtab = NULL;
	res = find_dyn_entry(dyn, 0x6); // DT_SYMTAB
	if (res != NULL) {
		symtab = (const elf_sym*) ((size_t) base + res->d_un);
		if (find_dyn_entry(dyn, 0xB)->d_un != sizeof(elf_sym)) { // DT_SYMENT
			LOGE("unexpected symbol table entry size.\n");
			return 0;
		}
	}

	for (const elf_dyn* it = dyn; it->d_tag != 0; it++) {
		if (it->d_tag != 1) continue; // DT_NEEDED: name of needed library
		load_needed_library(strtab + it->d_un);
	}

	int rel_done = 0;
	for (const elf_dyn* it = dyn; it->d_tag != 0; it++) { // DT_NULL
		switch (it->d_tag) {
		case 7: // DT_RELA
			if (rel_done) break;
			if (!check_and_do_rela(base, dyn, (const elf_rela*) ((size_t) base + it->d_un), symtab, strtab))
				return 0;
			rel_done = 1;
			break;
		case 0x11: // DT_REL
			if (rel_done) break;
			if (!check_and_do_rel(base, dyn, (const elf_rel*) ((size_t) base + it->d_un), symtab, strtab))
				return 0;
			rel_done = 1;
			break;
		case 0x17: // DT_JMPREL
			;
			size_t plt_rel_size = find_dyn_entry(dyn, 0x2)->d_un; // DT_PLTRELSZ
			int plt_rel = find_dyn_entry(dyn, 0x14)->d_un; // DT_PLTREL
			if (plt_rel == 0x11) { // DT_REL
				if (!rel_done) {
					res = find_dyn_entry(dyn, 0x11); // DT_REL
					if (res != NULL) {
						if (!check_and_do_rel(base, dyn, (const elf_rel*) ((size_t) base + res->d_un), symtab, strtab))
							return 0;
						rel_done = 1;
					}
				}
				plt_rel_size /= sizeof(elf_rel);
				LOGD("do jmprel with rel.\n");
				if (!do_rel(base, (elf_rel*) ((size_t) base + it->d_un), plt_rel_size, symtab, strtab)) return 0;
			} else if (plt_rel == 7) { // DT_RELA
				if (!rel_done) {
					res = find_dyn_entry(dyn, 7); // DT_RELA
					if (res != NULL) {
						if (!check_and_do_rela(base, dyn, (const elf_rela*) ((size_t) base + res->d_un), symtab, strtab))
							return 0;
						rel_done = 1;
					}
				}
				plt_rel_size /= sizeof(elf_rela);
				LOGD("do jmprel with rela.\n");
				if (!do_rela(base, (elf_rela*) ((size_t) base + it->d_un), plt_rel_size, symtab, strtab)) return 0;
			} else {
				LOGE("unexpected plt rel type: %d.\n", plt_rel);
				return 0;
			}
			break;
		}
	}

	res = find_dyn_entry(dyn, 0xC); // DT_INIT
	if (res != NULL) {
		void (*init)() = (void (*)()) ((size_t) base + res->d_un);
		LOGI("init proc detected: %p.\n", init);
		int choice = 'y';
		if (!init_array_filter) {
			do {
				LOGI("Execute init proc? [(y)es/(n)o] ");
				choice = getchar();
				if (choice != '\n') while (getchar() != '\n') ;
				if (choice >= 'A' && choice <= 'Z') choice += 0x20;
			} while (choice != 'y' && choice != 'n');
		} else if (init_array_filter(base, init)) {
			choice = 'y';
		} else {
			choice = 'n';
		}
		if (choice == 'y') {
			LOGI("\texecuting init at %p...\n", init);
			init();
		} else {
			LOGI("\t skipping init at %p...\n", init);
		}
	}

	res = find_dyn_entry(dyn, 0x19); // DT_INIT_ARRAY
	if (res != NULL) {
		void (**init_array)() = (void (**)()) ((size_t) base + res->d_un);
		int count = find_dyn_entry(dyn, 0x1B)->d_un / sizeof(size_t); // DT_INIT_ARRAYSZ
		while (*init_array == NULL && count) {
			init_array++;
			count--;
		}
		if (count) {
			LOGI("init array detected:\n");
			int choice = '?';
			for (int i = 0; i < count; i++) {
				if (!init_array[i]) continue;
				while (!init_array_filter && choice != 'y' && choice != 'n' && choice != 'a' && choice != 'o') {
					LOGI("\texecute function %p? [(y)es/(n)o/(a)ll items left/n(o)ne items left] ", init_array[i]);
					choice = getchar();
					if (choice != '\n') while (getchar() != '\n') ; // skip line
					if (choice >= 'A' && choice <= 'Z') choice += 0x20; // convert to lower case
				}
				if (init_array_filter) {
					if (init_array_filter(base, init_array[i])) {
						LOGI("\texecuting function at %p...\n", init_array[i]);
						init_array[i]();
					} else {
						LOGI("\t skipping function at %p...\n", init_array[i]);
					}
				} else if ((uchar) (choice - 'n') > 2) { // 'y' or 'a'
					LOGI("\texecuting function at %p...\n", init_array[i]);
					init_array[i]();
					if (choice == 'y') choice = '?';
				} else if (choice == 'n') choice = '?';
			}
		}
	}

	res = find_dyn_entry(dyn, 0xD); // DT_FINI
	if (res != NULL) {
		void (*fini)() = (void (*)()) ((size_t) base + res->d_un);
		LOGI("fini proc detected: %p.\n", fini);
	}

	res = find_dyn_entry(dyn, 0x1A); // DT_FINI_ARRAY
	if (res != NULL) {
		void (**fini_array)() = (void (**)()) ((size_t) base + res->d_un);
		int count = find_dyn_entry(dyn, 0x1C)->d_un / sizeof(size_t); // DT_FINI_ARRAYSZ
		while (*fini_array == NULL && count) {
			fini_array++;
			count--;
		}
		if (count) {
			LOGI("fini array detected:\n");
			for (int i = 0; i < count; i++) {
				if (fini_array[i]) {
					LOGI("\t%p\n", fini_array[i]);
				}
			}
		}
	}
	LOGI("load_dynamic done.\n");
	return 1;
}

// don't use this function directly, use load_static instead
FUNC int load_static(void* base, const char * buf, elf_header* header) {
	if (header->e_shentsize != sizeof(elf_section_header)) {
		LOGW("Unexpected section header entry size, skipped load_static\n");
		return 1; // something went wrong, maybe nothing important
	}
	if (header->e_shnum == 0) {
		LOGW("No section header\n");
		return 1; // maybe it's ok?
	}
	if (header->e_shnum <= header->e_shtrndx) {
		LOGE("string table index out of range\n");
		return 0;
	}
	elf_section_header sheader;
	// lseek(fd, header->e_shoff + sizeof(elf_section_header) * header->e_shtrndx, SEEK_SET);
	sheader = *((elf_section_header*) (buf + header->e_shoff + sizeof(elf_section_header) * header->e_shtrndx));
	//if (read(fd, &sheader, sizeof(sheader)) != sizeof(sheader)) {
	//	LOGE("read section header error\n");
	//	return 0;
	//}
	size_t strtab_size = sheader.s_size;
	char* strtab = (char*) malloc(strtab_size + 1);
	
	// lseek(fd, sheader.s_offset, SEEK_SET);
	//if (read(fd, strtab, strtab_size) != strtab_size) {
	//	LOGE("read section header string table error\n");
	//	free(strtab);
	//	return 0;
	//}
	memcpy(strtab, buf + sheader.s_offset, strtab_size);
	strtab[strtab_size] = 0;
	// ignored init
	// there's no SHT_INIT, and we can only determine init by section name ".init"
	// string compare is ugly and unexpected
	// nowadays init is always set to a empty function
	// so we just ignore it
	// if you indeed need to call init, call in your main or init_array_filter.
	// void (*init)() = NULL;
	void (**init_array)() = NULL;
	size_t init_array_count;
	// ignored fini
	// same as init
	// void (*fini)() = NULL;
	void (**fini_array)() = NULL;
	size_t fini_array_count;

	// lseek(fd, header->e_shoff, SEEK_SET);
	for (int i = 0; i < header->e_shnum; i++) {
		// if (read(fd, &sheader, sizeof(sheader)) != sizeof(sheader)) {
		//	LOGE("read section header error\n");
		//	free(strtab);
		//	return 0;
		//}
		sheader = *((elf_section_header*) (buf + header->e_shoff + sizeof(elf_section_header) * i));

		if (sheader.s_size == 0) continue;
		if (sheader.s_name >= strtab_size) {
			LOGE("bad section name\n");
			free(strtab);
			return 0;
		}
		if (sheader.s_addr) {
			//LOGV("section %d: `%s' (in memory: %p - %p)\n", i, strtab + sheader.s_name, (void*) ((size_t) base + sheader.s_addr), (void*) ((size_t) base + sheader.s_addr + sheader.s_size));
		} else {
			// LOGV("section %d: `%s' (in file: %p - %p)\n", i, strtab + sheader.s_name, (void*) ((size_t) base + sheader.s_offset), (void*) ((size_t) base + sheader.s_offset + sheader.s_size));
		}
		switch (sheader.s_type) {
		case 4: // SHT_RELA
			if (sheader.s_entsize != sizeof(elf_rela)) {
				LOG("bad rela entry size\n");
				// free(strtab);
				return 0;
			}
			if (sheader.s_size % sizeof(elf_rela) != 0) {
				LOG("bad rela size\n");
				// free(strtab);
				return 0;
			}
			LOGD("detected rela\n");
			// All items should be R_IRELATIVE
			// Here we just ignore this check
			do_rela(base, (elf_rela*) ((size_t) base + sheader.s_addr), sheader.s_size / sizeof(elf_rela), NULL, NULL);
			break;
		case 9: // SHT_REL
			if (sheader.s_entsize != sizeof(elf_rel)) {
				LOG("bad rel entry size\n");
				// free(strtab);
				return 0;
			}
			if (sheader.s_size % sizeof(elf_rel) != 0) {
				LOG("bad rel size\n");
				// free(strtab);
				return 0;
			}
			LOGD("detected rel\n");
			// All items should be R_IRELATIVE
			// Here we just ignore this check
			do_rel(base, (elf_rel*) ((size_t) base + sheader.s_addr), sheader.s_size / sizeof(elf_rel), NULL, NULL);
			break;
		case 14: // SHT_INIT_ARRAY
			init_array = (void*) ((size_t) base + sheader.s_addr);
			init_array_count = sheader.s_size;
			if (init_array_count % sizeof(size_t) != 0) {
				LOG("bad init array size\n");
				// free(strtab);
				return 0;
			}
			init_array_count /= sizeof(size_t);
			while (init_array_count && *init_array == NULL) {
				init_array++;
				init_array_count--;
			}
			break;
		case 15: // SHT_FINI_ARRAY
			fini_array = (void*) ((size_t) base + sheader.s_addr);
			fini_array_count = sheader.s_size;
			if (fini_array_count % sizeof(size_t) != 0) {
				LOGD("bad init array size\n");
				free(strtab);
				return 0;
			}
			fini_array_count /= sizeof(size_t);
			while (fini_array_count && *fini_array == NULL) {
				fini_array++;
				fini_array_count--;
			}
			break;
		default:
			break;
		}
	}
	// free(strtab);
	if (init_array && init_array_count) {
		LOGI("init array detected:\n");
		// int choice = '?';
		for (int i = 0; i < init_array_count; i++) {
			if (!init_array[i]) continue;
			// choice exec
			/*
			while (!init_array_filter && choice != 'y' && choice != 'n' && choice != 'a' && choice != 'o') {
				LOGI("\texecute function %p? [(y)es/(n)o/(a)ll items left/n(o)ne items left] ", init_array[i]);
				choice = getchar();
				if (choice != '\n') while (getchar() != '\n') ; // skip line
				if (choice >= 'A' && choice <= 'Z') choice += 0x20; // convert to lower case
			}
			*/
			if (init_array_filter) {
				if (init_array_filter(base, init_array[i])) {
					LOG("\texecuting function at ");
					print_hex(init_array[i]);
					init_array[i]();
					LOG("\n");
				} else {
					LOG("\t skipping function at ");
					print_hex(init_array[i]);
					LOG("\n");
				}
			}
				LOG("executing function at ");
				print_hex(init_array[i]);
				LOG("\n");
				init_array[i]();
		}
	}
	if (fini_array && fini_array_count) {
		LOGI("fini array detected:\n");
		for (int i = 0; i < fini_array_count; i++) {
			if (fini_array[i]) {
				LOGI("\t%p\n", fini_array[i]);
			}
		}
	}
	LOGI("load_static done.\n");
	return 1;
}

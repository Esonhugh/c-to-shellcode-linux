#define __PRINT
#include "../common/common.h"
#include "include/elf_struct.h"

#ifdef __PRINT
#define LOG(str) print(str)
#else
#define LOG(str)  \
  do              \
  {               \
    if (0)        \
      print(str); \
  } while (0)
#endif

FUNC void *get_global_symbol(const char *symbol)
{
  return NULL;
}

FUNC const elf_dyn *find_dyn_entry(const elf_dyn *dyn, int type)
{
  for (; dyn->d_tag != 0; dyn++)
  { // DT_NULL
    if (dyn->d_tag == type)
      return dyn;
  }
  return NULL;
}

FUNC int filter(void *base, void (*init_array_item)())
{
  LOG("Executing init array item...\n");
  return 1;
}

static int (*init_array_filter)(void *base, void (*init_array_item)()) = filter;

FUNC void load_needed_library(const char *libname)
{
  LOG("need to load needed library ");
  LOG(libname);
  LOG("\n");
  // void* handle = dlopen(libname, RTLD_NOW | RTLD_GLOBAL);
  // if (handle == NULL) {
  //	LOG("failed to load needed library `%s': %s.\n", libname, dlerror());
  //}
}

#define R_COPY 5
#define R_GLOB_DAT 6
#define R_JUMP_SLOT 7
#define R_RELATIVE 8
#define R_IRELATIVE 37

FUNC int do_reloc(void *base, size_t offset, size_t info, size_t addend, const elf_sym *symtab, const char *strtab)
{
#define sym (elf_r_sym(info))
#define type (elf_r_type(info))
#define value (symtab[sym].st_value)
#define size (symtab[sym].st_size)
#define name (strtab + symtab[sym].st_name)
  switch (type)
  {
  case R_NONE:
    LOG("R_NONE.\n");
    break;
  case R_COPY:
    if (value)
    {
      LOG("R_COPY: from ");
      print_hex(value);
      LOG(" to ");
      print_hex(offset);
      LOG("\n");
      if (offset != value)
      {
        memcpy((void *)((size_t)base + offset), (const void *)((size_t)base + value), size);
      }
      else
      {
        LOG("Maybe unspecified R_COPY at ");
        print_hex(offset);
        LOG("\n");
        goto R_COPY_name;
      }
    }
    else
    {
    R_COPY_name:
      LOG("R_COPY: from");
      LOG(name);
      print_hex(offset);
      LOG("\n");
      const void *sym_value = get_global_symbol(name);
      if (!sym_value)
      {
        LOG("failed to resolve symbol ");
        LOG(name);
        LOG(".\n");
        break;
      }
      memcpy((void *)((size_t)base + offset), sym_value, size);
    }
    break;
  case R_GLOB_DAT:
  case R_JUMP_SLOT:
    if (value)
    {
      // LOGV("R_GLOB_DAT/R_JUMP_SLOT: set +0x%llx at +0x%llx.\n", value, offset);
      LOG("R_GLOB_DAT/R_JUMP_SLOT: set +");
      print_hex(value);
      LOG(" at +");
      print_hex(offset);
      LOG("\n");
      *(size_t *)((size_t)base + offset) = (size_t)base + value;
    }
    else
    {
      // LOGV("R_GLOB_DAT/R_JUMP_SLOT: set `%s' at +0x%llx.\n", name, offset);
      LOG("R_GLOB_DAT/R_JUMP_SLOT: set ");
      LOG(name);
      print_hex(offset);
      LOG("\n");
      const void *sym_value = get_global_symbol(name);
      if (!sym_value)
      {
        LOG("failed to resolve symbol");
        LOG(name);
        LOG(".\n");
        break;
      }
      *(size_t *)((size_t)base + offset) = (size_t)sym_value;
    }
    break;
  case R_RELATIVE:
    // LOGV("R_RELATIVE: set +0x%llx at +0x%llx.\n", addend, offset);
    // LOGV("R_RELATIVE: set +0x%llx at +0x%llx.\n", addend, offset);
    LOG("R_RELATIVE: set +");
    print_hex(addend);
    LOG(" at +");
    print_hex(offset);
    LOG("\n");
    *(size_t *)((size_t)base + offset) = (size_t)base + addend;
    break;
  case R_IRELATIVE:
    // LOGV("R_IRELATIVE: set (+0x%llx)() at +0x%llx.\n", addend, offset);
    LOG("R_IRELATIVE: set (+");
    print_hex(addend);
    LOG(")() at +");
    print_hex(offset);
    LOG("\n");

    *(size_t *)((size_t)base + offset) = ((size_t (*)())((size_t)base + addend))();
    break;
  case 1: // R_X86_64_64
    if (value)
    {
      // OGV("R_X86_64_64: set +0x%llx+0x%llx at +0x%llx.\n", value, addend, offset);
      // LOGV("R_X86_64_64: set +0x%llx+0x%llx at +0x%llx.\n", value, addend, offset);
      LOG("R_X86_64_64: set +");
      print_hex(value);
      LOG(" +");
      print_hex(addend);
      LOG(" at +");
      print_hex(offset);
      LOG("\n");

      *(size_t *)((size_t)base + offset) = (size_t)base + value + addend;
    }
    else
    {
      // OGV("R_X86_64_64: set `%s'+0x%llx at +0x%llx.\n", name, addend, offset);
      LOG("R_X86_64_64: set ");
      LOG(name);
      LOG(" +");
      print_hex(addend);
      LOG(" at +");
      print_hex(offset);
      LOG("\n");

      const void *sym_value = get_global_symbol(name);
      if (!sym_value)
      {
        // LOGW("failed to resolve symbol `%s'.\n", name);
        LOG("failed to resolve symbol ");
        LOG(name);
        break;
      }
      *(size_t *)((size_t)base + offset) = (size_t)sym_value + addend;
    }
    break;
  default:
    // LOGW("unimplemented reloc type: %d.\n", type);
    LOG("unimplemented reloc type!");
    break;
  }
#undef sym
#undef type
#undef value
#undef size
#undef name
  return 1;
}

FUNC int do_rel(void *base, const elf_rel *rel, int count, const elf_sym *symtab, const char *strtab)
{
  for (int i = 0; i < count; i++)
  {
    if (!do_reloc(base, rel[i].r_offset, rel[i].r_info, *(size_t *)((size_t)base + rel[i].r_offset), symtab, strtab))
      return 0;
  }
  return 1;
}

FUNC int do_rela(void *base, const elf_rela *rela, int count, const elf_sym *symtab, const char *strtab)
{
  LOG("doing rela\n");
  for (int i = 0; i < count; i++)
  {
    if (!do_reloc(base, rela[i].r_offset, rela[i].r_info, rela[i].r_addend, symtab, strtab))
      return 0;
  }
  return 1;
}

FUNC int check_and_do_rel(void *base, const elf_dyn *dyn, const elf_rel *rel, const elf_sym *symtab, const char *strtab)
{
  if (find_dyn_entry(dyn, 0x13)->d_un != sizeof(elf_rel))
  { // DT_RELENT
    LOG("unexpected rel table entry size.\n");
    return 0;
  }
  LOG("do rel.\n");
  int rel_count = find_dyn_entry(dyn, 0x12)->d_un / sizeof(elf_rel); // DT_RELSZ
  if (!do_rel(base, rel, rel_count, symtab, strtab))
    return 0;
  return 1;
}

FUNC int check_and_do_rela(void *base, const elf_dyn *dyn, const elf_rela *rela, const elf_sym *symtab, const char *strtab)
{
  if (find_dyn_entry(dyn, 0x9)->d_un != sizeof(elf_rela))
  { // DT_RELAENT
    LOG("unexpected rela table entry size.\n");
    return 0;
  }
  LOG("do rela.\n");
  int rela_count = find_dyn_entry(dyn, 0x8)->d_un / sizeof(elf_rela); // DT_RELASZ
  if (!do_rela(base, rela, rela_count, symtab, strtab))
    return 0;
  return 1;
}

// don't use this function directly, use load_dynamic instead
FUNC int load_dynamic(void *base, const elf_dyn *dyn)
{
  const elf_dyn *res = find_dyn_entry(dyn, 5); // DT_STRTAB
  if (res == NULL)
  {
    LOG("string table not found.\n");
    return 0;
  }
  const char *strtab = (const char *)((size_t)base + res->d_un);

  const elf_sym *symtab = NULL;
  res = find_dyn_entry(dyn, 0x6); // DT_SYMTAB
  if (res != NULL)
  {
    symtab = (const elf_sym *)((size_t)base + res->d_un);
    if (find_dyn_entry(dyn, 0xB)->d_un != sizeof(elf_sym))
    { // DT_SYMENT
      LOG("unexpected symbol table entry size.\n");
      return 0;
    }
  }

  for (const elf_dyn *it = dyn; it->d_tag != 0; it++)
  {
    if (it->d_tag != 1)
      continue; // DT_NEEDED: name of needed library
    load_needed_library(strtab + it->d_un);
  }

  int rel_done = 0;
  for (const elf_dyn *it = dyn; it->d_tag != 0; it++)
  { // DT_NULL
    switch (it->d_tag)
    {
    case 7: // DT_RELA
      if (rel_done)
        break;
      if (!check_and_do_rela(base, dyn, (const elf_rela *)((size_t)base + it->d_un), symtab, strtab))
        return 0;
      rel_done = 1;
      break;
    case 0x11: // DT_REL
      if (rel_done)
        break;
      if (!check_and_do_rel(base, dyn, (const elf_rel *)((size_t)base + it->d_un), symtab, strtab))
        return 0;
      rel_done = 1;
      break;
    case 0x17: // DT_JMPREL
        ;
      size_t plt_rel_size = find_dyn_entry(dyn, 0x2)->d_un; // DT_PLTRELSZ
      int plt_rel = find_dyn_entry(dyn, 0x14)->d_un;        // DT_PLTREL
      if (plt_rel == 0x11)
      { // DT_REL
        if (!rel_done)
        {
          res = find_dyn_entry(dyn, 0x11); // DT_REL
          if (res != NULL)
          {
            if (!check_and_do_rel(base, dyn, (const elf_rel *)((size_t)base + res->d_un), symtab, strtab))
              return 0;
            rel_done = 1;
          }
        }
        plt_rel_size /= sizeof(elf_rel);
        LOG("do jmprel with rel.\n");
        if (!do_rel(base, (elf_rel *)((size_t)base + it->d_un), plt_rel_size, symtab, strtab))
          return 0;
      }
      else if (plt_rel == 7)
      { // DT_RELA
        if (!rel_done)
        {
          res = find_dyn_entry(dyn, 7); // DT_RELA
          if (res != NULL)
          {
            if (!check_and_do_rela(base, dyn, (const elf_rela *)((size_t)base + res->d_un), symtab, strtab))
              return 0;
            rel_done = 1;
          }
        }
        plt_rel_size /= sizeof(elf_rela);
        LOG("do jmprel with rela.\n");
        if (!do_rela(base, (elf_rela *)((size_t)base + it->d_un), plt_rel_size, symtab, strtab))
          return 0;
      }
      else
      {
        LOG("unexpected plt rel type!\n");
        return 0;
      }
      break;
    }
  }

  res = find_dyn_entry(dyn, 0xC); // DT_INIT
  if (res != NULL)
  {
    void (*init)() = (void (*)())((size_t)base + res->d_un);
    // LOG("init proc detected: %p.\n", init);
    LOG("\texecuting init at");
    print_hex(init);
    LOG("\n");
    init();
  }

  res = find_dyn_entry(dyn, 0x19); // DT_INIT_ARRAY
  if (res != NULL)
  {
    void (**init_array)() = (void (**)())((size_t)base + res->d_un);
    int count = find_dyn_entry(dyn, 0x1B)->d_un / sizeof(size_t); // DT_INIT_ARRAYSZ
    while (*init_array == NULL && count)
    {
      init_array++;
      count--;
    }
    if (count)
    {
      LOG("init array detected:\n");
      for (int i = 0; i < count; i++)
      {
        if (!init_array[i])
          continue;
        LOG("\texecuting function at ");
        print_hex(init_array[i]);
        LOG("...\n");
        init_array[i]();
      }
    }
  }

  res = find_dyn_entry(dyn, 0xD); // DT_FINI
  if (res != NULL)
  {
    void (*fini)() = (void (*)())((size_t)base + res->d_un);
    LOG("fini proc detected");
    print_hex(fini);
  }

  res = find_dyn_entry(dyn, 0x1A); // DT_FINI_ARRAY
  if (res != NULL)
  {
    void (**fini_array)() = (void (**)())((size_t)base + res->d_un);
    int count = find_dyn_entry(dyn, 0x1C)->d_un / sizeof(size_t); // DT_FINI_ARRAYSZ
    while (*fini_array == NULL && count)
    {
      fini_array++;
      count--;
    }
    if (count)
    {
      LOG("fini array detected:\n");
      for (int i = 0; i < count; i++)
      {
        if (fini_array[i])
        {
          LOG("\t");
          print_hex(fini_array[i]);
          LOG("\n");
        }
      }
    }
  }
  LOG("load_dynamic done.\n");
  return 1;
}

// don't use this function directly, use load_static instead
FUNC int load_static(void *base, const char *buf, elf_header *header)
{
  if (header->e_shentsize != sizeof(elf_section_header))
  {
    LOG("Unexpected section header entry size, skipped load_static\n");
    return 1; // something went wrong, maybe nothing important
  }
  if (header->e_shnum == 0)
  {
    LOG("No section header\n");
    return 1; // maybe it's ok?
  }
  if (header->e_shnum <= header->e_shtrndx)
  {
    LOG("string table index out of range\n");
    return 0;
  }
  elf_section_header sheader;
  // lseek(fd, header->e_shoff + sizeof(elf_section_header) * header->e_shtrndx, SEEK_SET);
  sheader = *((elf_section_header *)(buf + header->e_shoff + sizeof(elf_section_header) * header->e_shtrndx));
  // if (read(fd, &sheader, sizeof(sheader)) != sizeof(sheader)) {
  //	LOG("read section header error\n");
  //	return 0;
  // }
  size_t strtab_size = sheader.s_size;

  // char *strtab = (char *)malloc(strtab_size + 1);
  char *strtab = (char *)mmap(0, strtab_size + 1,
                              PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  // lseek(fd, sheader.s_offset, SEEK_SET);
  // if (read(fd, strtab, strtab_size) != strtab_size) {
  //	LOG("read section header string table error\n");
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
  for (int i = 0; i < header->e_shnum; i++)
  {
    // if (read(fd, &sheader, sizeof(sheader)) != sizeof(sheader)) {
    //	LOG("read section header error\n");
    //	free(strtab);
    //	return 0;
    //}
    sheader = *((elf_section_header *)(buf + header->e_shoff + sizeof(elf_section_header) * i));

    if (sheader.s_size == 0)
      continue;
    if (sheader.s_name >= strtab_size)
    {
      LOG("bad section name\n");
      strtab == NULL;
      // free(strtab);
      return 0;
    }
    if (sheader.s_addr)
    {
      // LOGV("section %d: `%s' (in memory: %p - %p)\n", i, strtab + sheader.s_name, (void*) ((size_t) base + sheader.s_addr), (void*) ((size_t) base + sheader.s_addr + sheader.s_size));
    }
    else
    {
      // LOGV("section %d: `%s' (in file: %p - %p)\n", i, strtab + sheader.s_name, (void*) ((size_t) base + sheader.s_offset), (void*) ((size_t) base + sheader.s_offset + sheader.s_size));
    }
    switch (sheader.s_type)
    {
    case 4: // SHT_RELA
      if (sheader.s_entsize != sizeof(elf_rela))
      {
        LOG("bad rela entry size\n");
        // free(strtab);
        return 0;
      }
      if (sheader.s_size % sizeof(elf_rela) != 0)
      {
        LOG("bad rela size\n");
        // free(strtab);
        return 0;
      }
      LOG("detected rela\n");
      // All items should be R_IRELATIVE
      // Here we just ignore this check
      do_rela(base, (elf_rela *)((size_t)base + sheader.s_addr), sheader.s_size / sizeof(elf_rela), NULL, NULL);
      break;
    case 9: // SHT_REL
      if (sheader.s_entsize != sizeof(elf_rel))
      {
        LOG("bad rel entry size\n");
        // free(strtab);
        return 0;
      }
      if (sheader.s_size % sizeof(elf_rel) != 0)
      {
        LOG("bad rel size\n");
        // free(strtab);
        return 0;
      }
      LOG("detected rel\n");
      // All items should be R_IRELATIVE
      // Here we just ignore this check
      do_rel(base, (elf_rel *)((size_t)base + sheader.s_addr), sheader.s_size / sizeof(elf_rel), NULL, NULL);
      break;
    case 14: // SHT_INIT_ARRAY
      init_array = (void *)((size_t)base + sheader.s_addr);
      init_array_count = sheader.s_size;
      if (init_array_count % sizeof(size_t) != 0)
      {
        LOG("bad init array size\n");
        // free(strtab);
        return 0;
      }
      init_array_count /= sizeof(size_t);
      while (init_array_count && *init_array == NULL)
      {
        init_array++;
        init_array_count--;
      }
      break;
    case 15: // SHT_FINI_ARRAY
      fini_array = (void *)((size_t)base + sheader.s_addr);
      fini_array_count = sheader.s_size;
      if (fini_array_count % sizeof(size_t) != 0)
      {
        LOG("bad init array size\n");
        strtab = NULL;
        // free(strtab);
        return 0;
      }
      fini_array_count /= sizeof(size_t);
      while (fini_array_count && *fini_array == NULL)
      {
        fini_array++;
        fini_array_count--;
      }
      break;
    default:
      break;
    }
  }
  // free(strtab);
  if (init_array && init_array_count)
  {
    LOG("init array detected:\n");
    // int choice = '?';
    for (int i = 0; i < init_array_count; i++)
    {
      if (!init_array[i])
        continue;
      // choice exec
      /*
      while (!init_array_filter && choice != 'y' && choice != 'n' && choice != 'a' && choice != 'o') {
        LOG("\texecute function %p? [(y)es/(n)o/(a)ll items left/n(o)ne items left] ", init_array[i]);
        choice = getchar();
        if (choice != '\n') while (getchar() != '\n') ; // skip line
        if (choice >= 'A' && choice <= 'Z') choice += 0x20; // convert to lower case
      }
      */

      LOG("\texecuting function at ");
      print_hex(init_array[i]);
      init_array[i]();
    }
  }
  if (fini_array && fini_array_count)
  {
    LOG("fini array detected:\n");
    for (int i = 0; i < fini_array_count; i++)
    {
      if (fini_array[i])
      {
        LOG("\t");
        print_hex(fini_array[i]);
        LOG("\n");
      }
    }
  }
  LOG("load_static done.\n");
  return 1;
}

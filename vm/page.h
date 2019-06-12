#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stddef.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "lib/kernel/hash.h"

enum supplemental_pte_type {
  FILE = 1,
  MEMORY_MAPPED_FILE = 2,
};

struct supplemental_pte {
  void *vaddr;
  enum supplemental_pte_type type;
  struct file *file;
  off_t offset;
  size_t size;
  bool loaded;

  struct hash_elem elem;
};

void initialize_supplemental_page_table(struct hash *table);

void add_file(struct hash *table, struct file *file);

int add_memory_mapped_file(struct hash *table, struct file *file);

struct supplemental_pte *get_entry(struct hash *table, void *vaddr);

void remove_memory_mapped_file(struct hash *table, int id);

void unmap_all(struct hash *table);

void free_supplemental_page_table(struct hash *table);


#endif /* vm/page.h */

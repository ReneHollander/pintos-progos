#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stddef.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "lib/kernel/hash.h"

enum spte_type {
  SPTE_TYPE_FILE = 1,
  SPTE_TYPE_MEMORY_MAPPED_FILE = 2,
};

struct spte {
  void *vaddr;
  bool loaded;
  struct file *file;
  off_t offset;
  enum spte_type type;
  union {
    struct {
      uint32_t read_bytes;
      uint32_t zero_bytes;
      bool writable;
    } file_data;
    struct {
      int id;
      uint32_t length;
    } memory_mapped_file_data;
  };

  struct hash_elem elem;
  struct list_elem remove_elem;
};

typedef int spt_action_func (struct spte *e, void *aux);

void spt_init (struct hash *table);

void spt_add_file_entry (struct hash *table, void *vaddr,
        struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable);

void spt_add_memory_mapped_file_entry (struct hash *table, void *vaddr,
        struct file *file, off_t ofs, int id, uint32_t length);

struct spte *spt_get (struct hash *table, void *vaddr);

int spt_iterate_memory_mapped_file_entries (struct hash *table, int id, spt_action_func func, void *aux);

void spt_iterate_all_mmap_entries (struct hash *table, spt_action_func func, void *aux);

struct spte *spt_remove (struct hash *table, void *vaddr);

void spt_free (struct hash *table);

struct munmap_action_data {
  uint32_t *pagedir;
  struct list *to_remove;
  struct file *to_close;
};

int munmap_spt_action_function (struct spte *e, void *aux);

#endif /* vm/page.h */

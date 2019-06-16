#include "page.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel/hash.h"
#include "threads/malloc.h"

static unsigned spt_hash_func (const struct hash_elem *a, __attribute__((unused)) void *aux)
{
  struct spte *ae = hash_entry (a, struct spte, elem);

  return hash_bytes (&ae->vaddr, sizeof (ae->vaddr));
}

static bool spt_hash_less_func (const struct hash_elem *a, const struct hash_elem *b, __attribute__((unused)) void *aux)
{
  struct spte *ae = hash_entry (a, struct spte, elem);
  struct spte *be = hash_entry (b, struct spte, elem);

  return ae->vaddr < be->vaddr;
}

void spt_init (struct hash *table)
{
  hash_init (table, &spt_hash_func, &spt_hash_less_func, NULL);
}

void spt_add_file_entry (struct hash *table, void *vaddr,
    struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable)
{
  struct spte *e = malloc (sizeof (struct spte));
  e->vaddr = vaddr;
  e->file = file;
  e->offset = ofs;
  e->type = SPTE_TYPE_FILE;
  e->file_data.read_bytes = read_bytes;
  e->file_data.zero_bytes = zero_bytes;
  e->file_data.writable = writable;
  e->loaded = false;
  hash_insert (table, &e->elem);
}

void spt_add_memory_mapped_file_entry (struct hash *table, void *vaddr,
    struct file *file, off_t ofs, int id, uint32_t length)
{
  struct spte *e = malloc (sizeof (struct spte));
  e->vaddr = vaddr;
  e->file = file;
  e->offset = ofs;
  e->type = SPTE_TYPE_MEMORY_MAPPED_FILE;
  e->memory_mapped_file_data.id = id;
  e->memory_mapped_file_data.length = length;
  e->loaded = false;
  hash_insert (table, &e->elem);
}

struct spte *spt_get (struct hash *table, void *vaddr)
{
  struct spte needle;
  needle.vaddr = vaddr;
  struct hash_elem *e = hash_find (table, &needle.elem);

  if (e == NULL) {
      return NULL;
  }

  return hash_entry (e, struct spte, elem);
}

/*
 * Don't remove while iterating.
 */
void spt_iterate_memory_mapped_file_entries (struct hash *table, int id, spt_action_func func, void *aux)
{
  struct hash_iterator i;
  hash_first (&i, table);
  while (hash_next (&i)) {
    struct spte *e = hash_entry (hash_cur(&i), struct spte, elem);
    if (e->type == SPTE_TYPE_MEMORY_MAPPED_FILE && e->memory_mapped_file_data.id == id) {
      func(e, aux);
    }
  }
}

void spt_iterate_all_mmap_entries (struct hash *table, spt_action_func func, void *aux)
{
    struct hash_iterator i;
    hash_first (&i, table);
    while (hash_next (&i)) {
        struct spte *e = hash_entry (hash_cur(&i), struct spte, elem);
        if (e->type == SPTE_TYPE_MEMORY_MAPPED_FILE) {
            func(e, aux);
        }
    }
}

struct spte *spt_remove (struct hash *table, void *vaddr)
{
  struct spte *e = spt_get (table, vaddr);
  if (e == NULL) return NULL;
  hash_delete (table, &e->elem);
  return e;
}

static void spt_clear_hash_action_func (struct hash_elem *a, __attribute__((unused)) void *aux)
{
  struct spte *ae = hash_entry (a, struct spte, elem);
  free(ae);
}

void spt_free (struct hash *table)
{
  hash_destroy (table, spt_clear_hash_action_func);
}

#include "page.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "userprog/pagedir.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "kernel/hash.h"
#include "threads/malloc.h"
#include "userprog/process.h"

#ifdef VM

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
    struct file *file, off_t ofs, int id, off_t length)
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
int spt_iterate_memory_mapped_file_entries (struct hash *table, int id, spt_action_func func, void *aux)
{
  struct hash_iterator i;
  hash_first (&i, table);
  while (hash_next (&i)) {
    struct spte *e = hash_entry (hash_cur(&i), struct spte, elem);
    if (e->type == SPTE_TYPE_MEMORY_MAPPED_FILE && e->memory_mapped_file_data.id == id) {
      int res;
      if ((res = func(e, aux)) < 0) {
        return res;
      }
    }
  }
  return 0;
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

int munmap_spt_action_function (struct spte *e, void *aux)
{
  struct munmap_action_data *data = aux;
  data->to_close = e->file;
  // When page is loaded and dirty, write to file.
  if (e->loaded) {
    if (pagedir_is_dirty(data->pagedir, e->vaddr)) {
      if (file_write_at(e->file, e->vaddr, e->memory_mapped_file_data.length, e->offset) != e->memory_mapped_file_data.length) {
        return -1;
      }
    }

    void *kpage = pagedir_get_page (data->pagedir, e->vaddr);
    pagedir_clear_page(data->pagedir, e->vaddr);
    palloc_free_page(kpage);
  }

  if (data->to_remove != NULL) {
    list_push_back(data->to_remove, &e->remove_elem);
  }

  return 0;
}

bool load_page(void *addr) {
    void *upage = pg_round_down(addr);
    struct thread *curr = thread_current ();

    /* Lookup page in spt */
    struct spte *spte = spt_get(&curr->supplemental_page_table, upage);
    if(spte == NULL){
        return false;
    }

    /* Get a page of memory. */
    uint8_t *kpage = palloc_get_page (PAL_USER);
    if (kpage == NULL){
        return false;
    }

    uint32_t read_length = 0;
    bool writable = false;

    if(spte->type == SPTE_TYPE_MEMORY_MAPPED_FILE){
        read_length = spte->memory_mapped_file_data.length;
        writable = true;
    } else if(spte->type == SPTE_TYPE_FILE){
        read_length = spte->file_data.read_bytes;
        writable = spte->file_data.writable;
    } else {
        return false;
    }

    /* Load data into page. */
    if (file_read_at (spte->file, kpage, read_length, spte->offset) != (int) read_length) {
        palloc_free_page (kpage);
        return false;
    }

    if(spte->type == SPTE_TYPE_FILE) {
        memset(kpage + spte->file_data.read_bytes, 0, spte->file_data.zero_bytes);
    }

    /* Map the user page to the allocated kernel page */
    if (!install_page (upage, kpage, writable)) {
        palloc_free_page (kpage);
        return false;
    }

    spte->loaded = true;

    /* loading successful */
    return true;
}

#endif
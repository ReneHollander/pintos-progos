#include "page.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel/hash.h"

static unsigned supplemental_pte_hash_func(const struct hash_elem *a, void *aux) {
  struct supplemental_pte *ae = hash_entry(a, struct supplemental_pte, elem);

  return hash_bytes(&ae->vaddr, sizeof(ae->vaddr));
}

static bool supplemental_pte_hash_less_func(const struct hash_elem *a, const struct hash_elem *b, void *aux) {
  struct supplemental_pte *ae = hash_entry(a, struct supplemental_pte, elem);
  struct supplemental_pte *be = hash_entry(b, struct supplemental_pte, elem);

  return ae->vaddr < ae->vaddr;
}

void initialize_supplemental_page_table(struct hash *table) {
  hash_init(table, &supplemental_pte_hash_func, &supplemental_pte_hash_less_func, NULL);
}

void add_file(struct hash *table, struct file *file) {

}

int add_memory_mapped_file(struct hash *table, struct file *file) {
  return 0;
}

struct supplemental_pte *get_entry(struct hash *table, void *vaddr) {
  return NULL;
}

void remove_memory_mapped_file(struct hash *table, int id) {

}

static void unmap(struct supplemental_pte *e) {

}

static void unmap_hash_action_func(struct hash_elem *a, void *aux) {
  struct supplemental_pte *ae = hash_entry(a, struct supplemental_pte, elem);
  unmap(ae);
}

void unmap_all(struct hash *table) {
  hash_clear(table, &unmap_hash_action_func);
}

void free_supplemental_page_table(struct hash *table) {
  hash_destroy(table, NULL);
}

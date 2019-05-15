#ifndef __LIB_KERNEL_STACK_H
#define __LIB_KERNEL_STACK_H

#include <stdbool.h>
#include <stdint.h>

#define STACK_SIZE (4092)

struct stack
  {
    uint32_t top;
    uint8_t elements[STACK_SIZE];
  };

void stack_init (struct stack *);

bool stack_is_empty (struct stack *);

uint32_t stack_size (struct stack *);

bool stack_push (struct stack *, uint8_t);

bool stack_pop (struct stack *, uint8_t *);

#endif /* lib/kernel/stack.h */

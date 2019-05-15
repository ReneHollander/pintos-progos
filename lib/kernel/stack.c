#include "stack.h"

void
stack_init (struct stack *stack)
{
  stack->top = -1;
}

bool
stack_is_empty (struct stack *stack)
{
  return stack_size(stack) == 0;
}

uint32_t
stack_size (struct stack *stack)
{
  return stack->top + 1;
}

bool
stack_push (struct stack *stack, uint8_t element)
{
  if (stack->top + 1 >= STACK_SIZE) {
    return false;
  }
  stack->elements[++stack->top] = element;
  return true;
}

bool
stack_pop (struct stack *stack, uint8_t *element)
{
  if (stack_is_empty(stack)) {
    return false;
  }
  *element = stack->elements[stack->top--];
  return true;
}

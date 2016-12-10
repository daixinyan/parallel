#include "include.h"


Stack* createStack(int size)
{
  Stack* stack = (Stack*)malloc(sizeof(Stack));
  stack->queue = (TYPE*)malloc(sizeof(TYPE)*size);
  stack->max_size = size;
  stack->size = 0;
  return stack;
}
void freeStatck(Stack* stack)
{
  free(stack->queue);
  free(stack);
}
TYPE pop(Stack* stack)
{
  stack->size = stack->size - 1;
  return stack->queue[stack->size];
}
void push(Stack* stack, TYPE elem)
{
  // if(stack->size==stack->max_size)
  // {
  //   TYPE* list = (TYPE*)malloc(sizeof(TYPE)*(stack->max_size*2+1));
  //   memcpy(list, stack->queue, sizeof(TYPE)/sizeof(char));
  //   free(stack->queue);
  //   stack->queue = list;
  // }
  stack->queue[stack->size] = elem;
  stack->size++;
}

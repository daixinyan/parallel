typedef int TYPE;
typedef struct SizeLimitedStack{
  int size;
  TYPE max_size;
  TYPE* queue;
}Stack;

Stack* createStack(int size);
void freeStatck(Stack* stack);
TYPE pop(Stack* stack);
TYPE push(Stack* stack, TYPE elem);
void clear(Stack* stack);

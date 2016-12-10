typedef struct SizeLimitedStack{
  int size;
  int max_size;
  int* queue;
}Stack;
Stack* create(int size);
int pop(Stack* stack);
void push(Stack* stack, int elem);

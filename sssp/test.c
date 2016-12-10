#include "include.h"

int main()
{
  int command;
  int input;
  Stack* stack = createStack(20);
  while(1)
  {
    printf("input command 0 for pop 1 for push\n" );
    scanf("%d\n", &command);
    if(command==1)
    {
      scanf("%d\n",&input);
      push(stack, input);
      printf("push one data%d\n", input);
    }
    else if(command==0)
    {
      if(stack->size>0)
      {
        printf("pop one data%d\n", pop(stack));
      }
    }else {
      printf("no format\n");
    }
  }
}

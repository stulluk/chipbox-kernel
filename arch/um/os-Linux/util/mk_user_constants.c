#include <stdio.h>
#include <user-offsets.h>

int main(int argc, char **argv)
{
  printf("/*\n");
  printf(" * Generated by mk_user_constants\n");
  printf(" */\n");
  printf("\n");
  printf("#ifndef __UM_USER_CONSTANTS_H\n");
  printf("#define __UM_USER_CONSTANTS_H\n");
  printf("\n");
  /* I'd like to use FRAME_SIZE from ptrace.h here, but that's wrong on
   * x86_64 (216 vs 168 bytes).  user_regs_struct is the correct size on
   * both x86_64 and i386.
   */
  printf("#define UM_FRAME_SIZE %d\n", __UM_FRAME_SIZE);

  printf("\n");
  printf("#endif\n");

  return(0);
}


#include <stdio.h>

main()
{
  int i;

  for (i=0; i<10000; i++)
	fputc(0, stdout);

}

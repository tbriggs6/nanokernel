
int main(int argc, char **argv)
{
   int a[16] =  { 0 };
 
   int x = 0, j = 0;

   while (1) {
     a[x % 16] = j;
     for (x = 0; x < j + a[x%16]; x++)  {
	asm("nop;");
     }
     a[x % 16] = j;

     j++;
   }
}
    
void _start(void) {
  main(0, (void *)0);
}


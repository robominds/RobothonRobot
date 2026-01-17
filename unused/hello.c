
int main(void) {
  int *i;

  i = (int*)0x2000;

  (*i) = 0x1000;

  while((*i)<0x8000) {
    (*i)++;
  }
  while(1);
}

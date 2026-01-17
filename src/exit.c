
void exit(int e) { 
  e++; 
  while(1) ;
}

void atexit(int e) {
  exit(e);
}

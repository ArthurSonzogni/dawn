
int f(int i) {
  int a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  return a[i];
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}


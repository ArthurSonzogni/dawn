SKIP: INVALID


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  vector<float16_t, 3> b = a;
}


SKIP: FAILED


static uint3 u = (1u).xxx;
void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
c:\src\dawn\Shader@0x000001CB048F3760(4,10-18): error X3000: syntax error: unexpected token 'float16_t'


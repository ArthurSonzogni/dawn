SKIP: FAILED


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  vector<float16_t, 2> b = a;
}

FXC validation failure:
c:\src\dawn\Shader@0x000001BDA2D03780(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
c:\src\dawn\Shader@0x000001BDA2D03780(5,10-18): error X3000: syntax error: unexpected token 'float16_t'


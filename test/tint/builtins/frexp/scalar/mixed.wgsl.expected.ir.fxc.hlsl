SKIP: FAILED

struct frexp_result_f32 {
  float fract;
  int exp;
};


[numthreads(1, 1, 1)]
void main() {
  float runtime_in = 1.25f;
  frexp_result_f32 res = {0.625f, 1};
  frexp_result_f32 v = frexp(runtime_in);
  res = v;
  frexp_result_f32 v_1 = {0.625f, 1};
  res = v_1;
  float fract = res.fract;
  int exp = res.exp;
}

FXC validation failure:
c:\src\dawn\Shader@0x000001AC38D23940(11,24-40): error X3013: 'frexp': no matching 1 parameter intrinsic function
c:\src\dawn\Shader@0x000001AC38D23940(11,24-40): error X3013: Possible intrinsic functions are:
c:\src\dawn\Shader@0x000001AC38D23940(11,24-40): error X3013:     frexp(float|half, out float|half exp)


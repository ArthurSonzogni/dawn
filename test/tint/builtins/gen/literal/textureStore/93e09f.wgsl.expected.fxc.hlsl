//
// fragment_main
//

RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_93e09f() {
  arg_0[(1u).xx] = (int(1)).xxxx;
}

void fragment_main() {
  textureStore_93e09f();
}

//
// compute_main
//

RWTexture2D<int4> arg_0 : register(u0, space1);
void textureStore_93e09f() {
  arg_0[(1u).xx] = (int(1)).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_93e09f();
}


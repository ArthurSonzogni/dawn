
RWByteAddressBuffer a : register(u0);
void main() {
  a.Store(4u, (a.Load(4u) - 1u));
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}


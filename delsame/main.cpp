// delsame.cpp - kill file if same as a.song

#include "../../stv/os.h"

char Buf [8*1024*1024],  Bf2 [8*1024*1024];

int main (int argc, char *argv [])
{ File  f;
  TStr  fn, f2;
  ulong sz;
   App.Init (CC("pcheetah"), CC("delsame"), CC("delsame"));  
   if (argc < 2)  return 99;
   StrCp (fn, argv [1]);
   StrCp (f2, fn);   Fn2Path (f2);   StrAp (f2, CC("/a.song"));
   if (f.Size (fn) == f.Size (f2)) {
      sz = f.Load (fn, Buf, sizeof (Buf));
           f.Load (f2, Bf2, sizeof (Bf2));
      if (! MemCm (Buf, Bf2, sz))  f.Kill (fn);
   }
   return 0;
}

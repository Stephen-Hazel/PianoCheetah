// UnSYX.c - suck in a MOD, save out somethin USABle:  WAVs and MIDi

#include "os.h"

ubyte Buf [1024*1024];                 // they max out at 66K

int Go ()
{ TStr  fn, n;
  File  f;
  ulong ln, i, j, r = 0;
DBG("{ UnSYX Go");

   StrCp (fn, "c:\\_");
   if (! f.AskR (fn, "which .syx file?"))  return 99;

   ln = f.Load (fn, Buf, sizeof (Buf));
   i = 15;
   do {
      MemCp (n, & Buf [i], 16);  n [16] = '\0';
      for (j = 0;  j < 16;  j++)  if (n [j] == ' ')  n [j] = '_';
      j = StrLn (n);   while (j-- && (n [j] == '_'))  n [j] = '\0';
DBG("`<16s `>3d . .", n, r);
      r++;   i += 526;
   } while (i < ln);

DBG("} UnSYX Go");
   return 0;
}

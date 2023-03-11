// midimp.cpp - midi import

#include "../../stv/os.h"

bool Did, Did2;
TStr DirF, DirT;

void FixFn (char *s)
// do merge check and tack on a number if needed
{ TStr  t;
  char *p;
  ubyt4 n;
  FDir  d;
   StrCp (t, DirT);   StrAp (t, CC("/"));   StrAp (t, s);
   if (d.Got (t)) {
      StrAp (s, CC("_2"));   StrAp (t, CC("_2"));
      while (d.Got (t)) {
         for (p = & s [StrLn (s)-2];  *p != '_';  p--)  ;
         n = Str2Int (p+1);   StrFmt (p+1, "`d", ++n);
         StrCp (t, DirT);   StrAp (t, CC("/"));   StrAp (t, s);
      }
   }
//DBG(s);
}


char *Redo (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
{ ubyt4 ln;                            // re Mid2Song each mid
  ubyt2 i, lln;
  TStr  to, dr, cmd, ls [256];
  File  f;
  FDir  d;
   (void)len;   (void)pos;   (void)ptr;
   ln = StrLn (fn);   if (MemCm (& fn [ln-8], CC("orig."), 5))  return nullptr;

// kill any .songs in dir
   StrFmt (to, "`s/`s", DirT, fn);
   StrCp (dr, to);   Fn2Path (dr);   lln = d.FLst (dr, ls, BITS (ls));
   for (i = 0;  i < lln;  i++) {
      ln = StrLn (ls [i]);
      if (! StrCm (& ls [i][ln-5], CC(".song")))
         {StrFmt (to, "`s/`s", dr, ls [i]);   f.Kill (to);}
   }
   StrFmt (to, "`s/`s", DirT, fn);
   StrCp (cmd, CC("mid2song "));   StrAp (cmd, to);   App.Run (cmd);
   return nullptr;
}


char *Move (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
// move and Mid2Song each mid
{ TStr fr, to, fnx, ext, cmd;
  File f;
   (void)len;   (void)pos;   (void)ptr;
   StrCp (fnx, fn);
   StrCp (ext, & fnx [StrLn (fnx)-4]);   StrAp (fnx, CC(""), 4);;
   FixFn (fnx);
   Did = true;
   StrFmt (fr, "`s/`s",        DirF, fn);
   StrFmt (to, "`s/`s/orig`s", DirT, fnx, ext);
   StrCp (cmd, CC("mid2song "));   StrAp (cmd, to);
   f.Copy (fr, to);   f.Kill (fr);   App.Run (cmd);
   return nullptr;
}


char *Wipe (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
{ ubyte i;
  TStr  fr, to, fnx;
  File  f;                             // move flattened fn to midi_junk dir
   (void)len;   (void)pos;   (void)ptr;
   Did2 = true;
   StrCp (fnx, fn);
   for (i = 0;  i < StrLn (fnx);  i++)  if (fnx [i] == '/')  fnx [i] = '_';
   StrFmt (fr, "`s/`s", DirF, fn);
   StrFmt (to, "`s/`s", DirT, fnx);   f.ReNm (fr, to);   return nullptr;
}


int main (int argc, char *argv [])
{ TStr c, s;
  File f;
  FDir d;
   (void)argc;   (void)argv;
   App.Init (CC("pcheetah"), CC("midimp"), CC("midimp"));
   App.Path (DirF, 'd');   StrCp (DirT, DirF);
DBG("midimp bgn");
// from ..pianocheetah/midi_import to ..pianocheetah/4_queue
   StrAp (DirF, CC("/midi_import"));
   StrAp (DirT, CC("/4_queue"));

// any 4_queue/redo.txt signals
//    ll every .mid(.rmi,etc) in 4_queue dir into midi cache n re-Mid2Song em
   StrFmt (s, "`s/redo.txt", DirT);
   if (f.Size (s)) {
DBG("midimp 4_queue redo bgn");
      f.Kill (s);
      StrCp (c, CC("ll midi "));   StrAp (c, DirT);   App.Run (c);
      StrCp (s, DirT);   StrAp (s, CC("/_midicache.txt"));
      f.DoText (s, nullptr, Redo);
      f.Kill (s);                      // cleanup cache
DBG("midimp 4_queue redo end");
   }

// list midi files in midi_import n move+mid2song em
   StrCp (c, CC("ll midi "));   StrAp (c, DirF);   App.Run (c);
   StrCp (s, DirF);   StrAp (s, CC("/_midicache.txt"));
   f.DoText (s, nullptr, Move);

// list off leftovers n move to midi_junk
   StrCp (c, CC("ll alll "));   StrAp (c, DirF);   App.Run (c);
   Fn2Path (DirT);   StrAp (DirT, CC("/midi_junk"));   d.Make (DirT);
   StrCp (s, DirF);   StrAp (s, CC("/_cache.txt"));
   f.DoText (s, nullptr, Wipe);
   if (! Did2)  d.Kill (DirT);         // kill it if didn't put nothin in
   d.Kill (DirF);   d.Make (DirF);     // kill n remake DirF so left empty
DBG("midimp end");
   return 0;
}

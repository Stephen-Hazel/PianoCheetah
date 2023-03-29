// midimp.cpp - midi import

#include "../../stv/os.h"

bool Did;
TStr DirF, DirT;

void FixFn (char *s)
// do merge check and tack on a number if needed
{ TStr  t;
  char *p;
  ubyt4 n;
  FDir  d;
   StrCp (t, DirT);   StrAp (t, CC("/"));   StrAp (t, s);
   if (d.Got (t)) {
      StrAp (t, CC("_2"));   StrAp (s, CC("_2"));
      while (d.Got (t)) {              // gotta return s but test w t
         for (p = & s [StrLn (s)-2];  *p != '_';  p--)  ;
         n = Str2Int (p+1);   StrFmt (p+1, "`d", ++n);
         StrCp (t, DirT);   StrAp (t, CC("/"));   StrAp (t, s);
      }
   }
//DBG(s);
}


char *Move (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
// move and Mid2Song each mid.  renamin non .mid to .mid
{ TStr fr, to, fnx, c;
  File f;
   (void)len;   (void)pos;   (void)ptr;
// take off .ext, FixFn does merge check
   StrCp (fnx, fn);   StrAp (fnx, CC(""), 4);   FixFn (fnx);
   StrFmt (fr, "`s/`s",       DirF, fn);
   StrFmt (to, "`s/`s/a.mid", DirT, fnx);
// move n mid2song
   f.ReNm (fr, to);   App.Run (StrFmt (c, "mid2song '`s'", to));
   return nullptr;
}


char *Wipe (char *fn, ubyt2 len, ubyt4 pos, void *ptr)
{ ubyte i;
  TStr  fr, to, fnx;
  File  f;                             // move flattened fn to midi_junk dir
   (void)len;   (void)pos;   (void)ptr;
   Did = true;                         // settin this leave junk dir there
   StrCp (fnx, fn);
   for (i = 0;  i < StrLn (fnx);  i++)  if (fnx [i] == '/')  fnx [i] = '_';
   StrFmt (fr, "`s/`s", DirF, fn);
   StrFmt (to, "`s/`s", DirT, fnx);   f.ReNm (fr, to);
   return nullptr;
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

// list midi files in midi_import n move+mid2song em
   StrCp (c, CC("ll midi "));   StrAp (c, DirF);   App.Run (c);
   StrCp (s, DirF);   StrAp (s, CC("/_midicache.txt"));
   f.DoText (s, nullptr, Move);

// list off leftovers n move to midi_junk
   StrCp (c, CC("ll alll "));   StrAp (c, DirF);   App.Run (c);
   Fn2Path (DirT);   StrAp (DirT, CC("/midi_junk"));   d.Make (DirT);
   StrCp (s, DirF);   StrAp (s, CC("/_cache.txt"));
   f.DoText (s, nullptr, Wipe);
   if (! Did)  d.Kill (DirT);          // kill it if didn't put nothin in
   d.Kill (DirF);   d.Make (DirF);     // kill n remake DirF so left empty
DBG("midimp end");
   return 0;
}

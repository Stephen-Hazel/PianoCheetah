// midimp.cpp - midi import

#include "../../stv/os.h"

bool Did, Did2;
TStr DirF, DirT;

bool FixFn (char *s)
// clean up genre/group/title of a .mid file
// _pop/^/asdf_a_jkl => _pop/asdf/_a_jkl  or  asdfjkl => _pop/_/asdfjkl
// kill _x_ rating n _lyr suffix n any ' or `
// init cap
// non rating ___x => X  spacespacespacex => X
// kill group outa title if doesnt leave it too short
// kill 1-9 suffix if no prev #
// do merge check and number if needed
{ TStr  ge, gr, t;
  char *p;
  bool  got;
  ubyt4 n;
  FDir  d;
                StrCp (ge, s);     if (! (p = StrCh (ge, '/')))  return false;
   *p = '\0';   StrCp (gr, ++p);   if (! (p = StrCh (gr, '/')))  return false;
   *p = '\0';   StrCp (t,  ++p);
   if (! StrCm (gr, CC("^"))) {
      StrCp (gr, t);
      if      ((p = StrCh (gr, '_')))
         {*p = '\0';   StrCp (t, & t [StrLn (gr)]);}
      else if ((p = StrCh (gr, '-')))
         {*p = '\0';   StrCp (t, & t [StrLn (gr)]);}
      else if ((p = StrCh (gr, ' ')))
         {*p = '\0';   StrCp (t, & t [StrLn (gr)]);}
      else             StrCp (gr, CC("_"));
   }
   if (! MemCm (t, CC("_x_"), 3))  StrCp (t, t+3);
   if ( (StrLn (t) > 4) && (! StrCm (& t [StrLn (t)-4], CC("_lyr"))) )
                       StrAp (t, CC(""), 4);
   p = gr;
   *p = CHUP (*p);
   while (*p) {
      while ((*p == '`') || (*p == '\'') ||
             (*p == '(') || (*p == ')'))  StrCp (p, p+1);
      got = false;
      while ((*p == '_') || (*p == ' ') ||
                            (*p == '-'))  {got = true;   StrCp (p, p+1);}
      if (got)  {if (*p)  *p = CHUP (*p);}   else  p++;
   }
   p = ((t [0] == '_') && (t [2] == '_')) ? t+3 : t;
   *p = CHUP (*p);
   while (*p) {
      while ((*p == '`') || (*p == '\'') ||
             (*p == '(') || (*p == ')'))  StrCp (p, p+1);
      got = false;
      while ((*p == '_') || (*p == ' ') ||
                            (*p == '-'))  {got = true;   StrCp (p, p+1);}
      if (got)  {if (*p)  *p = CHUP (*p);}   else  p++;
   }
   if ( (StrLn (t) > (StrLn (gr)+4)) && (p = StrSt (t, gr)) )
      StrCp (p, p + StrLn (gr));
   p = & t [StrLn (t)-1];
   if ( (*p >= '1') && (*p <= '9') && (! CHNUM (*(p-1))) )
      StrAp (t, CC(""), 1);
   StrFmt (s, "`s/`s/`s", ge, gr, t);
   StrCp       (t, DirT);   StrAp (t, CC("/"));   StrAp (t, s);
   if (d.Got (t)) {
      StrAp (s, CC("_2"));   StrAp (t, CC("_2"));
      while (d.Got (t)) {
         for (p = & s [StrLn (s)-2];  *p != '_';  p--)  ;
         n = Str2Int (p+1);   StrFmt (p+1, "`d", ++n);
         StrCp (t, DirT);   StrAp (t, CC("/"));   StrAp (t, s);
      }
   }
//DBG(s);
   return true;
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
{ TStr fr, to, fnx, ext, cmd;
  File f;                              // move and Mid2Song each mid
   (void)len;   (void)pos;   (void)ptr;
   StrCp (fnx, fn);
   StrCp (ext, & fnx [StrLn (fnx)-4]);   StrAp (fnx, CC(""), 4);;
   if (FixFn (fnx)) {
      Did = true;
      StrFmt (fr, "`s/`s",        DirF, fn);
      StrFmt (to, "`s/`s/orig`s", DirT, fnx, ext);
      StrCp (cmd, CC("mid2song "));   StrAp (cmd, to);
      f.Copy (fr, to);   f.Kill (fr);   App.Run (cmd);
   }
   else  DBG("bad fn `s", fnx);
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
   StrAp (DirF, CC("/midi_import"));
   StrAp (DirT, CC("/4_queue"));

   StrFmt (s, "`s/redo.txt", DirF);
   if (f.Size (s)) {
      f.Kill (s);
      StrCp (c, CC("ll midi "));   StrAp (c, DirT);   App.Run (c);
      StrCp (s, DirT);   StrAp (s, CC("/_midicache.txt"));
      f.DoText (s, nullptr, Redo);
      f.Kill (s);
   }

// build cache n SLMove
   StrCp (c, CC("ll midi "));   StrAp (c, DirF);   App.Run (c);
   StrCp (s, DirF);   StrAp (s, CC("/_midicache.txt"));
   f.DoText (s, nullptr, Move);

// list off leftovers n move to midi_junk
   StrCp (c, CC("ll alll "));   StrAp (c, DirF);   App.Run (c);
   Fn2Path (DirT);   StrAp (DirT, CC("/midi_junk"));   d.Make (DirT);
   StrCp (s, DirF);   StrAp (s, CC("/_cache.txt"));
   f.DoText (s, nullptr, Wipe);
   if (! Did2)  d.Kill (DirT);         // kill it if didn't put nothin in

// kill n remake DirF so left empty
   d.Kill (DirF);   d.Make (DirF);

// recache 4_queue songs for pc
   App.Path (DirT, 'd');   StrAp (DirT, CC("/4_queue"));
   StrCp (c, DirT);   StrAp (c, CC("/_songcache.txt"));
   if (Did || (! f.Size (c)))
      {StrCp (c, CC("ll song "));   StrAp (c, DirT);   App.Run (c);}
   return 0;
}

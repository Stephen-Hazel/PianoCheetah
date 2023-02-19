// ll.cpp - list long like unix (but no sortin, only midi)

#include "../../stv/os.h"

TStr Ext, Top;
bool  Q;
ulong N;
File  F;

void DoDir (char *dir)
{ FDir d;
  TStr fn, a;
  char df;
   if ((df = d.Open (fn, dir))) {
      do {
         if (df == 'f') {
           ulong ln = StrLn (fn);      // only care bout midi files
            if      (! StrCm (Ext, CC("midi"))) {
               if ( (ln > 4) && ((! StrCm (& fn [ln-4], CC(".mid"))) ||
                                 (! StrCm (& fn [ln-4], CC(".kar"))) ||
                                 (! StrCm (& fn [ln-4], CC(".rmi")))) )
                  {F.Put (& fn [StrLn (Top)+1]);   F.Put (CC("\n"));   N++;}
            }
            else if (! StrCm (Ext, CC("song"))) {
               if ( (ln > 7) && (! StrCm (& fn [ln-7], CC("/a.song"))) )
                  {StrCp (a, fn);   Fn2Path (a);
                   F.Put (& a  [StrLn (Top)+1]);   F.Put (CC("\n"));   N++;}
            }
            else                // "alll"
               if ( (ln < 10) || StrCm (& fn [ln-9], CC("cache.txt")) )
                  {F.Put (& fn [StrLn (Top)+1]);   F.Put (CC("\n"));   N++;}
            if (Q && (N >= 100))  {d.Shut ();   return;}
         }
         else  DoDir (fn);
      } while ((df = d.Next (fn)));
      d.Shut ();
   }
}


int main (int argc, char *argv [])
{ ulong p;
  TStr  fn, to;
   if (argc < 3)  return 99;
   App.Init (CC("pcheetah"), CC("ll"), CC("ll"));
   StrCp (Ext, argv [1]);   StrCp (Top, argv [2]);
   if (! StrCm (Ext, CC("songq")))  {Q = true;   StrCp (Ext, CC("song"));}

// make temp out file n fill it
   p = StrLn (Top)+1;   if (! StrCm (Ext, CC("alll")))  *Ext = '\0';
   StrFmt (fn, "`s/__`scache.txt", Top, Ext);
   if (F.Open (fn, "w"))  {DoDir (Top);   F.Shut ();}

// replace old cache
   StrCp (to, fn);   StrCp (& to [p], & to [p+1]);
   if (F.Size (to))  F.Kill (to);
   F.ReNm (fn, to);   return 0;
}

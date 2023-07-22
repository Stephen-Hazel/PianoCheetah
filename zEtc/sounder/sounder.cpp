// sounder.cpp - devtype\sound.txt maker
// load progch bankhi banklo CustomSoundname
// save gmdir\gmname\custom_soundname progch bankhi banklo

#include <os.h>
#include <MidiIO.h>

ubyte grp [16] = {                     // gm sound group order
    0, // Piano
    2, // Organ
    1, // ChromPerc
   10, // SynLead
   11, // SynPad
    3, // Guitar
    5, // SoloStr
    6, // Ensemble
    7, // Brass
    8, // Reed
    9, // Pipe
   13, // Ethnic
   12, // SynFX
    4, // Bass
   14, // Perc
   15  // SndFX
};

void Cvt ()
{ File  f;
  TStr  fni, fno, ts, ps;
  ulong r, i;
  ubyte c, g, p;
  char *ch;
  bool  adj [3];
  STable t;
   App.Path (fni, 'd');   StrAp (fni, "\\device");
   f.AskR (fni, "where's your .txt file of patches?");
   t.Load (fni, "-", 4);

   StrCp (fno, fni);   Fn2Path (fno);   StrAp (fno, "\\sound.txt");
   if (! StrCm (fni, fno))
      Die ("I'm gonna write a sound.txt file "
           "so DON'T name your input file that!");

// see if any cols need to go from 1 to 0 based (have a 128)
   MemSet (adj, 0, sizeof (adj));
   for (r = 0; r < t.NRow (); r++)  for (c = 0; c < 3; c++)  if (! c)
         if (Str2Int (t.Get (r, c)) == 128)                      adj [c] = true;
   for (r = 0; r < t.NRow (); r++)  for (c = 0; c < 3; c++)  if (adj [c])
      {i = Str2Int (t.Get (r, c));   StrFmt (ts, "`d", i-1);
                    t.Upd (r, c, ts);}

// ok, write out sound.txt prog by prog...
   if (! f.Open (fno, "w"))  Die ("can't write file :(", fno);
   for (g = 0; g < BITS (grp); g++)  for (p = g*8; p < g*8+8; p++) {
      for (r = 0; r < t.NRow (); r++)  if (Str2Int (t.Get (r, 0)) == p) {
         StrCp (ps, MProg [p]);   StrAp (ps, "\\");   StrAp (ps, t.Get (r, 3));
         while (ch = StrCh (ps, ' '))  *ch = '_';     // any spaces to _
         StrFmt (ts, "`<40s `>3d `>3d `>3d\r\n",
                 ps, Str2Int (t.Get (r, 0)), Str2Int (t.Get (r, 1)),
                                             Str2Int (t.Get (r, 2)) );
         f.Put (ts);
      }
   }
   f.Shut ();
}


int Go ()  {TRC("{ Go");   Cvt ();   TRC("} Go");   return 0;}

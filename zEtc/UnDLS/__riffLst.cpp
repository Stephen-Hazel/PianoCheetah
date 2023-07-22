// dlsLst.cpp - list .dls contents

#include "ui.h"

#define EVEN(n)     ((n)&0xFFFFFFFE)
#define EVEN_UP(n)  EVEN((n)+1)

ubyte B [16*1024*1024];
ulong L;

typedef struct {char id [4];  ulong sz;} ChDef;

void Load ()
{ ulong p;
  ChDef ch;
  char  spot [80+1], id [5], out [8000];
  ulong endp [80];

// check header for RIFF, DLS
   if (memcmp (& B [0], "RIFF", 4))  throw Excep ("not a RIFF file");
   memcpy (id, & B [8], 4);  id [4] = '\0';
   sprintf (out, "FORM='%s'\n\n", id);  DBG (out);

// start parsin chunks
   for (*spot = '\0', p = 12;  (p + 8) <= L;) {
   // get chunk
      memcpy (& ch, & B [p], 8);    p += 8;

   // see if LIST outa scope
      while (strlen (spot) && (p >= endp [strlen (spot) - 1]))
         spot [strlen (spot) - 1] = '\0';
      if (memcmp (ch.id, "LIST", 4) == 0) { // LISTs...
         if (p+4 > L) 
            throw Excep ("bad LIST chunk");
         memcpy (id, & B [p], 4);  id [4] = '\0';  p += 4;
         sprintf (out, "%s%s\n", spot, id);  DBG (out);
         endp [strlen (spot)] = p + ch.sz - 4;
         strcat (spot, " ");
      }
      else { // chunk...
         if ((p + EVEN_UP (ch.sz)) > L)
            throw Excep ("bad chunk size");
         memcpy (id, ch.id, 4);  id [4] = '\0';
         sprintf (out, "%s%s\n", spot, id);  DBG (out);
         p += EVEN_UP (ch.sz);
      }
   }
}

int Go ()
{ char fn [MAX_PATH];
  File f;
   strcpy (fn, App.parm);  // "c:\\_\\Src\\Ditty\\_dlsLst\\_new1.dls");
   L = f.Load (fn, B, sizeof (B));
   if ((L == 0) || (L == sizeof (B)))
      throw Excep ("file has len=0 or is TOO BIG");

   Load ();

   return 0;
}

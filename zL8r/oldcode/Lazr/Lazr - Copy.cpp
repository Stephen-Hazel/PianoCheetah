// Lazr.cpp - control a lazzzzerr with PianoCheetah

#include "rc\resource.h"
#include "ui.h"
#include "EthDrm.h"
#include "FrmTmr.h"
#include "math2.h"


TStr  FN [3000];   ulong NFN, XFN;
ubyte Ild [128*1024*1024];             // mem to hold the whole .Ild file
ulong IldLn;                           // len of buf

Color Clr [65536], ClrInit [] = {
   {255,   0,   0},  {255,  16,   0},  {255,  32,   0},  {255,  48,   0},
   {255,  64,   0},  {255,  80,   0},  {255,  96,   0},  {255, 112,   0},
   {255, 128,   0},  {255, 144,   0},  {255, 160,   0},  {255, 176,   0},
   {255, 192,   0},  {255, 208,   0},  {255, 224,   0},  {255, 240,   0},
   {255, 255,   0},  {224, 255,   0},  {192, 255,   0},  {160, 255,   0},
   {128, 255,   0},  { 96, 255,   0},  { 64, 255,   0},  { 32, 255,   0},
   {  0, 255,   0},  {  0, 255,  36},  {  0, 255,  73},  {  0, 255, 109},
   {  0, 255, 146},  {  0, 255, 182},  {  0, 255, 219},  {  0, 255, 255},
   {  0, 227, 255},  {  0, 198, 255},  {  0, 170, 255},  {  0, 142, 255},
   {  0, 113, 255},  {  0,  85, 255},  {  0,  56, 255},  {  0,  28, 255},
   {  0,   0, 255},  { 32,   0, 255},  { 64,   0, 255},  { 96,   0, 255},
   {128,   0, 255},  {160,   0, 255},  {192,   0, 255},  {224,   0, 255},
   {255,   0, 255},  {255,  32, 255},  {255,  64, 255},  {255,  96, 255},
   {255, 128, 255},  {255, 160, 255},  {255, 192, 255},  {255, 224, 255},
   {255, 255, 255},  {255, 224, 224},  {255, 192, 192},  {255, 160, 160},
   {255, 128, 128},  {255,  96,  96},  {255,  64,  64},  {255,  32,  32}
};
ulong NClr;
uword                           Frm [65536];          uword NFrm, Rep;
struct {uword x, y;  Color c;}  Dot [16*1024*1024];   ulong NDot;
ulong Ixmn, Ixmx, Iymn, Iymx;

void LoadIld (char *fn)
{ File  f;
  ulong p;
  uword d, nd, dw, fr;                 // dot#, num dots, detail width, frame#
  ubyte fmt;
  TStr  sfr, sco;
  sword x, y;
  Color c;
  ubyte *fp;
   NFrm = 0;   NDot = 0;   Rep = 1;   Ixmn = Iymn = 65535;   Ixmx = Iymx = 0;
   MemCp (Clr, ClrInit, sizeof (ClrInit));   NClr = BITS (ClrInit);
   if ((IldLn = f.Load (fn, Ild, sizeof (Ild))) == 0)
                             {DBG ("`s not found", fn);   return;}
   if (IldLn >= BITS (Ild))  {DBG ("`s too big", fn);     return;}
   p = 0;
   do {
   // header parsin
      fp = & Ild [p];
      if (MemCm ((char *)fp, "ILDA", 4))
         {DBG("hdr not ILDA");   NFrm = 0;   NDot = 0;   return;}
      fmt = fp [7];
      MemCp (sfr, & fp [ 8], 8);   sfr [8] = '\0';
      MemCp (sco, & fp [16], 8);   sco [8] = '\0';
      nd = (fp [24]<<8) | fp [25];   fr = (fp [26]<<8) | fp [27];
                                   NFrm = (fp [28]<<8) | fp [29];
DBG("fmt=`d fr_name=`s co_name=`s ndot=`d frm=`d nfrm=`d proj=`d",
fmt, sfr, sco, nd, fr, NFrm, fp [30]);
      p += 32;

      switch (fmt) {
         case 0: dw =  8;   break;     case 1: dw = 6;   break;
         case 2: dw =  3;   break;
         case 4: dw = 10;   break;     case 5: dw = 8;   break;
         default:  DBG("bad .Ild format=`d", fmt);   NFrm = 0;   NDot = 0;
                   return;
      }
      if (fmt == 3) NClr = 0;   else {Frm [fr] = nd;}

   // detail recs (dots / colors)
      for (d = 0;  d < nd;  d++, p += dw)  {
         fp = & Ild [p];
         if (fmt == 2)
              {Clr [NClr].r = fp [0];   Clr [NClr].g = fp [1];
                                        Clr [NClr].b = fp [2];   NClr++;}
         else {
            x = (fp [0]<<8) | fp [1];
            y = (fp [2]<<8) | fp [3];
            if ((fmt == 0) || (fmt == 4))  fp += 2;   // throw out z
            if (fmt < 4)  c = Clr [fp [5]];
            else         {c.r = fp [7];   c.g = fp [6];   c.b = fp [5];}
            if (fp [4] & 0x40)  c = OFF;
            Dot [NDot  ].x = (uword)(32768+x);
            Dot [NDot  ].y = (uword)(65535-(32768+y));
            if (Dot [NDot].x < Ixmn)  Ixmn = Dot [NDot].x;
            if (Dot [NDot].x > Ixmx)  Ixmx = Dot [NDot].x;
            if (Dot [NDot].y < Iymn)  Iymn = Dot [NDot].y;
            if (Dot [NDot].y > Iymx)  Iymx = Dot [NDot].y;
            Dot [NDot++].c = c;
//DBG("   fr=`d d=`d  `d,`d  `d,`d,`d",
//fr, d, 32768+x, 65535-(32768+y), c.r, c.g, c.b);
         }
      }

      if (nd == 0)  NFrm = fr;         // some .ild files are lame like this :(
   } while ((fr+1) < NFrm);
   p = StrLn (fn);
   if ((p > 6) && (! MemCm (& fn [p-7], "_r", 2)))    // _r9.ild
      Rep = (uword)Str2Int (& fn [p-5]);
   if ((p > 7) && (! MemCm (& fn [p-8], "_r", 2)))    // _r99.ild
      Rep = (uword)Str2Int (& fn [p-6]);
DBG("fn=`s NFrm=`d NDot=`d", & fn [XFN], NFrm, NDot);
}


//______________________________________________________________________________
EDCfg  Dac [] = {                  // name, xFlip, yFlip, seg, dwell, pps
   {"Ether Dream 5a0fffffffe5", true,true, 2048, 1, 30000}
};

EthDrm ED;                             // etherdream thingy
FrmTmr FT;                             // frametimer thingy
ulong  FR;                             // the almighty frame #

Color TerH [14] = {
   {255,   0,   0},    // 200 red
   {255, 128,   0},    // 210 orange
   {255, 255,   0},    // 220 yellow
   {128, 255,   0},    // 120 lime

   {  0, 255,   0},    // 020 green
   {  0, 255, 128},    // 021 cyan-green
   {  0, 255, 255},    // 022 cyan
   {  0, 128, 255},    // 012 cyan-blue

   {  0,   0, 255},    // 002 blue
   {128,   0, 255},    // 102 purple  magenta-blue
   {255,   0, 255},    // 202 magenta
   {255,   0, 128},    // 201 pink    magenta-red

   {255, 255, 255},    // 222 white
   {128, 128, 128}     // 222 half
};

Color TerL [14] = {
   {128,   0,   0},    // 200 red
   {128,  64,   0},    // 210 orange
   {128, 128,   0},    // 220 yellow
   { 64, 128,   0},    // 120 lime

   {  0, 128,   0},    // 020 green
   {  0, 128,  64},    // 021 cyan-green
   {  0, 128, 128},    // 022 cyan
   {  0,  64, 128},    // 012 cyan-blue

   {  0,   0, 128},    // 002 blue
   { 64,   0, 128},    // 102 purple  magenta-blue
   {128,   0, 128},    // 202 magenta
   {128,   0,  64},    // 201 pink    magenta-red

   {128, 128, 128},    // 222 white
   { 64,  64,  64}     // 222 half
};


/*
Color TerH [14] = {
   {32767, 0,     0    },    // 200 red
   {32767, 16384, 0    },    // 210 orange
   {16384, 32767, 0    },    // 220 yellow
   {8192,  32767, 0    },    // 120 lime

   {0,     32767, 0    },    // 020 green
   {0,     32767, 4096 },    // 021 cyan-green
   {0,     32767, 32767},    // 022 cyan
   {0,     16384, 8192},     // 012 cyan-blue

   {0,     0,     32767},    // 002 blue
   {8192,  0,     32767},    // 102 purple  magenta-blue
   {16384, 0,     32767},    // 202 magenta
   {32767, 0,     32767},    // 201 pink    magenta-red

   {16384, 32767, 32767},    // 222 white
   {32767, 32767, 32767}     // 222 full
};

Color TerL [14] = {
   {1568, 0,    0   },       // red
   {5556, 1600, 0   },       // orange
   {3639, 1600, 0   },       // yellow
   {2236, 1600, 0   },       // lime

   {0,    1600, 0   },       // green
   {0,   12000, 1600},       // cyan-green
   {0,    1600, 1600},       // cyan
   {0,    1600, 3200},       // cyan-blue

   {0,    0,    1600},       // blue
   {1568, 0,    1600},       // purple  magenta-blue
   {2380, 0,    1600},       // magenta
   {3840, 0,    1600},       // pink    magenta-red

   {3200, 1600, 1600},       // white
   {1568, 1600, 1600}        // low
};

Color CMid (Color c1, Color c2, ubyte per)
{ Color o;
  ulong mn, mx;
   mn = c1.r;   mx = c2.r;   if (c2.r < mn) {mn = c2.r;   mx = c1.r;}
   o.r = (uword)(mn + (mx-mn)*per / 100);
   mn = c1.g;   mx = c2.g;   if (c2.g < mn) {mn = c2.g;   mx = c1.g;}
   o.g = (uword)(mn + (mx-mn)*per / 100);
   mn = c1.b;   mx = c2.b;   if (c2.b < mn) {mn = c2.b;   mx = c1.b;}
   o.b = (uword)(mn + (mx-mn)*per / 100);
}
*/


//______________________________________________________________________________
// draw short horiz line at y
void LnH (Color c, slong y)  {ED.Dot (0, y);   ED.Dot (32000, y, c);}

void CTst (Color ci)
// color test - draw TerH, n TerL, n test color
{ slong y, c;
   y = 0;
//   for (c = 0;  c < 13;  c++, y += 2000)  LnH (TerH [c], y);
//   y += 2000;
   for (c = 0;  c < 13;  c++, y += 2000)  LnH (TerL [c], y);
   y += 5000;   LnH (ci, y);
   ED.Put ();
}



void RLin ()
{ slong x, y, x2, y2;
  Color c;
   x  = Rand () * 2 - 1;   y  = Rand () * 2 - 1;
   x2 = Rand () * 2 - 1;   y2 = Rand () * 2 - 1;
   c = TerH [Rand () % 14];
   ED.Dot (x, y, c);   ED.Dot (x2, y2, c);
   x  = Rand () * 2 - 1;   y  = Rand () * 2 - 1;
   x2 = Rand () * 2 - 1;   y2 = Rand () * 2 - 1;
   c = TerH [Rand () % 14];
   ED.Dot (x, y, c);   ED.Dot (x2, y2, c);   ED.Put ();
}



void Cir (Color c, slong cx, slong cy, slong r)
{ real  a;
  slong i, x, y, x1, y1;
   for (a = 0.0, i = 0;  i < 128;  i++, a += (2*(real)PI / 128)) {
      x = (slong)(cx + r * cos (a));
      y = (slong)(cy - r * sin (a));
      if (i == 0)  ED.Dot (x1 = x, y1 = y);
      else         ED.Dot (x, y, c);
   }
   ED.Dot (x1, y1, c);
}

void Circ (Color c, slong cx = 32767, slong cy = 32767, slong r = 30000)
{  Cir (c, cx, cy, r);   ED.Put ();  }



void Olym ()
{ slong r = 4096;
   Cir (TerL [12],32767,         r+1,     r); // (white) black top middle
   Cir (TerH [8], 32767-2*r+r/8, r+1,     r); // blue top left
   Cir (TerH [0], 32767+2*r-r/8, r+1,     r); // red top right
   Cir (TerH [2], 32767-r+r/16,  3*r-r/2, r-256); // yellow bot left
   Cir (TerH [4], 32767+r-r/16,  3*r-r/2, r-256); // green bot right
   ED.Put ();
}



void RCir ()
{ slong x, y, r;
  Color c;
   c = TerH [Rand () % 14];
   r = Rand () * 1024 / 32767;
   do {x = Rand () * 2 - 1;   y  = Rand () * 2 - 1;}
   while ((x-r < 0) || (x+r > 65535) || (y-r < 0) || (y+r > 65535));
   Cir (c, x, y, r);
   c = TerH [Rand () % 14];
   r = Rand () * 1024 / 32767;
   do {x = Rand () * 2 - 1;   y  = Rand () * 2 - 1;}
   while ((x-r < 0) || (x+r > 65535) || (y-r < 0) || (y+r > 65535));
   Cir (c, x, y, r);
   ED.Put ();
}



void Rect ()
// draw expanding n shrinking rects
{ ulong i;
  slong w, x;
  Color c;
   i = FR % 256;   w = 65536 - i*256;   x = (65536 - w) / 2;
   c = TerH [(FR/128) % 13];
   ED.Dot (x, x);
   ED.Dot (x+w-1, x, c);   ED.Dot (x+w-1, x+w-1, c);
                           ED.Dot (x,     x+w-1, c);   ED.Dot (x, x, c);
   i = 255 - i;   w = 65536 - i*256;   x = (65536 - w) / 2;
   c = TerL [(FR/128) % 13];
   ED.Dot (x, x);
   ED.Dot (x+w-1, x, c);   ED.Dot (x+w-1, x+w-1, c);
                           ED.Dot (x,     x+w-1, c);   ED.Dot (x, x, c);
   ED.Put ();
}



void Tri (real rad = 30000, real nTri = 256, real nRot = 1)
            // radius,          #triangles,      #rotations
// draw some cool lookin shrinkin rotatin triangles :)
// ang is angle (in radians) that rotates to make the triangle rotate.
// len is length that shrinks to make the tris.
{ real  ang, len;
  slong x1, y1, x2, y2, x3, y3;
  ulong i;
  Color c;
// frame steps i from 0..nTri-1;  1 triangle per frame
   i = FR % (slong)nTri;
   c = TerL [(FR/128) % 13];           // 2 colors per nTri
   len = rad - i * rad / nTri;
   ang =       i * D2Rad (360.0) * nRot / nTri;
//DBG(" i=`d len=`d ang=`d", i, (slong)len, (long)ang);
   x1 = (slong)(XX + len * cos (ang + D2Rad ( 90.0)));
   y1 = (slong)(YY - len * sin (ang + D2Rad ( 90.0)));
   x2 = (slong)(XX + len * cos (ang + D2Rad (210.0)));
   y2 = (slong)(YY - len * sin (ang + D2Rad (210.0)));
   x3 = (slong)(XX + len * cos (ang + D2Rad (330.0)));
   y3 = (slong)(YY - len * sin (ang + D2Rad (330.0)));
//DBG(" `d,`d `d,`d `d,`d", x1, y1, x2, y2, x3, y3);
   ED.Dot (x1, y1);   ED.Dot (x2, y2, c);   ED.Dot (x3, y3, c);
                                            ED.Dot (x1, y1, c);
   ED.Put ();
}



void Plan (slong seg = 512, slong y1 = 65535-1024, slong y2 = 65535)
// draw a plane (line) of hues
{ slong x, cb;
  Color c;
   cb = FR % 13;
   for (x = seg;     x < WW;  x += seg)
      {c = TerL [(x/seg + cb) % 13];   ED.Dot (x, y1, c);}
   for (x = WW-seg;  x > 0;   x -= seg)
      {c = TerL [(x/seg + cb) % 13];   ED.Dot (x, y2, c);}
   ED.Put ();
}


//______________________________________________________________________________
const slong WormSeg = 25, WormRad = 100;    // num segments, segment radius
real  WormDir;
slong WormX [WormSeg], WormY [WormSeg];

void WormInit ()
// Start in the center.  calc initial worm segs
// change WormDir, bias a little in one direction to make worm loop a bit.
// from WormDir, determine coords of new head position.
// wrap the new coords across borders, if needed.
{ slong i, x, y;
   WormDir = 0.0;   WormX [0] = 32767;   WormY [0] = 32767;
   for (i = 1;  i < WormSeg;  i++) {
      WormDir += (Rand () % 3) ? 0.1745 : -0.1745;
      x = WormX [i-1] + (slong)(((real)WormRad) * cos (WormDir));
      y = WormY [i-1] + (slong)(((real)WormRad) * sin (WormDir));
      if (x <         (WormRad/2))  x = 65535 - WormRad/2;      // wrap
      if (x > (65535 - WormRad/2))  x = WormRad/2;
      if (y <         (WormRad/2))  y = 65535 - WormRad/2;
      if (y > (65535 - WormRad/2))  y = WormRad/2;
      WormX [i] = x;   WormY [i] = y;
   }
}

void Worm ()
// toss old tail, make new head
// bump HeadPos to where new coords are to be stored. (where old tail is now)
{ slong i, x, y;
  Color c = TerH [Rand () % 14];
   for (i = 0;  i < WormSeg;  i++)
      Cir (i ? c : OFF, WormX [i], WormY [i], WormRad);
   ED.Put ();
   MemCp (WormX, & WormX [1], sizeof (WormX) - sizeof (WormX [0]));
   MemCp (WormY, & WormY [1], sizeof (WormY) - sizeof (WormY [0]));
   WormDir += (Rand () % 3) ? 0.1745 : -0.1745;
   x = WormX [WormSeg-2] + (uword)(((real)WormRad) * cos (WormDir));
   y = WormY [WormSeg-2] + (uword)(((real)WormRad) * sin (WormDir));
   if (x < (WormRad/2))          x = 65535 - WormRad/2;
   if (x > (65535 - WormRad/2))  x = WormRad/2;
   if (y < (WormRad/2))          y = 65535 - WormRad/2;
   if (y > (65535 - WormRad/2))  y = WormRad/2;
   WormX [WormSeg-1] = x;
   WormY [WormSeg-1] = y;
}


//______________________________________________________________________________
real RRand ()  {return ((real)Rand () / 65535.0);}

struct {
   bool  Rand;
   slong BrUp, BrOut, BrLen1;
   real  BrAng1, LenFac, AngFac;
} Tree = {
   true,
   5, 5, 60,
   100.0, 0.8, 0.95
};

void Branch (slong BrLvl, slong x, slong y, real Ang, real BrLen, real BrAng)
{ sword xofs, yofs, i;
  real  theta;
  Color c = TerH [Rand() % 14];
   if (BrLvl == 0) {                /* draw a leaf */
      ED.Dot ((uword)(x - 2), y);
      ED.Dot ((uword)(x + 2), y, c);//branch
      ED.Dot (x, (uword)(y - 1));
      ED.Dot (x, (uword)(y + 1), c);//leaf
   }
   else {                           /* draw a span of branches */
      xofs = (sword) (BrLen * cos (Ang * (PI / 180.0))) * 2;
      yofs = (sword) (BrLen * sin (Ang * (PI / 180.0)));
      ED.Dot (x, y);
      ED.Dot ((uword)(x + xofs), (uword)(y - yofs), c);//branch
      theta = BrAng / (Tree.BrOut - 1);
      for (Ang -= BrAng / 2.0, i = 0;  i < Tree.BrOut;  i++, Ang += theta) {
         if (Tree.Rand)
              Branch (BrLvl-1, x+xofs, y-yofs, Ang + theta*(RRand ()-0.5),
                      BrLen * Tree.LenFac * (RRand ()+0.25),
                      BrAng * Tree.AngFac);
         else Branch (BrLvl-1, x+xofs, y-yofs, Ang,
                      BrLen * Tree.LenFac, BrAng * Tree.AngFac);
      }
   }
}

void DoTree ()
{  Branch (Tree.BrUp, 32767, 20000, 90.0,
           (real) Tree.BrLen1, Tree.BrAng1);
}


//______________________________________________________________________________
struct {real a, b, c,   w, h,   x, y;}
Cry = {69.0, 53.0, 72.0,   1.0, 1.0,   200.0, 200.0};

void CrysInit ()  { real dx = 0.0, dy = 0.0; }

void Crys ()
{ real dx, dy, tx, ty;
  Color c = TerH [Rand () % 14];
   tx = Cry.x + Cry.w * (dx + dy);
   ty = Cry.y + Cry.h * (dy - dx);
   ED.Dot ((slong)tx, (slong)ty, c);
   tx = dy - (dx == 0.0 ? 0.0 : (dx > 0.0 ? 1.0 : -1.0)) *
             (sqrt (fabs (Cry.b * dx - Cry.c)));
   dy = Cry.a - dx;
   dx = tx;
}


//______________________________________________________________________________
struct CtlDef {
   CtlCmbo pick;
   CtlEdit msg, r,  g,  b,  nfr,  dwl,  seg;
   CtlSpin      rs, gs, bs, nfrs, dwls, segs;
};


void Ilda (CtlDef *_c)
{ uword i, r = (uword)_c->r.Get (), b;
  static uword pr = 3;
  static ulong pp;
   if ((ulong)r > NFN)  r = (uword)(NFN-1);
   if (r != pr) {
      LoadIld (FN [r]);   FR = 0;   pp = 0;   pr = r;
      if (NFrm == 0)  LoadIld (FN [0]);
     TStr s;
      StrFmt (s, "frames=`d dots=`d w=`d h=`d `s",
              NFrm, NDot,  Ixmx-Ixmn+1, Iymx-Iymn+1, & FN [r][XFN]);
      _c->msg.Set (s);
      _c->g.Set (Rep);
   }
   b = (uword)_c->b.Get ();
   if (b == 0) {
      FR = FR % NFrm;   if (FR == 0)  pp = 0;
      for (i = Frm [FR];  i--;)
        {ED.Dot (Dot [pp].x, Dot [pp].y, Dot [pp].c);   pp++;}
   }
   else {
      b = (b-1) % NFrm;
      for (pp = 0, i = 0;  i < b;  i++)  pp += Frm [i];
      Ixmx = Iymx = 0;   Ixmn = Iymn = 65535;
      for (i = Frm [b];  i--;) {
         ED.Dot (Dot [pp].x, Dot [pp].y, Dot [pp].c);
         if (Dot [pp].x < Ixmn)  Ixmn = Dot [pp].x;
         if (Dot [pp].x > Ixmx)  Ixmx = Dot [pp].x;
         if (Dot [pp].y < Iymn)  Iymn = Dot [pp].y;
         if (Dot [pp].y > Iymx)  Iymx = Dot [pp].y;
       pp++;
      }
     TStr s;
      StrFmt (s, "frame=`d/`d w=`d h=`d `s",
              b, NFrm, Ixmx-Ixmn+1, Iymx-Iymn+1, & FN [r][XFN]);
      _c->msg.Set (s);
   }
   ED.Put ((uword)_c->g.Get ());
   return;
}


class Framer: public Thread {
public:
   Framer (CtlDef *c): Thread (App.wndo), _c (c)  {}
private:
   CtlDef *_c;
   DWORD Go ();
   void  Frame ();
};


void Framer::Frame ()                  // DA GUTS !!
{ uword p;
   Dac [0].dwell = (uword)_c->dwl.Get ();
   Dac [0].seg   = (uword)_c->seg.Get ();
//DBG("Frame `d", FR);
   p = (uword)_c->pick.Pos ();
   if      (p == 0)  Ilda (_c);
   else if (p == 1)  {
     Color c;
      c.r = (ubyte)_c->r.Get ();   c.g = (ubyte)_c->g.Get ();
                                   c.b = (ubyte)_c->b.Get ();
      CTst (c);
   }
   else if (p == 2)  Circ (TerH [(FR/128) % 13]);
   else if (p == 3)  Tri  ();
   else if (p == 4)  Rect ();
   else if (p == 5)  Olym ();
   else if (p == 6)  RLin ();
   else if (p == 7)  RCir ();
   else if (p == 8)  Plan ();
}



DWORD Framer::Go ()
// timer currently ignored - tight loop city, but checkin ui at least
{ MSG    msg;
  bool   noClose = true;
  DWORD *m;
  ShMem  shm;
DBG("{ Framer::Go");

// hey wihnders!  gimme a message queue.
   InitCom ();   InitComCtl ();   InitMsgQ ();   msg.message = 0;
   RandInit ();                        // and seed Rand() n tell timer go!
   FT.Go ();

// am i already here?  if so, sneak out
   if (shm.Open ("lazr.exe", sizeof (DWORD)))
      {shm.Shut ();   QuitCom ();   return 0;}

// i ain't, so open up shop
   if (! (m = (DWORD *) shm.Init ("lazr.exe", sizeof (DWORD))))
      Die ("shm.Init died");
   *m = ::GetCurrentThreadId ();
TRC("shm ok");

DBG("loopin'");
   FR = 0;
   while (noClose) {
      while (::PeekMessage (& msg, (HWND)NULL, 0, 0, PM_REMOVE)) {
         switch (msg.message) {
            case MSG_TIMER:
            // Frame (FR++);
               break;
            case MSG_CLOSE+1:
            { slong i = msg.wParam;
DBG("msg=`d", i);
/*
   {M_NT(M_C, 2), "Kick", 0, "BassDrum1Electric"},
   {M_NT(M_B, 1), "Kik2", 0, "BassDrum2Acoustic"},
   {M_NT(M_D, 2), "Snar", 1, "Snare1Acoustic"},
   {M_NT(M_E, 2), "Snr2", 1, "Snare2Electric"},
   {M_NT(M_Db,3), "Cras", 3, "CymbalCrash1"},
   {M_NT(M_A, 3), "Cra2", 3, "CymbalCrash2"},
               if ((i == 36) || (i == 35) || (i == 36+2) || (i == 36+4) ||
                   (i == 48+1) || (i == 48+9))
*/
               i = _c->r.Get ();   i++;   if (i >= (slong)NFN) i = 0;
               _c->r.Set (i);
               break;
            }
            case WM_CLOSE:
               noClose = false;
               break;
         }
      }
//    if (noClose && (! ::PeekMessage (& msg, (HWND)NULL, 0, 0, PM_NOREMOVE)))
//       ::WaitMessage ();

      if (noClose)  {Frame ();   FR++;}
   }

DBG("loopin' DONE");
   shm.Shut ();
   PostDadW (MSG_CLOSE, 0, 0);         // ok, you can close too, Pop
DBG("} Framer::Go");
   return 0;
}


//______________________________________________________________________________
bool DoIld (void *ptr, char dfx, char *fn)
// find any .ild files and put em in a
{ StrArr *a  = (StrArr *) ptr;
  ulong   ln = StrLn (fn);
   if ( (dfx == 'f') && (ln >= 4) && (! StrCm (& fn [ln-4], ".ild")) )
      a->Add (fn);
   return false;
}


class Lazr: public DialogTop {
public:
   Lazr (): DialogTop (IDD_APP, IDI_APP), _f (NULL)  {}
  ~Lazr ()                        {delete _f;}

   virtual void Open ()
   {  DBG("Lazr::Open");
      PosLoad ("Lazr_Wndo");           // reload window pos
      _c.pick.Init (App.wndo, IDC_PICK);
      _c.r.Init    (App.wndo, IDC_R);
      _c.g.Init    (App.wndo, IDC_G);
      _c.b.Init    (App.wndo, IDC_B);
      _c.nfr.Init  (App.wndo, IDC_NFR);
      _c.dwl.Init  (App.wndo, IDC_DWL);
      _c.seg.Init  (App.wndo, IDC_SEG);
      _c.rs.Init   (App.wndo, IDC_SPR, 0, 255);
      _c.gs.Init   (App.wndo, IDC_SPG, 0, 255);
      _c.bs.Init   (App.wndo, IDC_SPB, 0, 255);
      _c.inis.Init (App.wndo, IDC_SPNFR,     1,   999);
      _c.dwls.Init (App.wndo, IDC_SPDWL,     0,    20);
      _c.segs.Init (App.wndo, IDC_SPSEG,    32,  2048);
      _c.msg.Init  (App.wndo, IDC_MSG);

      _c.pick.LstZZ (
         ".ild files\0"
         "RGB Test\0"
         "Circle\0"
         "Triangle\0"
         "Rectangle\0"
         "Olympic\0"
         "Random Lines\0"
         "Random Dots\0"
         "Plane\0"
      );
      _c.pick.SetPos (0);
      _c.nfr.Set (1);   _c.dwl.Set (1);   _c.seg.Set (2048);
      _c.r.Set ((slong)0);   _c.g.Set ((slong)0);   _c.b.Set ((slong)0);
      _c.msg.Set ("...");

   // load FN[] with all the .ild files
     TStr  fn;
     File  f;
     ulong i;
     StrArr t ("sIld", 6000, 6000*sizeof (TStr)/2);
      App.Path (fn, 'd');   StrAp (fn, "\\clip\\ild");
      XFN = StrLn (fn) + 1;
      f.DoDir (fn, & t, DoIld);   t.Sort ();
      for (i = 0;  i < t.num;  i++)  StrCp (FN [i], t.str [i]);
      NFN = t.num;

      _f = new Framer (& _c);          // kick the thread
   }

   virtual bool Shut ()
   {  DBG("Lazr::Shut");
      _f->PostKid (WM_CLOSE, 0, 0);   PosSave ("Lazr_Wndo");
      return false;
   }

   virtual bool Do   (int ctrl, int evnt, LPARAM l)
   {  return DoEvMap (this, _evMap, ctrl, evnt, l);  }

   void SaveFr (LPARAM lp)
   { File f;
     TStr fn, s;
     uword fr, i;
     ulong pp;
     sword x, y;
     ubyte b;
      fr = (uword)_c.b.Get ();   if (! fr)  return;

      StrFmt (fn, "`s\\clip\\ild\\_fr\\`d_`d_`d.ild",
         App.Path (s, 'd'), fr, Ixmx-Ixmn+1, Iymx-Iymn+1);
      if (! f.Open (fn, "w"))  return;

   // header
      f.Put ("ILDA\0\0\0\5                ", 24);
      b = Frm [fr] >> 8;       f.Put (& b, 1);
      b = Frm [fr] & 0x00FF;   f.Put (& b, 1);
      f.Put ("\0\0\0\1\0\0", 6);

   // single frame o dots
      for (pp = 0, i = 0;  i < fr;  i++)  pp += Frm [i];
      for (i = Frm [fr];  i--;  pp++) {
         x =          (Dot [pp].x - Ixmn)  - 32768;
         y = (65535 - (Dot [pp].y - Iymn)) - 32768;
         b = x >> 8;       f.Put (& b, 1);
         b = x & 0x00FF;   f.Put (& b, 1);
         b = y >> 8;       f.Put (& b, 1);
         b = y & 0x00FF;   f.Put (& b, 1);
         b = 0x80;   if (Dot [pp].c.r || Dot [pp].c.g || Dot [pp].c.b)  b = 0;
         if (! i)  b |= 0x40;
         f.Put (& b, 1);
         f.Put (& Dot [pp].c.b, 1);
         f.Put (& Dot [pp].c.g, 1);
         f.Put (& Dot [pp].c.r, 1);
      }

   // end header
      f.Put ("ILDA\0\0\0\5                ", 24);
      f.Put ("\0\0\0\0\0\1\0\0", 8);
      f.Shut ();
   }

private:
   Framer *_f;
   CtlDef  _c;
   static EvMap<Lazr> _evMap [];
};

EvMap<Lazr> Lazr::_evMap [] = {
   {IDC_SAVEFR, BN_CLICKED, & Lazr::SaveFr},
   {0}
};


//______________________________________________________________________________
int Go ()
{
DBG("{ Go");
   ::CoInitialize (NULL);   InitComCtl ();
   if (! ED.Open (Dac, BITS (Dac)))  return 0;
   if (! FT.Open (24))  {ED.Shut ();   return 0;}

DBG("EvPump");
  Lazr dlg;
  int  rc;
   dlg.Init();

   rc = App.EvPump ();

   FT.Shut ();   ED.Shut ();
DBG("} Go");
   return 0;
}


// need dumb crt startup due to reals
int WINAPI WinMain (HINSTANCE inst, HINSTANCE pInst, LPSTR cmdLn, int nShowCmd)
{  return AppBoot ();  }

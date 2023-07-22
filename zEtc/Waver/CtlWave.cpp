// CtlWave.cpp

#include "Waver.h"

const COLORREF DOTC = RGB(255,255,0);       // sample dot    - yellow
const COLORREF LINC = RGB(255,0,128);       // line btw dots - magenta-ish
const COLORREF BGNC = RGB(255,0,0);         // clip begin    - red
const COLORREF ENDC = RGB(255,0,0);         // clip end      - red
const COLORREF LUPC = RGB(0,255,0);         // loop markers  - green


void CtlWave::Brkt (Canvas *cnv, uword x, uword h, COLORREF c, char dir)
// draw a vertical dividing line, then the bracket lines in picked direction
{ GDIPen pen (cnv, c);
   cnv->Line (x, 0, x, h-1);    if (dir == '[')  cnv->Line (x+1, 0, x+1, h-1);
                                else if (x)      cnv->Line (x-1, 0, x-1, h-1);
   if (dir == '[')  {cnv->Line (x, 0, x+4, 0);   cnv->Line (x, h-1, x+4, h-1);}
   else if (x >= 4) {cnv->Line (x, 0, x-4, 0);   cnv->Line (x, h-1, x-4, h-1);}
   else             {cnv->Line (x, 0, 0,   0);   cnv->Line (x, h-1, 0,   h-1);}
}


inline uword S2Y (slong s, uword h)
{  return (uword)((h/2) + (shuge)s * h/2 / MAXSL);  }


void CtlWave::Draw (Canvas *cnv, bool foc)
{ Wave *wv = _w->_wave;
  ubyte lr;
  ubyte mag;
  uword W, H, x, xInc, y, py [2];
  slong smp, min, max;
  ulong bgn, end, pos, i, inc, num;
  shuge avg;       // running/final average and average's running remainder

   { GDIPen  pen (cnv, LINC);
//HGDIOBJ oldfont = cnv->Set (wv->_font.Font ());  // set decent font

// get W, H from ui;
   WH (& W, & H);
   if (! wv->_mono)  H /= 2;
   if (W < 50)  W = 50;
   if (H < 16)  H = 16;

   mag = (ubyte) _w->_c.mag.Pos ();  if (mag > 21)  mag = 21;

   bgn = (ulong) _w->_c.scrl.Pos ();
   if (bgn >= wv->_len)  bgn = 0;

   if (mag < 3) {
      if (wv->_len > (ulong)(W >> (2-mag)))
           _w->_c.scrl.SetRng (0, wv->_len-1, W >> (2-mag));
      else _w->_c.scrl.SetRng (0,0,0);
      end = bgn + (W >> (2-mag));
   }
   else {
      if ((wv->_len >> (mag-2)) > W)
           _w->_c.scrl.SetRng (0, wv->_len-1, W << (mag-2));
      else _w->_c.scrl.SetRng (0,0,0);
      end = bgn + (W << (mag-2));
   }

// draw horiz lines for top,bot,axis and if stereo do r bot,axis
   {    GDIPen pen (cnv, GREY(20));   cnv->Line (0, 0,     W-1, 0    );
                                      cnv->Line (0, H-1,   W-1, H-1  ); }
   {    GDIPen pen (cnv, GREY(34));   cnv->Line (0, H/2,   W-1, H/2  ); }
   if (! wv->_mono) {
      { GDIPen pen (cnv, GREY(20));   cnv->Line (0, 2*H-1, W-1, 2*H-1); }
      { GDIPen pen (cnv, GREY(34));   cnv->Line (0, 3*H/2, W-1, 3*H/2); }
   }

// draw bgn,end,lBgn,lEnd
   y = H;  if (! wv->_mono) y <<= 1;
   if (mag < 3) {
      if (wv->_loop) {
         if ((wv->_lBgn >= bgn) && (wv->_lBgn < end))
            Brkt (cnv, (uword)((wv->_lBgn - bgn) << (2-mag)), y, LUPC, '[');
         if ((wv->_lEnd >= bgn) && (wv->_lEnd < end))
            Brkt (cnv, (uword)((wv->_lEnd - bgn) << (2-mag)), y, LUPC, ']');
      }
      if ((wv->_bgn >= bgn) && (wv->_bgn < end))
            Brkt (cnv, (uword)((wv->_bgn  - bgn) << (2-mag)), y, BGNC, '[');
      if ((wv->_end >= bgn) && (wv->_end < end))
            Brkt (cnv, (uword)((wv->_end  - bgn) << (2-mag)), y, ENDC, ']');
   }
   else {
      if (wv->_loop) {
         if ((wv->_lBgn >= bgn) && (wv->_lBgn < end))
            Brkt (cnv, (uword)((wv->_lBgn - bgn) >> (mag-2)), y, LUPC, '[');
         if ((wv->_lEnd >= bgn) && (wv->_lEnd < end))
            Brkt (cnv, (uword)((wv->_lEnd - bgn) >> (mag-2)), y, LUPC, ']');
      }
      if ((wv->_bgn >= bgn) && (wv->_bgn < end))
            Brkt (cnv, (uword)((wv->_bgn  - bgn) >> (mag-2)), y, BGNC, '[');
      if ((wv->_end >= bgn) && (wv->_end < end))
            Brkt (cnv, (uword)((wv->_end  - bgn) >> (mag-2)), y, ENDC, ']');
   }

// draw samples
   if (mag < 3) {                      // at least 1 pixel per sample
      if (bgn < wv->_len) {            // do 1st dot
         cnv->Pixl    (0, py [0] =   S2Y (wv->Smp (bgn, 0), H),  DOTC);
         if (! wv->_mono)
            cnv->Pixl (0, py [1] = H+S2Y (wv->Smp (bgn, 1), H),  DOTC);
      }
      for (pos = bgn+1, x = 1, xInc = 1 << (2-mag);
           (pos < wv->_len) && (x < W);  pos++, x += xInc)
         for (lr = 0;  lr < (wv->_mono ? 1 : 2);  lr++) {
            y = (lr?H:0) + S2Y (wv->Smp (pos, lr), H);
            if (xInc > 1)  cnv->Line (x, py [lr], x+xInc-2, py [lr]);
            cnv->Line (x+xInc-1, py [lr], x+xInc-1, y);
            cnv->Pixl (                   x+xInc-1, y, DOTC);
            py [lr] = y;
         }
   }
   else {                              // 2+ samples per pixel
      for (pos = bgn, inc = 1 << (mag-2), x = 0;
           (x < W) && (pos < wv->_len);  x++, pos += inc)
         for (lr = 0;  lr < (wv->_mono ? 1 : 2);  lr++) {
            avg = 0;   min = -(MAXSL+1);  max = MAXSL;
            num = ((pos + inc) >= wv->_len) ? (wv->_len - pos) : inc;
            for (i = 0;  i < num;  i++) {
               smp = wv->Smp (pos+i, lr);
               if (smp < min)  min = smp;
               if (smp > max)  max = smp;
               avg += smp;
            }
            avg /= num;
            { GDIPen pen (cnv, GREY(0));  // was 34
               cnv->Line (x, (lr?H:0) + S2Y (min, H),
                          x, (lr?H:0) + S2Y (max, H));
            }
            y = (lr?H:0) + S2Y ((slong)avg, H);
            if (x)  cnv->Line (x-1, py [lr], x, y);
            py [lr] = y;
         }
   }
// cnv->Set (oldfont);  // restore ole font

   }
// scoot pos if not visible; draw it XOR style; store prev x,h
   if ((wv->_pos < bgn) || (wv->_pos >= end))  wv->SetPos (bgn);
   y = H;  if (! wv->_mono) y <<= 1;
   x = (mag < 3) ? (uword)((wv->_pos - bgn) << (2-mag))
                 : (uword)((wv->_pos - bgn) >> (mag-2));
   wv->_pX = x;  wv->_pH = y;
   cnv->SetROp (R2_NOTXORPEN);
   cnv->Line (x, 0, x, y-1);
}


void CtlWave::MsMv (ulong btn, sword x, sword y)
{ Wave *wv = _w->_wave;
  ubyte mag;
  uword W, H;
  ulong bgn, pos;
  Canvas c (Wndo ());
// get W, H from ui;
   WH (& W, & H);         if (W < 50)           W = 50;
   mag = (ubyte) _w->_c.mag.Pos ();   if (mag > 21)         mag = 21;
   bgn = (ulong) _w->_c.scrl.Pos ();  if (bgn >= wv->_len)  bgn = 0;
   if      (mag < 3) {
        pos = bgn + (x >> (2-mag));
        x = (uword)((pos - bgn) << (2-mag));
   }
   else pos = bgn + (x << (mag-2));
   wv->SetPos (pos);
   c.SetROp (R2_NOTXORPEN);   c.Line (wv->_pX, 0, wv->_pX, wv->_pH-1);
   wv->_pX = x;               c.Line (wv->_pX, 0, wv->_pX, wv->_pH-1);
}

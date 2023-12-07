// sNote.cpp - draw da notes

#include "song.h"

QColor CRng [128], CScl [12], CSclD [12], CTnt [4],
       CMid, CBBg, CBt;

static ubyte cmap [12] = {0, 4, 8,   1, 5, 9,   2, 6, 10,   3, 7, 11};

QColor CMap (ubyte n)  {return CScl [cmap [n % 12]];}

void CInit ()
{ ColRng cr;                           // Scl[12]=red,ora,yel,x,grn,trq,x,blu,x,
   cr.Init (CRng);                                                  // prp,x,mag
   for (int i = 0;  i < 12;  i++)  {CScl  [i] = HSL (30*i, 100, 75);
                                    CSclD [i] = HSL (30*i, 100, 50);}
   CTnt [0] = QColor (203, 254, 255);  // blue, red, green, yellow
   CTnt [1] = QColor (255, 220, 255);//255,203,244
   CTnt [2] = QColor (203, 255, 212);
   CTnt [3] = QColor (255, 255, 203);
   CMid     = QColor (255,  72,  77);  // middle c
   CBBg     = QColor (225, 225, 225);  // black key bg / drum div
   CBt      = QColor (205, 205, 205);  // beat line color
/*
for (ubyte i = 0;  i < 12;  i++)
DBG("`02d `02x`02x`02x", i, CScl[i].red (), CScl[i].green (), CScl[i].blue());
for (ubyte i = 0;  i < 12;  i++)
DBG("`02d `02x`02x`02x", i, CSclD[i].red (), CSclD[i].green (),CSclD[i].blue());
*/
}


ubyte Song::DrawRec (bool all, ubyt4 pp)
// draw rec trk notes (on top) - either all or ONly pNow-1..now
// very similar to DrawPg, but just a rect instead of DrawSym, etc (read it 1st)
{ ubyte nt, dnt, t, tt, dPos, c, ncc,      tp;
  ubyt2 nx, cx, h, x, x1, x2, y, y2, th = Up.txH, tpMn, tpMx, vl, v2;
  bool  drm, got;
  char  cl;
  ubyt4 tMn, tMx, pMn, pMx, t1, t2, p, ne, tDn, tUp, on [128];
  TStr  str;
  QColor  qc;
  TrkEv  *e, ev;
  TpoRow *te;
  struct {TStr s;  ubyte id;  char ty;  ubyt4 tm;  ubyt2 df, vl;}  cc [128];
  PagDef *pg = & _pag [pp];
  ColDef  co;
  DownRow *dn;
//TRC("DrawRec all=`b pp=`d", all, pp);
   if (all) {
      for (pMn = pMx = 0, t = Up.rTrk;  t < _f.trk.Ln;  t++) {
         ne = _f.trk [t].ne;
         if (ne && ((t1 = _f.trk [t].e [ne-1].time) > pMx))  pMx = t1;
      }
   }
   else  {pMn = _pNow-1;   pMx = _rNow;}
//TStr d1,d2,d3,d4;
//DBG(" pMn=`s pMx=`s  pNow=`s",
//TmSt(d1,pMn), TmSt(d2,pMx), TmSt(d3,_pNow));

// init ctl arr cc[] w ctrl id,type,default value from _f.ctl[],cc.txt
   tp = 0;                             // default to no tempo
   for (ncc = c = 0;  c < _f.ctl.Ln;  c++)  if (_f.ctl [c].sho) {
      StrCp (cc [ncc].s, _f.ctl [c].s);
      cc [ncc].id = 0x80 | c;   cc [ncc].ty = 'u';
                                cc [ncc].df = 0;
      for (t = 0;  t < NMCC;  t++)  if (! StrCm (MCC [t].s, cc [ncc].s))
         {cc [ncc].df = MCC [t].dflt;   cc [ncc].ty = MCC [t].typ;   break;}
      if (! StrCm (cc [ncc].s, CC("tmpo"))) {
         tp = ncc+1;   cc [ncc].id = 0xFF;

      // show prev col if we're on col>0 n we're <= tMn+M_WHOLE
         for (t = 0;  t < pg->nCol;  t++) {
            MemCp (& co, & pg->col [t], sizeof (co));      // load column specs
            tMn = co.blk [0].tMn;   tMx = co.blk [co.nBlk-1].tMx;
            if (tMx <= pMn)  continue;      // NEXT !
            if (tMn >= pMx)  break;         // we're done

            if ((! all) && t && (pMn <= tMn+M_WHOLE))  pMn = tMn - M_WHOLE;
         }                                  // gotta show prev rec tempo
      }
//DBG(" cc=`s col=`d id=$`02x dflt=`d typ=`c",
//cc [ncc].s, ncc, cc [ncc].id, cc [ncc].df, cc [ncc].ty);
      ncc++;
   }

   for (c = 0;  c < pg->nCol;  c++) {
      MemCp (& co, & pg->col [c], sizeof (co));  // load column specs
      tMn = co.blk [0].tMn;   tMx = co.blk [co.nBlk-1].tMx;
      if (tMx <= pMn)  continue;       // NEXT !
      if (tMn >= pMx)  break;          // we're done

      nx = Nt2X (co.nMn, & co);   cx = CtlX (& co);
//DBG(" col=`d tMn=`s tMx=`s nMn=`s nMx=`s w=`d h=`d x=`d nx=`d cx=`d",
//c,TmSt(d1,tMn),TmSt(d2,tMx),
//MKey2Str(d3,co.nMn),MKey2Str(d4,co.nMx),co.w,co.h,co.x, nx,cx);
      if (tp) {                        // tempo is super special...:/
         x  = cx + (tp-1) * th;
      // go thru prescribed tempo evs, get min,max and adjust w clip for tpMn,Mx
         te = & _f.tpo [0];   ne = _f.tpo.Ln;   vl = 120;
         for (p = 0;  (p < ne) && (te [p].time < tMn);  p++)  vl = te [p].val;
         t1 = 0;   tpMn = tpMx = TmpoAct (vl);

      // if dot at top, no prev val: gotta init tpMn,Mx in loop
         if ((p < ne) && (te [p].time == tMn))  t1 = NONE;
//DBG("  tmpo t1=`s p=`d/`d vl=`d", TmSt(d1,t1), p, ne, vl);
         for (     ;  (p < ne) && (te [p].time < tMx);  p++) {
            vl = TmpoAct (te [p].val);
            if (t1 == NONE)  {t1 = 0;   tpMn = tpMx = vl;}   // did init
            if (vl < tpMn)   tpMn = vl;
            if (vl > tpMx)   tpMx = vl;
         }
         tpMn -= (tpMn / 4);   tpMx += ( (tpMx / 4) + (((tpMx%4)>=2)?1:0) );
//DBG("  tpMn=`d tpMx=`d p=`d/`d", tpMn, tpMx, p, ne);

      // go thru rec tempos n connect n dot em
         dn = & _dn [0];   ne = _dn.Ln;   t1 = NONE;   vl = 0;   cl = '\0';
         for (p = 0;  (p < ne) && (dn [p].time < tMn);  p++)
            {t1 = dn [p].time;   vl = dn [p].tmpo;   cl = dn [p].clip;}
         vl = TmpoAct (vl);            // catch up p to min time
         if      ((vl == 0) || ((p < ne) && (dn [p].time == tMn)))  t1 = NONE;
         else if (t1 < tMn)                                         t1 = tMn;
//DBG("  init t1=`s vl=`d p=`d/`d", TmSt(d1, t1), vl, p, ne);

         for (;       (p < ne) && ((!p) || (dn [p-1].time < tMx));  p++) {
         // dn[p] => v2,t2 => x2,y2  (prev => vl,t1 => x1,y)
            v2 = TmpoAct (dn [p].tmpo);
            t2 = dn [p].time;   if (t2 > tMx)  {t2 = tMx;   v2 = 0;}
//DBG("  p=`d/`d  t1=`s vl=`d  t2=`s v2=`d",
//p, ne,  TmSt(d1, t1), vl,  TmSt(d2, t2), v2);
            if ((t1 != NONE) && vl) {
               x1 = (th-2)*(vl-tpMn)/(tpMx-tpMn);   y  = Tm2Y (t1, & co);
               x2 = (th-2)*(v2-tpMn)/(tpMx-tpMn);   y2 = Tm2Y (t2, & co);
//DBG("     x1=`d y=`d x2=`d y2=`d cl=`c", x1, y, x2, y2, cl);
               qc = CBLACK;   if (cl=='s') qc = CSclD [4]; // slow=>green
                              if (cl=='f') qc = CSclD [0]; // fast=>red
               Up.cnv.RectF (x+x1, y, 2, y2-y+1, qc); // vline vl, t1-t2
               if (v2) {                              // hline vl-v2, t2
                  if      (x1 < x2) Up.cnv.RectF (x+x1, y2, x2-x1+1, 2, qc);
                  else if (x1 > x2) Up.cnv.RectF (x+x2, y2, x1-x2+1, 2, qc);
                  Up.cnv.RectF                   (x+x2-1, y2-1, 4, 4, qc);
               }                                      // dot v2, t2
            }
            t1 = t2;   vl = v2;   cl = dn [p].clip;   // 2 becomes 1
         }
//DBG("  end p=`d/`d  t1=`s vl=`d cl=`c", p, ne, TmSt(d1,t1), vl, cl);
      }

   // draw rec notes - cache noteon stuff, draw upon noteoff or end of loop
      for (t = Up.rTrk;  t < _f.trk.Ln;  t++) {
         e = _f.trk [t].e;   ne = _f.trk [t].ne;
         drm = TDrm (t);
         for (nt = 0;  nt < 128;  nt++)   on [nt]    = NONE;
         for (tt = 0;  tt < ncc;  tt++)  {cc [tt].tm = NONE;
                                          cc [tt].vl = cc [tt].df;}
      // get existing noteOns,CCs till tMn
         for (p = 0;  (p < ne) && (e [p].time < tMn);  p++) {
            if (((nt = e [p].ctrl) & 0x80) == 0) {                   // note
               if      ((e [p].valu & 0x80) == 0)  on [nt] = NONE;   // up
               else if ((e [p].val2 & 0x80) == 0)  on [nt] = p;      // dn
            }
            else                                                     // ctrl
               for (tt = 0;  tt < ncc;  tt++)  if (nt == cc [tt].id)
                  {cc [tt].tm = tMn;   cc [tt].vl = e [p].valu;}
         }
         for (;       (p < ne) && (e [p].time < tMx);  p++) {
            if ((nt = e [p].ctrl) & 0x80) {                          // ctrl
               for (x = cx, tt = 0;  tt < ncc;  tt++, x += th)
                                                         if (cc [tt].id == nt) {
//TStr z1,z2;
//DBG("c  ?p=`d/`d e.tm=`s ctl=`d val=`d",
//p, ne, TmSt(z1,e [p].time), e [p].ctrl, e [p].valu);
                  if (((t2 = e [p].time) >= pMn) && (t2 <= pMx)) {
                     if (cc [tt].ty == 'x') {
                        CtlX2Str (str, cc [tt].s, & e [p]);
                        Up.cnv.TextVC (x, Tm2Y (t2, & co), str, CBLACK);
                     }
                     else if (cc [tt].ty == 'o') {
                        t1 = cc [tt].tm;
                        if ((t1 == NONE) || (t1 < tMn))  t1 = tMn;
                        if (t1 < pMn)  t1 = pMn;
                        y  = Tm2Y (t1, & co);
                        if (t2 > tMx)  t2 = tMx;
                        if (t2 > pMx)  t2 = pMx;
                        y2 = Tm2Y (t2, & co);

                        if (cc [tt].vl >= 64)
                           Up.cnv.RectF (x+3, y, 2, y2-y+1, CBLACK);
                     }
                     else {                 // s u
                        t1 = cc [tt].tm;
//DBG("c  pEv.tm=`s va1=`d", TmSt(z2,t1), cc [tt].vl);
                        if ((t1 == NONE) || (t1 < tMn))  t1 = tMn;
                        if (t1 < pMn)  t1 = pMn;
                        y  = Tm2Y (t1, & co);
                        if (t2 > tMx)  t2 = tMx;
                        if (t2 > pMx)  t2 = pMx;
                        y2 = Tm2Y (t2, & co);

                        x1 = (th-2)*cc [tt].vl/127;
                        if (cc [tt].vl != cc [tt].df)
                           Up.cnv.RectF (x+x1, y, 2, y2-y+1, CBLACK);

                        x2 = (th-2)*e [p].valu/127;
//DBG("c     h=`d y1=`d y2=`d x1=`d x2=`d t1=`s t2=`s",
//y2-y+1, y, y2, x1, x2, TmSt(z2,t1), TmSt(z1,t2));
                        if      (x1 < x2)
                           Up.cnv.RectF (x+x1, y2, x2-x1+1, 2,CBLACK);
                        else if (x1 > x2)
                           Up.cnv.RectF (x+x2, y2, x1-x2+1, 2,CBLACK);
                     }
                  }
                  cc [tt].tm = e [p].time;   cc [tt].vl = e [p].valu;
                  break;               // got dat ctl.  skip on out
               }
            }
            else {
               if      ((e [p].valu & 0x80) == 0) {                  // note up
                  tUp = e [p].time;
                  if (on [nt] == NONE) {                   // hmm, missing ntDn
                     MemSet (& ev, 0, sizeof (ev));
                     ev.time = tUp - 4;   ev.ctrl = nt;
                                          ev.valu = (ubyte)(0x80|100);
                  }
                  else {
                     MemCp (& ev, & e [on [nt]], sizeof (ev));
                     if (ev.time < tMn)  ev.time = tMn;
                  }
                  tDn = ev.time;
                  if ((tUp > pMn) && (tDn < pMx)) {
                     if (tDn < pMn)  tDn = pMn;
                     if (tUp > pMx)  tUp = pMx;
                     if (tUp >= tMx)  tUp = tMx-1;
                     dnt = nt;
                     if (! drm) {
                        if (nt < co.nMn) dnt = co.nMn;     // rec note COULD be
                        if (nt > co.nMx) dnt = co.nMx;     // anywhere, put on
                        x = Nt2X (dnt, & co);              // screen
                     }
                     else {
                        for (got = false, dPos = 0;  dPos < co.nDrm;  dPos++) {
                           tt = co.dMap [dPos];
                           if (_f.trk [tt].drm == nt) {got = true;   break;}
                        }
                        if (! got)  dPos = 1;    // rec'd - put weird ones in
                        x = Dr2X (dPos, & co);   // leftmost spot
                     }
                     y = Tm2Y (tDn, & co);   h = Tm2Y (tUp, & co) - y;
//TStr db1,db2,db3;
//DBG("   a nt=`s tDn=`s tUp=`s y=`d h=`d",
//MKey2Str(db3,nt), TmSt(db1,ev.time), TmSt(db2,tUp), y, h);
                  // clr = GRAY (196 - (ev.valu & 0x7F));
                     Up.cnv.RectF ( x+6, y, W_NT-12, h, CBLACK);
                     if (tDn == ev.time) {       // head
                        Up.cnv.RectF (x+0, y,   W_NT-0, 1, CBLACK);
                        Up.cnv.RectF (x+2, y+1, W_NT-4, 1, CBLACK);
                        Up.cnv.RectF (x+4, y+2, W_NT-8, 1, CBLACK);
                     }
                  }
                  on [nt] = NONE;
               }
               else if ((e [p].val2 & 0x80) == 0)  on [nt] = p;      // note dn
            }
         }
         for (nt = 0;  nt < 128;  nt++)  if (on [nt] != NONE) {
//TStr db1;
//DBG("   dnOnly nt=`s", MKey2Str (db1, nt));
         // for rec trk, we MIGHT have a NtDn with no up RECORDED yet
            tDn = e [on [nt]].time;   tUp = NONE;
            for (p = on [nt] + 1;  p < ne;  p++)  if (e [p].ctrl == nt)
               {if ((e [p].valu & 0x80) == 0)  tUp = e [p].time;   break;}
         // can't use now cuz it's often just not buffered yet (or somethin:/)
            if (tUp == NONE) {
            // maybe no noteup, maybe middle of rec note - just kill if > 2 bars
               tUp = pMx;
               if ( (tUp < tDn) || ((tUp-tDn) > (M_WHOLE*2)) )  tUp = tDn+4;
            }                             // must be lame
            if ((tUp < tMn) || (tDn > tMx))  continue;     // NEXT !!

            if (tDn < tMn)  tDn = tMn;   if (tUp > tMx)  tUp = tMx;
            if ((tUp > pMn) && (tDn < pMx)) {
               if (tDn < pMn)  tDn = pMn;
               if (tUp > pMx)  tUp = pMx;
               if (tUp >= tMx)  tUp = tMx-1;
               dnt = nt;
               if (! drm) {
                  if (nt < co.nMn) dnt = co.nMn;      // rec note COULD be
                  if (nt > co.nMx) dnt = co.nMx;      // anywhere, put on
                  x = Nt2X (dnt, & co);               // screen
               }
               else {
                  for (got = false, dPos = 0;  dPos < co.nDrm;  dPos++) {
                     tt = co.dMap [dPos];
                     if (_f.trk [tt].drm == nt)  {got = true;   break;}
                  }
                  if (! got)  dPos = 1;
                  x = Dr2X (dPos, & co);
               }
               y = Tm2Y (tDn, & co);   h = Tm2Y (tUp, & co) - y + 1;
//TStr db1,db2,db3;
//DBG("   b nt=`s tDn=`s tUp=`s y=`d h=`d",
//MKey2Str(db3,nt), TmSt(db1,tDn), TmSt(db2,tUp), y, h);
               Up.cnv.RectF ( x+6, y, W_NT-12, h, CBLACK );
               if (tDn == e [on [nt]].time) {    // head
                  Up.cnv.RectF (x+0, y,   W_NT-0, 1, CBLACK);
                  Up.cnv.RectF (x+2, y+1, W_NT-4, 1, CBLACK);
                  Up.cnv.RectF (x+4, y+2, W_NT-8, 1, CBLACK);
               }
            }
         }
         tUp = pMx;   if (tUp > tMx)  tUp = tMx;
         y2 = Tm2Y (tUp, & co);
//DBG("   zPrep y2=`d tUp=`d", y2, tUp);
         for (tt = 0;  tt < ncc;  tt++)  if ((tDn = cc [tt].tm) <= tUp) {
//TStr d1;
//DBG("c  z?  tt=`d tm=`s vl=`d", tt, TmSt(d1,tDn), cc [tt].vl);
            if ((cc [tt].ty != 'x') && (cc [tt].vl != cc [tt].df)) {
            // continue ctl val to bot of col/now
               if (tDn < tMn)  tDn = tMn;
               if (tDn < pMn)  tDn = pMn;
               if (tDn > tUp)  continue;    // shouldn't but does fer now :/

               y = Tm2Y (tDn, & co);
//DBG("c  z  h=`d y=`d tDn=`s", y2-y+1, y, TmSt(d1,tDn));
               if (cc [tt].ty == 'o')
                  {if (cc [tt].vl >= 64)
                      Up.cnv.RectF (cx+tt*th+3, y, 2, y2-y+1, CBLACK);}
               else   Up.cnv.RectF (cx+tt*th+(th-2)*cc [tt].vl/127, y,
                                                   2, y2-y+1, CBLACK);
            }
         }
      }
   }
//TRC("DrawRec  END");
   return tp;
}


//------------------------------------------------------------------------------
void Song::DrawFng (ubyt2 x, ubyt2 y, ubyte f, char tc)
{ ubyt2 wc [11] = {14, 16, 16, 16, 15,   15, 15, 20, 20, 26, 11},
           /* 0-13=14 14-29=16 30-45=16 46-61=16 62-76=15
              77-91=15 92-106=15 107-126=20 127-146=20 147-172=26 173-183=11 */
        o, w, o2, w2, h = Up.fng->height ();
  ubyte i, e, e2;
  Canvas *c;
   c = (tc == 't') ? (& Up.tcnv) : (& Up.cnv);
   y -= (h+1);
   if      (f <=  5)  e = f - 1;
   else if (f >= 26)  e = f - 26 + 5;
   else {
      e  = (f-6) / 4;
      w  = wc [e];    for (o  = 0, i = 0;  i < e;   i++)  o  += wc [i];
      e2 = (f-6) % 4;   if (e2 >= e)  e2++;
      w2 = wc [e2];   for (o2 = 0, i = 0;  i < e2;  i++)  o2 += wc [i];
      x -= ((w+w2)/2);
//DBG("f=`d e=`d e2=`d", f, e, e2);
      c->Blt (*Up.fng, x,   y,  o,  0, w,  h);
      c->Blt (*Up.fng, x+w, y,  o2, 0, w2, h);
      return;
   }
   w = wc [e];   for (o = 0, i = 0;  i < e;  i++)  o += wc [i];
   x -= (w/2);   c->Blt (*Up.fng, x, y,  o, 0,  w, h);
}


void Song::DrawSym (SymDef *s, ColDef *co)
// draw round rect w scale/velo color, fill with drum=blu,white/black + scc
{ ubyte tr, t, tc, key, ef, n;
  bool  dr, ez;
  char  ha;
  ubyt2 nx, mo, x, y, w, h, dx, dw, dh;
  TrkRow *trk;
  TrkNt  *nt;
  QColor  clr, kc;                     // main color, key color
   tr = s->tr;   trk = & _f.trk [tr];   nt = & trk->n [s->nt];
                 ez = TEz (tr);         dr = TDrm (tr);
   n = ez ? ((ubyte)s->nt) : nt->nt;
   nx = Nt2X (co->nMn, co);
   if (dr) {
      switch (MDrm2Grp (trk->din)) {
         case 0:  clr = CSclD [ 0];   break;     // kick red
         case 1:  clr = CSclD [ 4];   break;     // snar green
         case 2:  clr = CSclD [ 7];   break;     // hhat blue
         case 3:  clr = CSclD [ 1];   break;     // cymb orange
         case 4:  clr = CSclD [ 2];   break;     // toms yellow
         case 5:  clr = CSclD [ 9];   break;     // misc purple
         case 6:  clr = CSclD [11];   break;     // latn magenta
         default: clr = CSclD [ 6];   break;     // x    cyan
      }                                // oops doin velocity
      if (Cfg.ntCo == 1)  clr = CRng [(nt->dn == NONE) ? 64 :
                                      (trk->e [nt->dn].valu & 0x7F)];
   }
   else if (ez)                        // f to purple so it shows better
      clr = CScl [(tc = n % 12) == 5 ? 9 : tc];
   else
      switch (Cfg.ntCo) {
         case 2:                       // track
            for (tc = t = 0;  t < tr;  t++)  if (TSho (t))  tc++;
            clr = CMap (tc);
            break;
         case 1:                       // velocity
            clr = CRng [(nt->dn == NONE) ? 64 : (trk->e [nt->dn].valu & 0x7F)];
            break;
         default:                      // scale - pitched per keysig
            key = KSig (nt->tm)->key;
            clr = CScl [((n % 12) + 12 - key) % 12];
      }
   kc = (dr || (KeyCol [n%12] == 'w')) ? CWHITE : CBLACK;
   x = nx + s->x;   y = s->y;   w = s->w;   h = s->h;
                                       // white dudes: big head, little butt
   if (kc == CWHITE) {
      if (s->top)  {if (h >= 18)  h = 16;}
      else         {x = Nt2X (n, co, 'g');   w = W_NT;}
   }
   if (dr && (h >= 8))  h = 6;         // drums have skinny head n butt
//TStr sx;
//DBG("dr=`b nt=`s x=`d", dr, MKey2Str (sx, s->nt), x);
   if (s->top)  {Up.cnv.RectF (x+2, y,     w-4, 1, clr);
                 Up.cnv.RectF (x+1, y+1,   w-2, 1, clr);   y += 2;   h -= 2;}
   if (s->bot)  {Up.cnv.RectF (x+1, y+h-2, w-2, 1, clr);
                 Up.cnv.RectF (x+2, y+h-1, w-4, 1, clr);             h -= 2;}
                 Up.cnv.RectF (x, y, w, h, clr);
   if (s->top) {y -= 2;  h += 2;}   if (s->bot) h += 2;    // put em back

   if ((! ez) && (nt->dn != NONE) && (ef = trk->e [nt->dn].val2 & 0x1F))
      DrawFng (x + w/2, y, ef);

   if (dr) {
      if (h != s->h) {                 // room for butt?
         y += 6;   h = s->h - 6;   x += (w-4) / 2;   w = 4;
         if (s->bot)  {Up.cnv.RectF (x+1, y+h-1, 2, 1, clr);   h--;}
         Up.cnv.RectF (x, y, w, h, clr);
      }
      return;
   }

   mo = 3;                             // middle offset from x for fillin
   dh = 10;   if (h < 12)  dh = (h > 2) ? (h-2) : 2;
   if (s->top) {                       // rounded-ish dot aligned to hand
      ha = trk->ht;
      dw = w - mo*2;
      dx = x;   if      (ha == 'R')  dx += (mo*2);
                else if (ha != 'L')  dx +=  mo;
                   Up.cnv.RectF (dx+1, y,   dw-2, dh,   kc);
      if (dh > 2)  Up.cnv.RectF (dx,   y+1, dw,   dh-2, kc);
   }

   if ((kc == CWHITE) && s->top && (h != s->h)) {
   // white dudes have tail of only W_NT for true h
      y += 14;   h = s->h - 14;
      x = Nt2X (n, co, 'g');   w = W_NT;

      if (s->bot)  {Up.cnv.RectF (x+1, y+h-2, w-2, 1, clr);
                    Up.cnv.RectF (x+2, y+h-1, w-4, 1, clr);   h -= 2;}
      Up.cnv.RectF (x, y, w, h, clr);
   }
}


void Song::DrawPg (ubyt4 pp)
// draw pg's cols' bg n lrn chd,cue,etc
{ ubyte nd, nt, n2, oc, t, td, c, cc, ct, sb, key, ksig [12], cno, tn, bt;
  sbyte hit;
  char  vt;
  ubyt2 vl, df, tw, th, qx, qw, nx, wb, nw, cx, x, w, y, x1, x2, y2, w2,
        tpMn, tpMx;
  TStr  cs, str;
  ubyt4 nTrk, tMn, tMx, p, q, ne, t1, t2, ts, lt;
  bool  ccg, bug = false;
  KSgRow *ks;
  TrkEv  *e;
  TpoRow *te;
  TrkRow *trk;
  PagDef *pg = & _pag [0];
  ColDef  co;
  BlkDef *bl;
//TRC("DrawPg `d", pp);
// load constant-ish stuffs
   trk = & _f.trk [0];   nTrk = Up.rTrk;   tw = 8;   th = Up.txH;
   Up.cnv.RectF (0, 0, Up.w, Up.h, CWHITE);     // cls to white
   for (c = 0;  c < pg [pp].nCol;  c++) {
      MemCp (& co, & pg [pp].col [c], sizeof (co));    // load column specs
      tMn = co.blk [0].tMn;   tMx = co.blk [co.nBlk-1].tMx;
      qw = (_lrn.chd?th:0) + W_Q;   wb = 0;
      qx = co.x + 4;   nx = qx + qw;   cx = CtlX (& co);   nw = cx - nx;

   // non-last column border vline w rounded ends
      if (c < pg [pp].nCol-1) {
         Up.cnv.RectF (co.x+co.w-8,      1, 4, 1, CBLACK);
         Up.cnv.RectF (co.x+co.w-7,      2, 4, 1, CBLACK);
         Up.cnv.RectF (co.x+co.w-6,      3, 4, 1, CBLACK);
         Up.cnv.RectF (co.x+co.w-5, 4, 4, co.h-7, CBLACK);
         Up.cnv.RectF (co.x+co.w-6, co.h-3, 4, 1, CBLACK);
         Up.cnv.RectF (co.x+co.w-7, co.h-2, 4, 1, CBLACK);
         Up.cnv.RectF (co.x+co.w-8, co.h-1, 4, 1, CBLACK);
      }
//TStr s1,s2,s3,s4;
//DBG("c=`d tMn=`s tMx=`s nMn=`s nMx=`s w=`d h=`d x=`d "
//"qx=`d nx=`d cx=`d qw=`d nw=`d",
//c,TmSt(s1,tMn),TmSt(s2,tMx),
//MKey2Str(s3,co.nMn),MKey2Str(s4,co.nMx),co.w,co.h,co.x, qx,nx,cx,qw,nw);

   // draw bg horiz rect (white&black keyboard);  label octaves at b|c
      Up.cnv.SetFg (CBLACK);

   // prep ksig biz
      key = (ks = KSig (tMn))->key;   MemSet (ksig, 0, sizeof (ksig));
      if (ks->min)
           {MemCp (& ksig [key], CC("0 12 3 45 6 "),   12-key      );
            MemCp (  ksig,       CC("0 12 3 45 6 ") + (12-key), key);}
      else {MemCp (& ksig [key], CC("0 1 23 4 5 6"),   12-key      );
            MemCp (  ksig,       CC("0 1 23 4 5 6") + (12-key), key);}
      for (x = nx, oc = co.nMn/12;  oc <= co.nMx/12;  oc++, x += w) {
         nt =  0;   if (oc == co.nMn/12)  nt = co.nMn%12;
         nd = 11;   if (oc == co.nMx/12)  nd = co.nMx%12;
         x1 = nt*W_NT;   w = W_NT*(nd-nt+1);
      // got leftmost whiteBump?
         if (nt && (KeyCol [nt] == 'w'))  {wb = WXOfs [nt] * W_NT/12;
                                           x1 -= wb;   w += wb;}
//DBG(" oct x=`d nt=`d nd=`d w=`d x1=`d wb=`d", x, nt, nd, w, x1, wb);

      // keyboard oct at top of col
         Up.cnv.Blt (*Up.oct,  x, 0,                  x1, 0, w, H_KB);

      // background stripes down the col
         Up.cnv.Blt (*Up.pnbg, x, H_KB, w, co.h-H_KB, x1, 0, w, 1);

      // label at b|c borders;  also middle c line unless at left border
         StrFmt (str, "`d", oc-1);
         if (nt ==  0) {Up.cnv.Text (x+2,      13, str);
                        if ((oc == 5) && (x != nx))
                           Up.cnv.RectF (x, H_KB, 1, co.h-H_KB, CMid);
                       }
         if (nd == 11)  Up.cnv.Text (x+w-tw-3, 13, str);

      // draw curr keysig;  if in scale, put step color
         w2 = 3;
         if ((Cfg.ntCo == 0) && (! _lrn.ez))
            for (x2 = x+wb, n2 = oc*12+nt;  n2 <= oc*12+nd;  n2++, x2 += W_NT)
               if (ksig [n2 % 12] != ' ')
                  Up.cnv.RectF (x2 + w2, 5, W_NT-w2*2, W_NT-w2*2-2,
                                           CSclD [((n2 % 12) + 12 - key) % 12]);
         wb = 0;
      }
      for (nt = co.nMn;;  nt++) {   // 1st w from left non B,C
         nd = nt % 12;
         if (nt >= co.nMx)  break;
         if (KeyCol [nd] == 'w') {
            if (nd && (nd != 11))
               {StrFmt (str, "`d", nt/12-1);
                Up.cnv.Text (Nt2X (nt, & co, 'g')+2, 13, str);}
            break;
         }
      }
      for (nt = co.nMx;;  nt--) {   // 1st w from right non B,C
         nd = nt % 12;
         if (nt <= co.nMn)  break;
         if (KeyCol [nd] == 'w') {
            if (nd && (nd != 11))
               {StrFmt (str, "`d", nt/12-1);
                Up.cnv.Text (Nt2X (nt, & co, 'g')+2, 13, str);}
            break;
         }
      }
   //__________________________________
   // vert line per drum(top),ctl(base) - skip 1st ctl's line
      for (x = cx - co.nDrm*W_NT, t = 0;  t < co.nDrm;  t++)
         {x += W_NT;   Up.cnv.RectF (x-1, 0, 1, co.h, CBBg);}
      for (x = cx, t = 0;  t < _f.ctl.Ln;  t++)  if (_f.ctl [t].sho)
         {if (x > cx) Up.cnv.RectF (x, 0, 1, co.h, CBBg);   x += th;}

   // draw bars/beats/subbeats background lines - 1st time is beat just b4 tMn
      for (tn= bt = 0, t1 = tMn;  t1 < tMx;  t1 = t2, bt++) {
         y = Tm2Y (t1, & co, & bl);   sb = bl->sb;
        char *bp;
         TmStr (str, t1, & t2);   bp = 1 + StrCh (str, '.');
         if (! StrCm (bp, CC("1"))) {  // beat 1 = bar:  dk bar
            Up.cnv.RectF (nx, y, nw, 2, CDBLU);
            tn = TSig (t1)->num;
            tn = ((tn < 6) || (tn % 3)) ? 0 : (tn / 3);    // compound tsig num?
            bt = 0;
         }
         else if (sb)                  // color "sub compound" lighter
            Up.cnv.RectF (nx, y, nw, 1, (tn && (bt % 3)) ? CBt : CDBLU);
         if (sb > 1)  while (--sb) {   // loop thu any subbt
            ts = t1 + sb * (t2 - t1) / bl->sb;
            Up.cnv.RectF (nx, Tm2Y (ts, & co), nw, 1, CBBg);
         }
      }
      Up.cnv.RectF (nx, co.h, nw, 2, CDBLU);  // an end bar for col

   // drum & ctl labels on top of bg time lines
      x1 = cx - co.nDrm*W_NT;
      Up.cnv.SetFg (CBLACK);
      for (x = x1, p = 0;  p < co.nDrm;  p++) {
         t = co.dMap [p];
         StrCp (str, trk [t].name);   str [4] = '\0';
                               Up.cnv.TextV (x, 3, str);   x += W_NT;
      }
      for (x = cx, t = 0;  t < _f.ctl.Ln;  t++)
         if (_f.ctl [t].sho)  {Up.cnv.TextV (x, 3, _f.ctl [t].s);   x += th;}
      if ((x > x1) && (c < pg [pp].nCol-1))
         Up.cnv.RectF (x1, 0, x-x1-1, 2, CBLACK);
   //__________________________________
   // cue n chd
     ubyt2 ve = 0, ch = 0, br = 0;
     bool  sw = true;
   // cue,chd bg lt yellow default bg (ch,ve on top)
      y = Tm2Y (tMn, & co);
      Up.cnv.RectF (qx, y, qw, co.h-y, CTnt [3]);

   // FIRST (v/c/b/etc rect hilites so underneath-est (text later)
      Up.cnv.TextVC (nx-W_Q, 0, CC("Cues"), CBLACK);
      for (ne = _f.cue.Ln, p = 0;  p < ne;  p++)
         if (* (StrCp (str, _f.cue [p].s)) == '(') {
            t1 = _f.cue [p].time;   t2 = Bar2Tm (9999);
            for (q = p+1;  q < ne;  q++)
               if (_f.cue [q].s [0] == '(')
                  {t2 = _f.cue [q].time;   break;}
            if ((t2 >= tMn) && (t1 < tMx)) {
               if (t1 <  tMn)  t1 = tMn;
               if (t2 >= tMx)  t2 = tMx-1;
               y  = Tm2Y (t1, & co);   y2 = Tm2Y (t2, & co);
               if      (! StrCm (str, CC("(verse")))   cno = 0;
               else if (! StrCm (str, CC("(chorus")))  cno = 1;
               else if (  StrSt (str, CC("(break")))   cno = 2;
               else                                    cno = 3;
               Up.cnv.RectF (qx, y, qw, y2-y+1, CTnt [cno]);
            }
         }
   // THEN < or >
      for (p = 0;  p < ne;  p++)
         if ( _f.cue [p].tend &&
             (_f.cue [p].tend >= tMn) && (_f.cue [p].time < tMx)) {
         StrCp (str, _f.cue [p].s);
         if (*str == '[')  continue;     // loops only shown w fade later

         t1 = _f.cue [p].time;   if (t1 <  tMn)  t1 = tMn;
         t2 = _f.cue [p].tend;   if (t2 >= tMx)  t2 = tMx-1;
         y  = Tm2Y (t1, & co);   y2 = Tm2Y (t2, & co)-1;
         if (y2-y+1 < 8)  continue;

         else if (*str == '<')  {Up.cnv.Line (nx-W_Q/2,   y, nx-W_Q,     y2);
                                 Up.cnv.Line (nx-W_Q/2+1, y, nx-1,       y2);}
         else if (*str == '>')  {Up.cnv.Line (nx-W_Q,     y, nx-W_Q/2,   y2);
                                 Up.cnv.Line (nx-1,       y, nx-W_Q/2+1, y2);}
      }
   // FINALLY (v/c/b/etc text and non range cues on top
      for (p = 0;  (p < ne) && (_f.cue [p].time < tMn);  p++) {
         StrCp (str, _f.cue [p].s);
         if (! StrCm (str, CC("(verse")))   ve++;
         if (! StrCm (str, CC("(chorus")))  ch++;
         if (! StrCm (str, CC("(break")))   br++;
      }
      for (;       (p < ne) && (_f.cue [p].time < tMx);  p++) {
         StrCp (str, _f.cue [p].s);
         if (! StrCm (str, CC("(verse")))   ve++;
         if (! StrCm (str, CC("(chorus")))  ch++;
         if (! StrCm (str, CC("(break")))   br++;
         if ((*str == '(') && (_f.cue [p].time >= tMn) &&
                              (_f.cue [p].time <  tMx)) {
            t1 = _f.cue [p].time;   if (t1 < tMn)  t1 = tMn;
            if      (! StrCm (str, CC("(verse")))
                  StrFmt (str, "verse `d",  ve);
            else if (! StrCm (str, CC("(chorus")))
                  StrFmt (str, "chorus `d", ch);
            else if (! StrCm (str, CC("(break")))
                  StrFmt (str, "break `d",  br);
            else  StrCp  (str, & str [1]);
            y = Tm2Y (t1, & co);
            Up.cnv.TextVC (nx-W_Q, y+4, str, CBLACK);

            *str = '(';             // sorry !!
         }
         if (StrCh (CC("[(<>"), *str))  continue;

         y = Tm2Y (_f.cue [p].time, & co);
         x = 0;
         if      (        *str == '.'     )   {x =   1;   w =  7;}   // 2px over
         else if (! StrCm (str, CC("`fer")))  {x =  24;   w = 24;}
         else if (! StrCm (str, CC("`tre")))  {x =  48;   w = 17;}
         else if (! StrCm (str, CC("`sta")))  {x =  65;   w = 23;}
         else if (! StrCm (str, CC("`hap")))  {x =  88;   w = 25;}
         else if (! StrCm (str, CC("`sad")))  {x = 113;   w = 23;}
         else if (! StrCm (str, CC("`mad")))  {x = 136;   w = 24;}
         if (x)  Up.cnv.Blt (*Up.cue, (ubyt2)nx-W_Q, y, x, 0,
                                                        w, Up.cue->height ());
         else    Up.cnv.TextVC (nx-W_Q, y, str, CBLACK);
      }

   // chords (in their own column)
      if (_lrn.chd) {
         Up.cnv.TextVC (qx, 0, CC("Chds"), CBLACK);
         for (ne = _f.chd.Ln, p = 0;
                 (p < ne) && (_f.chd [p].time < tMn);  p++)  sw = (! sw);
         for (;  (p < ne) && (_f.chd [p].time < tMx);  p++) {sw = (! sw);
            y = Tm2Y (_f.chd [p].time, & co);
            Up.cnv.TextVC (qx, y, _f.chd [p].s, sw ? CSclD[0]:CSclD[7]);
         }
      }

   // draw LH shading
      t1 = tMn;   nt = 0;              // chase till tMn
      for (p = 0;  p < _lm.Ln;  p++)  {if (_lm [p].tm > tMn)  break;
                                       else  nt = _lm [p].nt;}
   // ok, draw em for our col
//TStr z1,z2,z3,z4;
      for (;  (p < _lm.Ln) && (_lm [p].tm < tMx);  p++) {
//DBG("LH  p=`d .tm=`s .nt=`s   t1=`s nt=`s",
//p, TmSt(z1,_lm[p].tm), MKey2Str(z2,_lm [p].nt),
//TmSt(z3,t1), MKey2Str(z4,nt));
         if ((nt > co.nMn) && (nt <= co.nMx)) {
            w = Nt2X (nt, & co, 'g') - nx;
            y = Tm2Y (t1, & co);   y2 = Tm2Y (_lm [p].tm, & co);
            if (y2 >= co.h)  y2 = co.h-1;
//DBG("1  x=`d y=`d w=`d h=`d y2=`d", nx, y, w, y2-y,y2);
            Up.cnv.Blt (*Up.lhmx, nx, y, w, y2-y, 0, 0, 1, 1);
         }
         t1 = _lm [p].tm;   nt = _lm [p].nt;
      }                                // continue last rect
      if ((t1 < tMx) && (nt > co.nMn) && (nt <= co.nMx)) {
            w = Nt2X (nt, & co, 'g') - nx;
            y = Tm2Y (t1, & co);   y2 = Tm2Y (_lm [p].tm, & co);
            if (y2 >= co.h)  y2 = co.h-1;
//DBG("2  x=`d y=`d w=`d h=`d y2=`d", nx, y, w, y2-y,y2);
            Up.cnv.Blt (*Up.lhmx, nx, y, w, y2-y, 0, 0, 1, 1);
      }

   // bar #s on top
      for (p = 0;  p < co.nBlk;  p++) {
         StrFmt (str, "`d", co.blk [p].bar);
         Up.cnv.TextC (nx+1, co.blk [p].y+3, str, CDBLU);
      }
   //__________________________________
   // ok, dump the note symbols
      for (p = 0;  p < co.nSym;  p++)  DrawSym (& co.sym [p], & co);

   //__________________________________
   // draw shown controls of (shown tracks  or  track"less"=dr#1)
      x = cx;                          // td to 1st drum trk else 255
      for (td = 255, t = 0;  t < nTrk;  t++)  if (TDrm (t))  {td = t;   break;}
      for (cc = 0;  cc < _f.ctl.Ln;  cc++)  if (_f.ctl [cc].sho) {
      // init ctl w str,typ,default value from cc.txt
         StrCp (cs, _f.ctl [cc].s);
         ccg = false;
         if ( (! StrCm (CC("ksig"), cs)) ||
              (! StrCm (CC("tsig"), cs)) || (! StrCm (CC("tmpo"), cs)) )
            ccg = true;
         df = 0;   vt = 'u';   cno = (Cfg.ntCo == 2) ? 0 : 7;
         for (ct = 0;  ct < NMCC;  ct++)  if (! StrCm (MCC [ct].s, cs))
            {df = MCC [ct].dflt;   vt = MCC [ct].typ;   break;}
//DBG(" cc=`d=`s dflt=`d typ=`c", cc, cs, df, vt);

      // tempo is super special...  _f.tpo has prescribed (non act) tmpo +-.25
         if (! StrCm (CC("tmpo"), cs)) {    // first get range +-.25 in tpMn,Mx
//TStr d1,d2,d3,d4;
           QColor qc= CScl [7];
            bug = true;
         // go thru prescribed tempo evs, get min,max and adj w clip for tpMn,Mx
            te = & _f.tpo [0];   ne = _f.tpo.Ln;   vl = 120;
            for (p = 0;  (p < ne) && (te [p].time < tMn); p++)  vl = te [p].val;
            t1 = 0;   tpMn = tpMx = TmpoAct (vl);

         // if dot at top, no prev val: gotta init tpMn,Mx in loop
            if ((p < ne) && (te [p].time == tMn))  t1 = NONE;
//DBG("  tmpo t1=`s p=`d/`d vl=`d", TmSt(d1, t1), p, ne, vl);
            for (     ;  (p < ne) && (te [p].time < tMx);  p++) {
               vl = TmpoAct (te [p].val);
               if (t1 == NONE)  {t1 = 0;   tpMn = tpMx = vl;}   // did init
               if (vl < tpMn)   tpMn = vl;
               if (vl > tpMx)   tpMx = vl;
            }
            tpMn -= (tpMn / 4);   tpMx += ( (tpMx / 4) + (((tpMx%4)>=2)?1:0) );
//DBG("  tmpo tpMn=`d tpMx=`d p=`d/`d", tpMn, tpMx, p, ne);

            t1 = NONE;   vl = 120;     // init ctl valu w default
            for (p = 0;  (p < ne) && (te [p].time < tMn);  p++)
               {t1 = te [p].time;   vl = te [p].val;}
            vl = TmpoAct (vl);
            if ((p < ne) && (te [p].time == tMn))  t1 = NONE;
//DBG("  tmpo init t1=`s vl=`d p=`d/`d", TmSt(d1, t1), vl, p, ne);

         // ok, draw em in our col
            for (;       (p < ne) && (te [p].time < tMx);  p++) {
               x2 = x+(th-2)*(TmpoAct (te [p].val)-tpMn)/(tpMx-tpMn);
               y2 = Tm2Y (             te [p].time, & co);
//DBG("  tmpo pTm=`s pVal=`d x2=`d y2=`d", TmSt(d1, t1), vl, x2, y2);
               if (t1 != NONE) {
                  x1 = x+(th-2)*(vl-tpMn)/(tpMx-tpMn);     // vline to prv vl
                  y  = Tm2Y (t1, & co);                    // hline to prv tm
                  Up.cnv.RectF (x1, y, 2, y2-y+1, qc);
                  if      (x1 < x2) Up.cnv.RectF (x1, y2, x2-x1+1, 2, qc);
                  else if (x1 > x2) Up.cnv.RectF (x2, y2, x1-x2+1, 2, qc);
               }
               Up.cnv.RectF (x2-1, y2-1, 4, 4, qc);   // dot at x2,y2
               t1 = te [p].time;   vl = TmpoAct (te [p].val);
            }                          // continue val to end of col
//DBG("  tmpo done t1=`s vl=`d", TmSt(z1, t1), vl);
            if ((t1 == NONE) || (t1 < tMn))  t1 = tMn;
            y = Tm2Y (t1, & co);   y2 = Tm2Y (tMx, & co);
            Up.cnv.RectF (x+(th-2)*(vl-tpMn)/(tpMx-tpMn), y, 2, y2-y+1, qc);
         }
         else
            for (t = 0;  t < nTrk;  t++)  if ( (   ccg  && (t == td)) ||
                                               ((! ccg) && TSho (t)) ) {
         // chase till tMn - get last time,val
            t1 = NONE;   vl = df;       // init ctl valu w default
            e = trk [t].e;   ne = trk [t].ne;
            for (p = 0;  (p < ne) && (e [p].time < tMn);  p++)
               if (e [p].ctrl == (0x80|cc)) {t1 = e [p].time;
                                             vl = e [p].valu | (e [p].val2<<7);}
//TStr z1;
//DBG("cc=`s  init t1=`s vl=`d p=`d/`d", cs, TmSt(z1, t1), vl, p, ne);
         // ok, draw ctrl evs in our col
            for (;       (p < ne) && (e [p].time < tMx);  p++)
                                                  if (e [p].ctrl == (0x80|cc)) {
//DBG("cc  p=`d/`d pTm=`s pVl=`d", p, ne, TmSt(z1,e [p].time), e [p].valu);
               if      (vt == 'x') {
                  CtlX2Str (str, cs, & e [p]);
                  Up.cnv.TextVC (x, Tm2Y (e [p].time, & co), str, CBLACK);
               }
               else if (vt == 'o') {
                  y2 = Tm2Y (e [p].time, & co);
//TStr z1;
//DBG("cc  t1=`s vl=`d x2=`d y2=`d", TmSt(z1, t1), vl, x2, y2);
                  if (t1 != NONE) {
                     if (t1 < tMn)  t1 = tMn;
                     y = Tm2Y (t1, & co);   // continue vert line
                     if (vl >= 64)  Up.cnv.RectF (x, y, 2, y2-y+1, CScl [cno]);
                  }
                  t1 = e [p].time;   vl = e [p].valu;
               }
               else {                  // s u - like tmpo but easier scaling
                  x1 = x+(th-2)* vl/127;
                  x2 = x+(th-2)* e [p].valu/127;
                  y2 = Tm2Y (e [p].time, & co);
//TStr z1;
//DBG("cc  t1=`s vl=`d x2=`d y2=`d", TmSt(z1, t1), vl, x2, y2);
                  if (t1 != NONE) {
                     if (t1 < tMn)  t1 = tMn;
                     y = Tm2Y (t1, & co);   // continue vert line
                     if (vl != df)  Up.cnv.RectF (x1, y, 2, y2-y+1, CScl [cno]);
                  }
                  if      (x1 < x2)
                     Up.cnv.RectF (x1, y2, x2-x1+1, 2, CScl [cno]);
                  else if (x1 > x2)
                     Up.cnv.RectF (x2, y2, x1-x2+1, 2, CScl [cno]);
                  Up.cnv.RectF (x2-1, y2-1, 4, 4, CScl [cno]);  // dot at x2,y2
                  t1 = e [p].time;   vl = e [p].valu;
               }
            }
            if ((vt != 'x') && (vl != df)) {     // continue ctl val to bot col
//DBG("cc  done t1=`s vl=`d", TmSt(z1, t1), vl);
               if ((t1 == NONE) || (t1 < tMn))  t1 = tMn;
               y = Tm2Y (t1, & co);   y2 = Tm2Y (tMx, & co);
               if (vt == 'o')
                  {if (vl >= 64)  Up.cnv.RectF (x,  y, 2, y2-y+1, CScl [cno]);}
               else  Up.cnv.RectF (x+(th-2)*vl/127, y, 2, y2-y+1, CScl [cno]);
            }
            if (Cfg.ntCo == 2)  if (++cno >= 12)  cno = 0;
         }
         x += th;
      }

   //__________________________________
   // bugs on top of tmpo
      if (bug) {
        ubyt2 dw, dh, w = Up.bug->width (), h = Up.bug->height ();
         ne = _f.bug.Ln;
         for (p = 0;  (p < ne) && (_f.bug [p].time < tMn);  p++)  ;
         for (;       (p < ne) && (_f.bug [p].time < tMx);  p++) {
            hit = (sbyte)Str2Int (_f.bug [p].s) - 1;
            dh = h/2 + hit * 5*h/4 / 8;
            dw = w*dh/h + ((w*dh%h >= h/2) ? 1:0);
            Up.cnv.Blt (*Up.bug, cx, Tm2Y (_f.bug [p].time, & co), dw, dh,
                                                              0, 0, w,  h);
         }
      }

   //__________________________________
   // fade nonloop if in prac mode
      if ((PRAC || (_lrn.pLrn == LPRAC))) {
         if ((_lrn.lpBgn > tMx) || (_lrn.lpEnd < tMn))     // all faded
            Up.cnv.Blt (*Up.fade, co.x, 0, x-co.x, co.h, 0, 0, 1, 1);
         else {
            lt = _lrn.lpBgn;   y  = Tm2Y (tMn, & co);
                               y2 = Tm2Y (lt,  & co);
            if ((lt > tMn) && (lt < tMx))
               Up.cnv.Blt (*Up.fade, co.x, y, x-co.x, y2-y, 0, 0, 1, 1);

            lt = _lrn.lpEnd;   y  = Tm2Y (lt,  & co);
                               y2 = Tm2Y (tMx, & co);
            if ((lt > tMn) && (lt < tMx))
               Up.cnv.Blt (*Up.fade, co.x, y, x-co.x, y2-y, 0, 0, 1, 1);
         }
      }
   }
   if (Cfg.ntCo == 1) {                // draw velocity scale?
      w = Up.w / 128;   if (Up.w % 128)  w++;
      for (ubyt4 c = 0;  c < 128;  c++)
         Up.cnv.RectF ((ubyt2)(c * Up.w / 128), Up.h-6, w, 4, CRng [c]);
   }
   DrawRec (true, pp);
//TRC("DrawPg end");
}


//------------------------------------------------------------------------------
void Song::DrawNow ()
// refresh rec;  draw now line at right spot, optionally with red,grn dots
{ ubyte t, tr, mt, nt, f, nn, g;
  ubyt2 nx, nw, cx, ww, x, yNow, yOvr;
  ubyt4 c, tMn, tMx, n, pn, p, pt;
  bool  up;
  DownRow *dn;
  PagDef  *pg = & _pag [0];
  ColDef   co;
  NtDef   *np;
  TrkRow  *tk = & _f.trk [0];
//TRC("DrawNow _pg=`d", _pg);
   if (! (p = _pg))  return;           // don't know pg at the moment :/

   Up.cnv.bgn (Up.pm);   Up.tcnv.bgn (Up.tpm);   // need pm too for DrawRec
   p--;   pn = _pNow;   n = _rNow;
//TStr d1,d2;
//DBG("DrawNow pg=`d pNow=`s rNow=`s",
//p, TmSt (d1,pn), TmSt (d2,n));
   if (pn)  DrawRec (false, p);        // pm gets JUST new rec nts

   for (c = 0;  c < pg [p].nCol;  c++) {         // find our col
      tMn = pg [p].col [c].blk [0].tMn;
      tMx = pg [p].col [c].blk [pg [p].col [c].nBlk-1].tMx;
      if (n >= tMn) {
         if (n < tMx-M_WHOLE/32)  break;
         if (n < tMx)  n = tMx;        // too low - bump to top of next col
      }
   }
   if (c >= pg [p].nCol)  c = pg [p].nCol - 1;   // but DON't bump page !!
   MemCp (& co, & pg [p].col [c], sizeof (co));  // load column specs
   yNow = Tm2Y (n, & co);              // REAL y
   yOvr = yNow - (H_NW-1);             // y of bmp overlay
   nx = Nt2X (co.nMn, & co);   cx = CtlX (& co);   nw = cx - nx;
   nx -= co.x;   cx -= co.x;           // 0 is offset WITHIN the col HERE !!

// bg bits => tCnv
   Up.tcnv.Blt (*Up.pm, 0, 0,  co.x, yOvr,  co.w, H_T);

// green fade now ln
   Up.tcnv.Blt (*Up.now,  nx, 0, nw, H_NW, 0, 0,
                                           Up.now->width (), Up.now->height ());
   if (_lrn.POZ) {
//TStr d1,d2,d3;
//DBG("DOTS poz=`b Cfg.lrn=`d pg=`d pNow=`s rNow=`s tmr=`s",
//_lrn.POZ, Cfg.lrn, p, TmSt (d1,pn), TmSt (d2,n),
//TmSt (d3,_timer->Get ()));
   // if poz'd, draw dots n no x,o in those lrns
      Up.tcnv.SetMode ('t');
      dn = & _dn [_pDn];
      pt = _pDn ? _dn [_pDn-1].time : 0;
      g = (DnOK () == 'a') ? 2 : 1;    // again gets bigger grn dots

      if (co.nDrm) {                   // none of this if we ain't got no drums
      // show "off the chart" drum rec notes on border
         for (up = true, nt = 0;  nt < 128;  nt++)
            if (_lrn.rec [1][nt].tm) {
               for (mt = 0;  mt < co.nDrm;  mt++) {
                  tr = co.dMap [mt];
                  if (tk [tr].drm == nt)  break;
               }
               if (mt >= co.nDrm)  {up = false;   break;}
            }
         if (! up)  Up.tcnv.Blt (*Up.dot, cx - co.nDrm*W_NT, H_NW-9,
                                                                 5, 0,  11, 16);
      // dot drums notes
         for (x = cx - co.nDrm*W_NT, mt = 0;  mt < co.nDrm;  mt++) {
            tr = co.dMap [mt];   nt = tk [tr].drm;
            for (np = NULL, nn = 0;  nn < dn->nNt;  nn++)
               if ( (tk [dn->nt [nn].t].chn == 9) && (dn->nt [nn].nt == nt) )
                  {np = & dn->nt [nn];   break;}
            t = 3;                     // default to no dot;  check grn;  red
            if (np)  {if ((g == 2) || (_lrn.rec [1][nt].tm <= pt))  t = g;}
            else      if (_lrn.rec [1][nt].tm)  t = 0;
            if (t < 3)  Up.tcnv.Blt (*Up.dot, x+W_NT/2-8, H_NW-9,
                                                              t*16, 0,  16, 16);
            x += W_NT;
         }
      }

   // show "off the chart" melo rec notes on borders
      for (up = true, nt = co.nMn-1;  nt >= MKey (CC("0a"));  nt--)
         if (_lrn.rec [0][nt].tm)  {up = false;   break;}
      if (! up)  Up.tcnv.Blt (*Up.dot, nx,    H_NW-9,  5, 0,  11, 16);
      for (up = true, nt = co.nMx+1;  nt <= MKey (CC("8c"));  nt++)
         if (_lrn.rec [0][nt].tm)  {up = false;   break;}
      if (! up)  Up.tcnv.Blt (*Up.dot, cx-11, H_NW-9,  0, 0,  11, 16);

   // dot melodic notes - just red;  green next
      for (x = 0, nt = co.nMn;  nt <= co.nMx;  nt++, x += W_NT) {
         for (np = NULL, nn = 0;  nn < dn->nNt;  nn++)
            if ( (tk [ dn->nt [nn].t].chn != 9) && (dn->nt [nn].nt == nt) )
               {np = & dn->nt [nn];   break;}
      // red dot if down per .rec[], not in _dn, not held (in .nt[])
         if (_lrn.rec [0][nt].tm && (! np) && (! (_lrn.nt [nt] & 0x80)) ) {
            x = Nt2X (nt, & co) - co.x;   ww = W_NT/2;
            if (KeyCol [nt%12] == 'w')    ww = 24/2;
            Up.tcnv.Blt (*Up.dot, x+ww-8, H_NW-9,  0, 0,  16, 16);
         }
      }
   // green dots if in _dn, no .rec[] (or again)
      for (nn = 0;  nn < dn->nNt;  nn++)  if (tk [dn->nt [nn].t].chn != 9) {
         np = & dn->nt [nn];   nt = np->nt;
         if ( (g == 2) || (_lrn.rec [0][nt].tm <= pt) ) {
            x = Nt2X (nt, & co) - co.x;   ww = W_NT/2;
            if (KeyCol [nt%12] == 'w')    ww = 24/2;
            Up.tcnv.Blt (*Up.dot, x+ww-8, H_NW-9,  g*16, 0,  16, 16);
            if ((f = tk [np->t].e [np->p].val2 & 0x1F))
               DrawFng (x+ww/2, H_NW-8, f, 't');
         }
      }
   }
   _pNow = n+1;
   Up.tpos.setLeft  (co.x);   Up.tpos.setTop    (yOvr);
   Up.tpos.setWidth (co.w);   Up.tpos.setHeight (H_T);
//TRC("DrawNow end");
   Up.cnv.end ();   Up.tcnv.end ();
   emit sgUpd ("nt");
}


void Song::Draw (char all)
// see where _now puts us page/trans wise.  DrawPg/trans if diff than last time
{ ubyt4 n, p, np;
  ubyt2 x, y, w, h,  xa, wa, ha, xb, yb;
  ubyte t;
  TStr  d1;
  PagDef *pg = & _pag [0];
  ColDef  co;
   if (Up.pm == nullptr)  return;
   Up.cnv.bgn (Up.pm);   Up.tcnv.bgn (Up.tpm);
//TRC("Draw _rNow=`s np=`d _pg=`d _tr=`d all=`c",
//TmSt(d1,_rNow), _pag.Ln, _pg, _tr, all);
   if (_pag.Ln == 0) {                 // nothin therez yet - cls
      Up.cnv.RectF  (0, 0, Up.w, Up.h, CWHITE);
      MemSet (& Up.tpos, 0, sizeof (QRect));
      Up.cnv.end ();   Up.tcnv.end ();
      return;
   }
   for (n = _rNow, np = _pag.Ln,  p = 0;  p < np;  p++)
      if (                (n >= pg [p  ].col [0].blk [0].tMn) &&
          ((p+1 >= np) || (n <  pg [p+1].col [0].blk [0].tMn)) )  break;
   if (p >= np)  --p;
   t = ((p+1 < np) && (n >= Bar2Tm (pg [p+1].col [0].blk [0].bar-1))) ? 1:0;
   if (all || (p+1 != _pg) || (t != _tr)) {
//TRC(" new pg,tr: _pg=`d _tr=`d p=`d t=`d", _pg, _tr, p, t);
      if (all || (p+1 != _pg))  {_pg = p+1;   DrawPg (p);}
      if (t) {                         // trans - draw next pg, last bar on top
         MemCp (& co, & pg [p].col [pg [p].nCol-1], sizeof (co));
         x = co.x;   y = co.blk [co.nBlk-1].y;
         w = co.w;   h = co.blk [co.nBlk-1].h;
         Up.tcnv.Blt (*Up.pm, 0, 0, x, y, w, h); // tCnv gets curr pg last bar
         _pg = p+2;   DrawPg (p+1);              // draw next pg
         _pg = p+1;                              // restore _pg
         ha = pg [p+1].h;   xa = 0;   wa = Up.w; // draw blank area around bar
         xb = (x < 80) ? 0 : (x-80);
         yb = (y < 80) ? 0 : (y-80);
         Up.cnv.RectF (xb, yb, wa-(xb-xa), ha-yb, CWHITE);
         Up.cnv.Blt (*Up.tpm, x, y, 0, 0, w, h); // stamp last bar on top
         _rc.setLeft  (x);   _rc.setTop    (y);
         _rc.setWidth (w);   _rc.setHeight (h);
      }
      else  MemSet (& _rc, 0, sizeof (QRect));
      _tr = t;
      _pNow = 0;                       // naw fer DrawRec cuz DrawPg did it all
   }
   Up.cnv.end ();   Up.tcnv.end ();    // DrawNow sometimes needs em on its own
                                       // and it'll kick nt update
   DrawNow ();
//TRC("Draw end");
}

void Song::Arp (ArpDef *arp, char getput)
{ ulong p, tp, dur;
  slong t1, t2, tm, nu, de;
  ubyte t, bss, tmp, stp, stp2, oct, oct2, nt, cPan;
  sbyte vP;
  bool  stpd, pan1;
  TStr  s;
  char *s2;
  TrkEv  *e;
  TrkDef *tr;
   if (getput == 'g') {                     // get
      MemSet (arp, 0, sizeof (ArpDef));     // init w defaults
      arp->on   = 1;
      arp->dur1 = 4;                        // 1/16
      arp->oct1 = 2;     arp->oct2 = 3;     // 3..4
      arp->vel1 = 100;   arp->vel2 = 60;
      arp->pan1 = -64;   arp->pan2 = 63;

      if (! (s2 = StrSt (_dsc, "arp=b,")))  return;
      s2 += 6;                              // load from dsc
      arp->on   = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->dur1 = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->dur2 = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->stp  = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->oct1 = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->oct2 = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->vel1 = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->vel2 = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->pan  = (ubyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->pan1 = (sbyte)Str2Int (s2, & s2);   if (*s2++ != ',')  return;
      arp->pan2 = (sbyte)Str2Int (s2);
      return;
   }
   NotesOff ();                        // SHUSH !
// else put - save in dsc then do it (or not)
   sprintf (s, "arp=b,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
      arp->on, arp->dur1, arp->dur2, arp->stp, arp->oct1, arp->oct2,
      arp->vel1, arp->vel2, arp->pan, arp->pan1, arp->pan2);
   DscUpd (s);

// set t to prv arp trk wiped, else make new one
   for (t = 0; t < _nTrk; t++)  if (! MemCm (_trk [t].name, "arpeggio", 8))
      {tr = & _trk [t];   EvDel (t, 0, tr->ne);   tr->p = tr->dur = 0;   break;}
   if (t >= _nTrk)  {t = AddTrkM ();   if (t == 255)  return;}

   e = _trk [t].e;   tp = 0;
   _trk [t].name.Set ("arpeggios:)");
   _trk [t].rec = false;   _trk [t].rDev = _trk [t].rChn = 0;
   if (! arp->on)  {_lrn.pBar = 0;   _c->wPno->Updt ();   Show ();   return;}

   dur = M_WHOLE / (1 << arp->dur1);   if (arp->dur2 == 1) dur = dur * 3 / 2;
                                       if (arp->dur2 == 2) dur = dur * 2 / 3;
   if (arp->pan)  cPan = CtlUpd ("Pan", t);

   for (p = 0;  p < _nChd;  p++) {
   // get bass note:  (oct)f..(oct+1)e
      *s = '1';   StrCp (s+1, _chd [p].s);
      bss = MKey2Int (s, & s2);
      if (! bss)                 continue;       // lame bss
      if (bss <= M_NT(M_E,1))  bss += 12;
      bss += (arp->oct1 * 12);
      oct = 0;                         // octave offset step
      oct2 = arp->oct2 - arp->oct1;

   // get chord template
      for (tmp = 0;  tmp < BITS (ChdTmp);  tmp++)
         if (! MemCm (s2, ChdTmp [tmp].lbl, StrLn (ChdTmp [tmp].lbl),
                      'x'))  break;
      if (tmp >= BITS (ChdTmp))  continue;       // lame tmp

   // init step steppin
      for (stp2 = 0;  ChdTmp [tmp].tmp [stp2+1] != 'x';  stp2++)  ;
      if (arp->stp == 1) {stp = stp2;   oct = oct2;   stpd = false;}
      else               {stp = 0;      oct = 0;      stpd = true;}

   // get time range t1..t2
      t1 = _chd [p].time;
      t2 = (p+1 < _nChd) ? _chd [p+1].time : Bar2Tm (_bEnd);

   // ok, make em - step every dur, bumping tmp's step
      for (pan1 = true, tm = t1;  tm+((slong)dur*3/4) < t2;  pan1 = ! pan1) {
         nt = bss + oct*12 + ChdTmp [tmp].tmp [stp];
         if (arp->pan) {
         // step to next pan val
            switch (arp->pan) {
            case 1:                          // step      1..2
               vP = (sbyte)((int)arp->pan1 +
                            ((int)arp->pan2 - (int)arp->pan1)*(tm-t1)/(t2-t1));
               break;
            case 2:                          // return    1..2..1
               nu = 2 * (tm - t1);   de = t2 - t1;    //  0..2 => 0..1..0
               if (nu <= de)
                    vP = (sbyte)((int)arp->pan1 +
                                 ((int)arp->pan2 - (int)arp->pan1) * nu/de);
               else vP = (sbyte)((int)arp->pan2 -
                                 ((int)arp->pan2 - (int)arp->pan1) *
                                                                (nu-de)/de);
//DBGF("p1=%d p2=%d tm=%d t1=%d t2=%d nu=%d de=%d vP=%d",
//arp->pan1,arp->pan2,tm,t1,t2,nu,de,vP);
               break;
            case 3:                          // pingpong  just swap btw 1 n 2
               vP = pan1 ? arp->pan1 : arp->pan2;
               break;
            case 4:                          // out2in    -64,63 .. 0,0
               if (pan1)  vP = (sbyte)(-64 * (t2-tm)/(t2-t1));
               else      {vP++;   vP = -vP;}
               break;
            case 5:                          // in2out    0,0 .. -64,63
               if (pan1)  vP = (sbyte)(-64 * (tm-t1)/(t2-t1));
               else      {vP++;   vP = -vP;}
               break;
            };

            if (! EvIns (t, tp, 1))  return;     // pan ev
            e [tp].time = tm;   e [tp].ctrl = cPan;
            e [tp].valu = (ubyte)(64+vP);
            e [tp].val2 = e [tp].x = 0;
            tp++;
         }

         if (! EvIns (t, tp, 1))  return;   // note down
         e [tp].time = tm;   e [tp].ctrl = nt;   e [tp].valu = 0x80 |
                   (ubyte)((int)arp->vel1 +
                          ((int)arp->vel2 - (int)arp->vel1)*(tm-t1)/(t2-t1));
         e [tp].val2 = 0;    e [tp].x = 2+stp;
//TStr db1;
//DBGF("stp=%d oct=%d nt=%s vel=%d",stp,oct,MKey2Str(db1,nt),e[tp].valu & 0x7F);
         tp++;

         if (! EvIns (t, tp, 1))  return;   // note up
         e [tp].time = tm + dur*3/4;   e [tp].ctrl = nt;   e [tp].valu = 64;
         e [tp].val2 = e [tp].x = 0;
         tp++;

         if (stpd) {                   // currently goin up
            if      (stp != stp2)  stp++;
         // eeerch!!  restart or start back down
            else if (arp->stp != 2) {
               if (oct < oct2) oct++;   else oct = 0;
               stp = 0;
            }
            else {
               if (oct < oct2)  oct++;
               else            {stp--;   stpd = ! stpd;}
            }
         }
         else {                        // currently goin down, baby
            if      (stp)  stp--;
            else if (arp->stp != 2) {
               if (oct) oct--;   else oct = oct2;
               stp = stp2;
            }
            else {
               if (oct)  oct--;
               else     {stp++;   stpd = ! stpd;}
            }
         }
         tm = (tm / dur) * dur + dur;
      }                                // bump tm to next dur interval
   }
   TmHop (0);
}

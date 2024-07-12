// songtime.cpp - Time funcs of Song

ubyt4 Song::Bar2Tm (ubyt2 b, ubyte bt)      // get song time of a bar + bt
{ ubyt2 s = 0;
  ubyt4 t;
   while (((ubyt4)s+1 < _tSg.Ln) && (_tSg [s+1].bar <= b))  s++;
   if ((s >= _tSg.Ln) || (_tSg [s].bar > b))
      t = (b-1) * M_WHOLE;
      return t + (b-1) * M_WHOLE / 4;  // none or none apply yet - use 4/4

   b -= _tSg [s].bar;                  // got tsig so offset from it
   t = _tSg [s].time + (b * M_WHOLE * _tSg [s].num / _tSg [s].den);
   return    t + (b-1) * M_WHOLE / 4;
}


ubyt4 Song::Str2Tm (char *s)           // turn rel str time to abs ubyt4
{ ubyt2 br = (ubyt2)Str2Int (s, & s);
   if (*s++ != '.')  return Bar2Tm (br, 1);
  ubyte bt = (ubyte)Str2Int (s, & s);
   if (*s++ != '.')  return Bar2Tm (br, bt);
  ubyte tk = (ubyte)Str2Int (s);
   return Bar2Tm (br, bt) + tk;
}


char *Song::TmStr (char *str, ubyt4 tm, ubyt4 *tL8r)
// put song time into a string w bar.beat;  maybe return time of next bt
{ ubyt4 dBr, dBt, l8r;
  ubyt2    s = 0, br, bt;
   while ((s+1 < _tSg.Ln) && (_tSg [s+1].time <= tm))  s++;
   if ((s >= _tSg.Ln) || (_tSg [s].time > tm)) {
      dBt = M_WHOLE / 4;               // none apply yet - use 4/4/1
      dBr = dBt     * 4;
      br  = (ubyt2)(1 + (tm / dBr));
      bt  = (ubyt2)(1 + (tm % dBr) / dBt);
      l8r = (br-1) * dBr + bt * dBt;
   }
   else {
      dBt = M_WHOLE / _tSg [s].den;
      dBr = dBt     * _tSg [s].num;
      br  = (ubyt2)(_tSg [s].bar + (tm - _tSg [s].time) / dBr);
      bt  = (ubyt2)(1 +           ((tm - _tSg [s].time) % dBr) / dBt);
      l8r = _tSg [s].time + (br - _tSg [s].bar) * dBr + dBt * bt;
   }
   StrFmt (str, "`04d.`d", br, bt);
   if (tL8r) *tL8r = l8r;
   return str;
}

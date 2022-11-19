// RepRpt.cpp - repertoire report

#include <ui.h>
#include "rc\resource.h"

#define MAX_SONG  (8000)
#define MAX_PRAC  (80)
#define MAX_NOTE  (80)
#define HOM       "http://rep.PianoCheetah.com"

typedef struct {
   TStr   fn, src;                     // fn beyond Practice\ ...
   ulong  bgnD, endD, maxD;            // min,complete,max day from pract,etc
   struct {ulong ym, d;}      pr [MAX_PRAC];   ubyte nPr;
   struct {ulong d;  TStr s;} nt [MAX_NOTE];   ubyte nNt;
} RepLstDef;

RepLstDef Lst [MAX_SONG];              // hopefully that's enough :/
ulong     Len;
TStr      Top;                         // top level dir (...\\Practice)

ubyte     Mid [16*1024];               // mem to hold start of .mid file
ulong     MidLen, MidP;                // len of buf and pos we're parsin' at

#define   MFLAG  (sizeof(TStr)-1)      // list of mp3 files in Recorded
TStr      MLst [MAX_SONG];   ulong MLen;
TStr      RLst [MAX_SONG];   ulong RLen;  // "  " mid files in Recorded
TStr      SLst [MAX_SONG];   ulong SLen;  // source list
char      Name [MAX_SONG][26];  ulong NLen;
char      HHdr [8*1024], HSep [8*1024], HTrl [8*1024];

char  Buf [128*1024];
ulong BLn;


int CmpAsc (void *p1, void *p2)        // just StrCm em
{ char *s1 = (char *)p1, *s2 = (char *)p2;
   return  StrCm (s1, s2);
}


//------------------------------------------------------------------------------
class Doer: public Thread {
public:
   Doer (CtlEdit *m): Thread (App.wndo), _msg (m)  {}
private:
   DWORD Go ();
   void  Quilt (INet *i);
   CtlEdit *_msg;
};


void Doer::Quilt (INet *i)
// rebuild rep.pc.com's index.html and an .html per name w/ index.html
{ char  m [16*1024], *ch;
  ulong p, j;
  TStr  ln, wn, fn, fn2;
  File  f;

// back up to top
   i->ChDir ("..");

// load in list.txt but strip names w no name/index.html
   BLn = i->Get (HOM "/list.txt", Buf, sizeof (Buf));
   Buf [BLn] = '\0';   NLen = 0;
   for (p = 0; p < BLn;) {
      p = NextLn (ln, Buf, BLn, p);
      if (ln [0] == '_')  continue;
      StrCp (Name [NLen++], ln);
   // gotta 404?  decr NLen back
      StrFmt (wn, HOM "/`s/index.html", ln);
      m [i->Get (wn, m, sizeof (m))] = '\0';
      if (StrSt (m, "PianoCheetah - home"))  NLen--;
   }

// load header template
   HHdr [i->Get (HOM "/__/hdr.html", HHdr, sizeof (HHdr))] = '\0';

// quilt em into etc/ind.html locally
   App.Path (fn, 'e');   StrAp (fn, "\\ind.html");
   for (p = 0; p < NLen; p++) {
      StrCp (Buf, HHdr);

   // replace NM => this user name
      while (ch = StrSt (Buf, "_NM"))
         {StrCp (ch+StrLn (Name [p])-3, ch);
          MemCp (ch, Name [p], StrLn (Name [p]));}

   // tack on built up submenu html
      for (j = 0; j < NLen; j++) {
         StrFmt (m,
" <li><a href=\"`s.html\"`s><span>`s</span></a></li>\n",
            Name [j], (j == p) ? " class=\"here\"" : "", Name [j]);
//DBG("p=`d j=`d m=`s", p, j, m);
         StrAp (Buf, m);
      }
      StrAp (Buf,
"</ul>\n\n\n"
"<pre class=\"fix\"><div id=\"main\">\n"
      );

   // tack on userinfo.cfg
      App.Path (fn2, 'e');   StrAp (fn2, "\\userinfo.cfg");
      BLn = StrLn (Buf);
      Buf [BLn + f.Load (fn2, & Buf [BLn], sizeof (Buf)-BLn)] = '\0';

   // load main body of index.html (at end of Buf)
      BLn = StrLn (Buf);
      StrFmt (wn, HOM "/`s/index.html", Name [p]);
      Buf [BLn + i->Get (wn, & Buf [BLn], sizeof (Buf)-BLn)] = '\0';

   // scoot body contents on top of old header
      if (ch = StrSt (Buf, "<body><pre class=\"fix\">\n"))
         StrCp (& Buf [BLn], ch+23);

   // replace ="Device/etc => ="__
   //         ="Recorded   => ="NAME/Recorded and
   //         =Recorded    => =NAME/Recorded
   // replace NM => this user name
      while (ch = StrSt (Buf, "=\"Device/etc"))
         {MemCp (ch+2, "__", 2);   StrCp (ch+4, ch+12);}
      while (ch = StrSt (Buf, "=\"Recorded"))
         {StrCp (ch+2+StrLn (Name [p])+1, ch+2);
          StrCp (ch+2, Name [p]);   *(ch+2+StrLn (Name [p])) = '/';}
      while (ch = StrSt (Buf, "=Recorded"))
         {StrCp (ch+1+StrLn (Name [p])+1, ch+1);
          StrCp (ch+1, Name [p]);   *(ch+1+StrLn (Name [p])) = '/';}

   // dump it to rep.pc.com;  also write it as index.html if we're 1st in list
      f.Save (fn, Buf, StrLn (Buf));
      StrFmt (wn, "`s.html", Name [p]);
      i->Put  (fn, wn);   if (p == 0)  i->Put (fn, "index.html");
   }
   if (NLen)  f.Kill (fn);
}


DWORD Doer::Go ()
{ TStr  pass, name, ln, fn, wn, fnl [300];
  char *ch;
  ulong p, n, i, tot;
  bool  got, rc;
  File  f;
  FDir  d;
TRC("{ Doer::Go");
   InitMsgQ ();

_msg->Set ("hookin to " HOM);
DBG       ("hookin to " HOM);
   *pass = '\0';
   { INet inp;
      inp.Get (HOM "/pianocheetahup.txt", pass, sizeof (pass));
      Chomp (pass);
      if (StrCm (pass, "NO") == 0)
         Die ("Hmmm, Steve has disabled uploading for now...  Sorry...");
   }
  INet in ("pianocheetahup", pass, "rep.PianoCheetah.com");
   if (GotClose ())  {PostDadW (MSG_CLOSE, 0, 0);   return 0;}

   App.CfgGet ("username", name);   Chomp (name);
   App.CfgGet ("userpass", pass);   Chomp (pass);
   if (*name == '\0')  Die ("Hmm, your PianoCheetah username ain't set :(");

// get rep.pc.com/list.txt and find username
   BLn = in.Get (HOM "/list.txt", Buf, sizeof (Buf));
   Buf [BLn] = '\0';
   for (got = false, p = 0;  (! got) && (p < BLn);)
      {p = NextLn (ln, Buf, BLn, p);   if (! StrCm (ln, name))  got = true;}

// if got name, check pass;  else set up new guy :)
   if (got) {
_msg->Set ("checkin' password");
DBG       ("checkin' password");
      StrFmt (wn, HOM "/__pass/`s", name);
      ln [in.Get  (wn, ln, sizeof (ln))] = '\0';
      if (StrCm (ln, pass))
         Die ("Wrong password:(   (or username already taken?)   "
              "email stephen.hazel@gmail.com if you forgot it");
   }
   else {
_msg->Set ("Welcome!!  :)  setting up new username...");
DBG       ("Welcome!!  :)  setting up new username...");
   // append name to rep.pc.com/list.txt
      StrAp (Buf, name);    StrAp (Buf, "\n");
      App.Path (fn, 'e');   StrAp (fn, "\\list.txt");
      f.Save (fn, Buf, StrLn (Buf));
      in.Put (fn, "list.txt");

   // make username dir and pass file
      in.MkDir (name);
      App.Path (fn, 'e');   StrAp (fn, "\\userpass.cfg");
      StrFmt (wn, "__pass/`s", name);
      in.Put (fn, wn);
   }

// cd username dir
   if (! in.ChDir (name))  Die (
                    "hmm, can't find your dir - email stephen.hazel@gmail.com");
_msg->Set ("deleting old stuff");
DBG       ("deleting old stuff");
   in.Wipe ();

_msg->Set ("putting info.txt, index.html, etc");
DBG       ("putting info.txt, index.html, etc");
   App.Path (fn, 'e');   StrAp (fn, "\\userinfo.cfg");
   in.Put   (fn,                          "info.txt");
   App.Path (fn, 'd');   StrAp (fn, "\\index.html");
   in.Put   (fn,                      "index.html");

   in.MkDir ("Recorded");
   in.MkDir ("Device");
   in.MkDir ("Device/etc");
   App.Path (fn, 'e');   StrAp (fn, "\\player_mp3.swf");
   in.Put   (fn,           "Device/etc/player_mp3.swf");
   App.Path (fn, 'e');   StrAp (fn, "\\site.js");
   in.Put   (fn,           "Device/etc/site.js");
   App.Path (fn, 'e');   StrAp (fn, "\\site.css");
   in.Put   (fn,           "Device/etc/site.css");
   if (GotClose ())  {in.Wipe ();   Quilt (& in);
                      PostDadW (MSG_CLOSE, 0, 0);   return 0;}

// make any subdirs under Recorded
   for (p = 0; p < RLen; p++) {
      StrCp (ln, RLst [p]);
      for (ch = ln;  ch = StrCh (ch, '\\');  ch++) {
         *ch = '\0';
         StrCp (wn, "Recorded/");   StrAp (wn, ln);  in.MkDir (wn);
         *ch = '\\';
      }
   }
   for (p = 0; p < MLen; p++) {
      StrCp (ln, MLst [p]);
      for (ch = ln;  ch = StrCh (ch, '\\');  ch++) {
         *ch = '\0';
         StrCp (wn, "Recorded/");   StrAp (wn, ln);  in.MkDir (wn);
         *ch = '\\';
      }
   }

   for (p = 0; p < RLen; p++) {
      if (GotClose ())  {in.Wipe ();   Quilt (& in);
                         PostDadW (MSG_CLOSE, 0, 0);   return 0;}
      StrFmt (fn, "Recorded file `d of `d: `s.mid",
                  p+1, RLen+MLen, RLst [p]);
_msg->Set (fn);
DBG       (fn);
      StrFmt (fn, "`s\\Recorded\\`s.mid", App.Path (ln, 'd'), RLst [p]);
      StrFmt (wn,     "Recorded/`s.mid", ReplCh (RLst [p], '\\', '/'));
      rc = in.Put (fn, wn);
   }

   for (p = 0; p < MLen; p++) {
      if (GotClose ())  {in.Wipe ();   Quilt (& in);
                         PostDadW (MSG_CLOSE, 0, 0);   return 0;}
      StrFmt (fn, "Recorded file `d of `d: `s.mp3",
              RLen+p+1, RLen+MLen, MLst [p]);
_msg->Set (fn);
DBG       (fn);
      StrFmt (fn, "`s\\Recorded\\`s.mp3", App.Path (ln, 'd'), MLst [p]);
      StrFmt (wn,     "Recorded/`s.mp3", ReplCh (MLst [p], '\\', '/'));
      rc = in.Put (fn, wn);
   }

// now, the d/l stuff and list.txt;
   *Buf = '\0';
   RLen = Len;                         // reuse RLst to sort Lst[] :/
   for (p = 0; p < RLen; p++)  StrCp (RLst [p], Lst [p].fn);
   Sort (RLst, RLen, sizeof (RLst [0]), CmpAsc);

// count up .mid + .zips we'll upload
   for (tot = p = 0; p < RLen; p++) {
      tot++;
      StrFmt (fn, "`s\\Practice\\`s", App.Path (ln, 'd'), RLst [p]);
      Fn2Name (fn);                    // strip ext
      if ( d.FLst (fn, fnl, BITS (fnl), "*.png") ||
           d.FLst (fn, fnl, BITS (fnl), "*.bmp") )  tot++;      // gotta zip
   }
   for (i = p = 0; p < Len; p++) {
      if (GotClose ())  {in.Wipe ();   Quilt (& in);
                         PostDadW (MSG_CLOSE, 0, 0);   return 0;}

   // fn needs AppPath d prefix;  wn needs leading dir stripped
      StrFmt (fn, "Practice file `d of `d: `s", ++i, tot, RLst [p]);
_msg->Set (fn);
DBG       (fn);
      StrFmt (fn, "`s\\Practice\\`s", App.Path (ln, 'd'), RLst [p]);
      StrCp (wn, RLst [p]);
      if (ch = StrCh (wn, '\\'))  StrCp (wn, ch+1);
      StrAp (Buf, wn);   StrAp (Buf, "\n");
      rc = in.Put (fn, wn);

   // if got .png/bmps, zip dir, put it and del it
      Fn2Name (fn);   Fn2Name (wn);    // strip ext
      if (n = d.FLst (fn, fnl, BITS (fnl), "*.png")) {
         StrFmt (ln, "Practice file `d of `d: `s.zip", ++i, tot, wn);
_msg->Set (ln);
DBG       (ln);
         if (GotClose ())  {in.Wipe ();   Quilt (& in);
                            PostDadW (MSG_CLOSE,0,0);   return 0;}

         Zip (fn);   StrAp (fn, ".zip");   StrAp (wn, ".zip");
         if (GotClose ())  {in.Wipe ();   Quilt (& in);
                            PostDadW (MSG_CLOSE,0,0);   return 0;}

         rc = in.Put (fn, wn);
         f.Kill (fn);
         StrFmt (fn, "`s  (`d sheets)\n", wn, n);   StrAp (Buf, fn);
      }
      if (! n)                         // same dang thing for .bmp :/
      if (n = d.FLst (fn, fnl, BITS (fnl), "*.bmp")) {
         StrFmt (ln, "Practice file `d of `d: `s.zip", ++i, tot, wn);
_msg->Set (ln);
DBG       (ln);
         if (GotClose ())  {in.Wipe ();   Quilt (& in);
                            PostDadW (MSG_CLOSE,0,0);   return 0;}

         Zip (fn);   StrAp (fn, ".zip");   StrAp (wn, ".zip");
         if (GotClose ())  {in.Wipe ();   Quilt (& in);
                            PostDadW (MSG_CLOSE,0,0);   return 0;}

         rc = in.Put (fn, wn);
         f.Kill (fn);
         StrFmt (fn, "`s  (`d sheets)\n", wn, n);   StrAp (Buf, fn);
      }
   }

_msg->Set ("putting list file");
DBG       ("putting list file");
   StrFmt (fn, "`s\\list.txt", App.Path (ln, 'e'));
   ReplCh (Buf, '/', '\\');
   f.Save (fn, Buf, StrLn (Buf));
   in.Put (fn, "list.txt");

_msg->Set ("re-quilting " HOM);
DBG       ("re-quilting " HOM);
   Quilt (& in);

_msg->Set ("DONE !!");
DBG       ("DONE !!");
   StrFmt (wn, "http://PianoCheetah.com/rep/`s.html", name);
   RUN ("open", wn);

   PostDadW (MSG_CLOSE, 0, 0);         // ok, you can close too, Pop
TRC("} Doer::Go");
   return 0;
}


class SongUL: public DialogTop {
public:
   SongUL (): DialogTop (IDD_UP, IDI_APP), _doer (NULL)  {}
  ~SongUL ()                               {delete _doer;}
   void Open ()  {_msg.Init (App.wndo, IDC_MSG);   _doer = new Doer (& _msg);}
   bool Shut ()  {_msg.Set ("Ok, CLOSING...   But gotta clean up a bit...");
                  _doer->PostKid (WM_CLOSE, 0, 0);   return false;}
private:
   Doer   *_doer;
   CtlEdit _msg;
};


//------------------------------------------------------------------------------
class DlgUp: public Dialog {
public:
   DlgUp (): Dialog (IDD_APP, IDI_APP)  {}

   void Open ()
   {  _name.Init (Wndo(), IDC_NAME);
      _pass.Init (Wndo(), IDC_PASS);
      _info.Init (Wndo(), IDC_INFO);
      App.CfgGet ("username", Buf);   _name.Set (Buf);   Chomp (Buf, 'e');
      if (*Buf == '\0')               _name.Set ("your pianoworld.com name?");
      App.CfgGet ("userpass", Buf);   _pass.Set (Buf);
      App.CfgGet ("userinfo", Buf);   _info.Set (Buf);   Chomp (Buf, 'e');
      if (*Buf == '\0')               _info.Set (
         "email <x> over at <x.com>\r\n"
         "<name> on pianoworld.com\r\n"
         "<name> on facebook.com\r\n"
         "I love my <piano model>\r\n"
         "etc, etc, etc"
      );
   }

   bool Shut ()
   { TStr name, pass;
      _name.Get (name, sizeof (name));
      _pass.Get (pass, sizeof (pass));
      if (*name == '\0')
         {Hey ("Name can't be empty");                           return false;}
      if (StrCh (name, ' '))
         {Hey ("Name can't have spaces - use . _ or - please :)");
                                                                 return false;}
      if (StrLn (name) > 25)
         {Hey ("Name can't be longer than 25 chars");            return false;}
      if (StrCh (name, '\\') || StrCh (name, '/') || StrCh (name, '?') ||
          StrCh (name, '>')  || StrCh (name, '<') || StrCh (name, '&') ||
          StrCh (name, '*')  || StrCh (name, '!') || StrCh (name, '#') )
         {Hey ("Name can't have \\ / < > ? & * ! # chars :(");   return false;}
      if (*name == '_')
         {Hey ("Names can't start with _");                      return false;}

   // get rep.pc.com/list.txt and see if username's in it
     ulong p;
     bool  newn, got;
     TStr  ln, wn;
     INet  in;
      App.CfgGet ("username", ln);
      Chomp (ln);
      newn = StrCm (ln, name) ? true : false;    // new username ?

      BLn = in.Get (HOM "/list.txt", Buf, sizeof (Buf));
      Buf [BLn] = '\0';
      for (got = false, p = 0;  (! got) && (p < BLn);)
         {p = NextLn (ln, Buf, BLn, p);   if (! StrCm (ln, name))  got = true;}

   // if got name, check pass
      if (got) {
         StrFmt (wn, HOM "/__pass/`s", name);
         ln [in.Get  (wn, ln, sizeof (ln))] = '\0';
         if (StrCm (ln, pass)) {
            if (newn)  Hey ("that name's taken, please try another");
            else       Hey ("Wrong password:(   "
                            "email stephen.hazel@gmail.com if you forgot it");
            return false;
         }
      }
      return true;
   }

   void Done ()
   {  _name.Get (Buf, sizeof (Buf));   App.CfgPut ("username", Buf);
      _pass.Get (Buf, sizeof (Buf));   App.CfgPut ("userpass", Buf);
      _info.Get (Buf, sizeof (Buf));
      if ((! StrLn (Buf)) || (Buf [StrLn (Buf)-1] != '\n')) StrAp (Buf, "\r\n");
                                       App.CfgPut ("userinfo", Buf);
   }
private:
   CtlEdit _name, _pass, _info;
};


//------------------------------------------------------------------------------
bool SExtOK (char *ifn)                // make sure file extension is ok...
{  if ( ((StrLn (ifn) < 8) || StrCm (& ifn [StrLn (ifn)-8], "song.txt")) &&
        ((StrLn (ifn) < 4) || StrCm (& ifn [StrLn (ifn)-4], ".mid"    )) &&
        ((StrLn (ifn) < 4) || StrCm (& ifn [StrLn (ifn)-4], ".kar"    )) &&
        ((StrLn (ifn) < 4) || StrCm (& ifn [StrLn (ifn)-4], ".rmi"    )) )
      return false;
   return true;
}


void FProc (char *fn, RepLstDef *r)
{ File  f;
  char *p, *q, *s;
  ulong ym, dd, d, min = 0, max = 0, i;
  TStr  st;
   StrCp (r->fn, & fn [StrLn (Top)+1]);
   r->src [0] = '\0';
   r->bgnD = r->endD = r->maxD = 0;
   r->nPr  = r->nNt  = 0;

// load fn and get Lst info
//DBG(fn);
   if ((MidLen = f.Load (fn, Mid, sizeof (Mid))) == 0)
      Die ("FProc  couldn't read file", fn);

// must not be a pcheater file...:/
   if (! (p = MemSt (Mid, "ditty_top", MidLen, 'x')))  return;
   Mid [sizeof (Mid)-1] = '\0';        // make sure the dang string ends
   p += 9;

// got source?
   for (q = p;  q = StrSt (q, "source=");  q += 7) {
      MemCp (st, q+7, sizeof (st)-1);   st [sizeof (st)-1] = '\0';
      Chomp (st);
      StrCp (r->src, st);
//DBG("src=`s", st);
      if (*st == '\0')  break;         // append to SLst if filled in n new
      for (i = 0; i < SLen; i++)  if (! StrCm (st, SLst [i]))  break;
      if ((i >= SLen) && (SLen < BITS (SLst)))  StrCp (SLst [SLen++], st);
      break;
   }

// do pract lines
   for (q = p;  q = StrSt (q, "pract ");  q += 6) {
      MemCp (st, q+6, sizeof (st)-1);   st [sizeof (st)-1] = '\0';
      Chomp (st);
      s = st;
      ym = dd = 0;
      if (StrLn (s) >= 6) {            // yyyymm{ dd}
         ym = Str2Int (s);   s += 6;
         while (StrLn (s) >= 3) {
            d = Str2Int (s);  s += 3;
            if (d <= 31)  dd |= (1 << d);
            if ((! min) || (ym*100+d < min))  min = ym*100+d;
            if ((! max) || (ym*100+d > max))  max = ym*100+d;
         }
         r->pr [r->nPr].ym = ym;
         r->pr [r->nPr].d  = dd;
//DBG("pract `d `d `08x", r->nPr, ym, dd);
         r->nPr++;   if (r->nPr >= MAX_PRAC)  break;
      }
   }

// do notes lines
   for (q = p;  q = StrSt (q, "notes ");  q += 6) {
      MemCp (st, q+6, sizeof (st)-1);   st [sizeof (st)-1] = '\0';
      Chomp (st);
      s = st;
      if (StrLn (s) >= 9) {            // yyyymmdd asdfasdf
         d = Str2Int (s);   s += 9;
         if (d) {
            if (! MemCm (s, "done", 4))  r->endD = d;
            else {
                      r->nt [r->nNt].d = d;
               StrCp (r->nt [r->nNt].s,  s);
//DBG("notes `d `d `s", r->nNt, d, s);
               r->nNt++;   if (r->nNt >= MAX_NOTE)  break;
            }
         }
      }
   }
   r->bgnD = min;
   r->maxD = max;
//DBG("min=`d max=`d", min, max);
}


void DoDir (void *ptr, char dfx, char *fn)
// find any song files and put em in Lst
{ char *p;
// need files, not in subdir beLOW learn/rep/done, ok file exten
   if ( (dfx == 'f')  &&  (p = StrCh (& fn [StrLn (Top)], '\\'))  &&
        (! StrCh (p+1, '\\'))  &&  SExtOK (fn) )
      FProc (fn, & Lst [Len++]);
}


//------------------------------------------------------------------------------
void DoRecDir (void *ptr, char dfx, char *fn)
// fill MLst[] w/ any Recorded\*.mp3 files    and RLst[] w .mids
{ char *fnp = (char *)ptr;
   if (dfx != 'f')  return;
   if ((StrLn (fn) >= 4) && (! StrCm (& fn [StrLn (fn)-4], ".mid")))
      {StrCp (RLst [RLen++], & fn [StrLn(Top)+1]);   return;}
   if ((StrLn (fn) >= 4) && (! StrCm (& fn [StrLn (fn)-4], ".mp3")))
      {StrCp (MLst [MLen++], & fn [StrLn(Top)+1]);   return;}
}


//------------------------------------------------------------------------------
char *dt (char *s, ulong ymd)
{  StrCp (s, "          ");
   if (ymd)  StrFmt (s, "`04d/`02d/`02d",
                      ymd/10000, ymd / 100 % 100, ymd % 100);
   return s;
}


int CmpRep (void *p1, void *p2)
// sort by learn/rep/done, then bgnDay desc, then full fn
{ RepLstDef *r1 = (RepLstDef *)p1, *r2 = (RepLstDef *)p2;
  TStr  s1, s2;
  char *p;
  int   t;
   StrCp (s1, r1->fn);   if (p = StrCh (s1, '\\'))  *p = '\0';
   StrCp (s2, r2->fn);   if (p = StrCh (s2, '\\'))  *p = '\0';
   if (t = StrCm (s1,        s2)     )  return t;
   if (t =        r2->bgnD - r1->bgnD)  return t;
   return  StrCm (r1->fn,    r2->fn);
}


ubyte MOLen [12] = {31,29,31,30,31,30,31,31,30,31,30,31};
ubyte MoLen (ulong ym)
{ ubyte p = (ubyte)(ym % 100);
   if ((! p) || (p > 12))  return 0;
   return MOLen [--p];
}


int Go ()
{ File  f;
  TStr  xGrp, grp, fno, fn, s1, s2;
  char *p, ln [1024];
  ulong i, j, nd, pd, s;
  ubyte    k;
  char *ind = "                       ";
TRC("{ RepRpt  Go");

// find all them Practice\* songs' stats n sort em
   Len = 0;
   App.Path (Top, 'd');  StrAp (Top, "\\2_learning");
                                                    f.DoDir (Top, NULL, DoDir);
   App.Path (Top, 'd');  StrAp (Top, "\\3_repertoire");
                                                    f.DoDir (Top, NULL, DoDir);
   App.Path (Top, 'd');  StrAp (Top, "\\4_done");   f.DoDir (Top, NULL, DoDir);
   Sort ( Lst,  Len, sizeof ( Lst [0]), CmpRep);

// sort source list and save to etc/source.txt
   Sort (SLst, SLen, sizeof (SLst [0]), CmpAsc);
   App.Path (fno, 'e');   StrAp (fno, "\\source.txt");
   if (! f.Open (fno, "w"))  Die ("Go  couldn't write etc/source.txt");
   for (i = 0; i < SLen; i++)  {f.Put (SLst [i]);   f.Put ("\r\n");}
   f.Shut ();

   if (*App.parm) {                    // this is all we do fer now :/
TRC("} RepRpt  Go - just doin source.txt");
      return 0;    // just refreshing source.txt?  DONE!
   }

// list Recorded\*.mp3;.mid
   MLen = RLen = 0;   App.Path (Top, 'd');   StrAp (Top, "\\Recorded");
   f.DoDir (Top, NULL, DoRecDir);
   Sort (MLst, MLen, sizeof (MLst [0]), CmpAsc);      // recorded MP3s
   Sort (RLst, RLen, sizeof (RLst [0]), CmpAsc);      // recorded MIDs

// open rpt file
   App.Path (fno, 'd');   StrAp (fno, "\\index.html");
   if (! f.Open (fno, "w"))  Die ("Go  couldn't write index.html");
   f.Put (
"<html><head>\n"
" <title>repertoire</title>\n"
" <link rel=\"stylesheet\" type=\"text/css\" href=\"Device/etc/site.css\" />\n"
" <script type=\"text/javascript\" src=\"Device/etc/site.js\"></script>\n"
"</head>\n"
"<body><pre class=\"fix\">\n"
   );

// first, the list of mp3s...
   if (MLen)  f.Put (
"<a rel=\"exps1\">Recorded MP3s</a><div id=\"s1\" class=\"sho\">\n"
              );
   for (i = 0; i < MLen; i++) {
      StrAp (MLst [i], "", 4);         // strip .mp3 ext
      for (s = 0xFFFFFFFF, j = 0;  j < Len;  j++) {        // get song# ?
         StrCp (fn, Lst [j].fn);
         if (p = StrCh (fn, '\\'))  {*p = '\0';   StrCp (fn, p+1);}
      // swab off the file exten
         if ((StrLn (fn) > 8) && (! StrCm (& fn [StrLn (fn)-8], "song.txt")))
              StrAp (fn, "", 8);
         else StrAp (fn, "", 4);       // trim .mid
         if (! MemCm (MLst [i], fn, StrLn (fn)))  {s = j;   break;}
      }
      StrFmt (fn, "Recorded/`s.mp3", MLst [i]);
      StrFmt (ln,
"<object type=\"application/x-shockwave-flash\"\n"
" data=\"Device/etc/player_mp3.swf\"\n"
" width=\"200\" height=\"20\">\n"
" <param name=\"movie\"     value=\"Device/etc/player_mp3.swf\" />\n"
" <param name=\"FlashVars\" value=\"mp3=`s&amp;bgcolor=FFFFFF\" />\n"
"</object> "
"<a href=\"`s\">`s</a>",
         fn, fn, MLst [i]);
      f.Put (ln);
      if (s != 0xFFFFFFFF) {
         StrFmt (ln,
"  <a href=\"#P`d\">#`d</a>",
            s, Len-s);
         f.Put (ln);
      }
      f.Put (
"\n"
      );
   }
   if (MLen)  f.Put (
"</div>\n"
              );
   f.Put ("\n");


// next, the list of mids...
   if (RLen)  f.Put (
"<a rel=\"exph2\">Recorded MIDs</a><div id=\"h2\" class=\"hid\">\n"
              );
   for (i = 0; i < RLen; i++) {
      StrAp (RLst [i], "", 4);         // strip .mid ext
      for (s = 0xFFFFFFFF, j = 0;  j < Len;  j++) {        // get song# ?
         StrCp (fn, Lst [j].fn);
         if (p = StrCh (fn, '\\'))  {*p = '\0';   StrCp (fn, p+1);}
      // swab off the file exten
         if ((StrLn (fn) > 8) && (! StrCm (& fn [StrLn (fn)-8], "song.txt")))
              StrAp (fn, "", 8);
         else StrAp (fn, "", 4);       // trim .mid
         if (! MemCm (RLst [i], fn, StrLn (fn)))  {s = j;   break;}
      }
      StrFmt (fn, "Recorded/`s.mid", RLst [i]);
      StrFmt (ln,
"<a href=\"`s\">`s</a>",
         fn, RLst [i]);
      f.Put (ln);
      if (s != 0xFFFFFFFF) {
         StrFmt (ln,
"  <a href=\"#P`d\">#`d</a>",
            s, Len-s);
         f.Put (ln);
      }
      f.Put (
"\n"
      );
   }
   if (RLen)  f.Put (
"</div>\n"
              );
   f.Put ("\n");


// prep groupin' n dump it
   f.Put (
"<a rel=\"exps3\">Practice</a><div id=\"s3\" class=\"sho\">\n"
   );
   *xGrp = '\0';
   for (i = 0; i < Len; i++) {
      StrCp (grp, Lst [i].fn);   *fn = '\0';
      if (p = StrCh (grp, '\\'))  {*p = '\0';   StrCp (fn, p+1);}
      if (StrCm (grp, xGrp)) {
         StrCp (xGrp, grp);
         f.Put ((*xGrp == 'z') ? (xGrp+1) : xGrp);
         f.Put (" ----------------------------------------\n");
      }

   // swab off the file exten
      if ((StrLn (fn) > 8) && (! StrCm (& fn [StrLn (fn)-8], "song.txt")))
           StrAp (fn, "", 8);
      else StrAp (fn, "", 4);

   // song# fn and P# anchorthingy
      StrFmt (ln, " `>3d)<a id=\"P`d\"></a> ", Len-i, i);   f.Put (ln);

   // dates n pract
      nd = pd = 0;
      if (Lst [i].bgnD) {
      // pract
         for (nd = pd = j = 0;  j < Lst [i].nPr;  j++) {
            StrFmt (ln, "`s`04d/`02d", ind, Lst [i].pr [j].ym / 100,
                                             Lst [i].pr [j].ym % 100);
            for (k = 1;  k <= MoLen (Lst [i].pr [j].ym);  k++)
               if (Lst [i].pr [j].d & (1 << k)) {
                  nd++;
                  if (Lst [i].endD &&
                     ((Lst [i].pr [j].ym * 100 + k) <= Lst [i].endD))  pd++;
                  StrFmt (& ln [StrLn (ln)], " `02d", k);
               }
            StrAp (ln, "\n");
   //naw    f.Put (ln);
         }
      }
      StrFmt (ln, "`s-`s `s",
         dt (s1, Lst [i].bgnD), dt (s2, Lst [i].endD) + 5, fn);
         //dt (s3, Lst [i].maxD),
      f.Put (ln);
      if (nd > 2) {
         StrFmt (ln, "  pract=`d", nd);
         if (pd) StrFmt (& ln [StrLn (ln)], "  learn=`d", pd);
         f.Put (ln);
      }
      if (Lst [i].src [0]) {
         for (j = 0; j < SLen; j++)  if (! StrCm (SLst [j], Lst [i].src)) break;
         if (j < SLen)
            {StrFmt (ln, "  <a href=\"#S`d\">@`d</a>", j, j+1);
             f.Put (ln);}
      }
      f.Put ("\n");

   // any notes
      for (j = 0;  j < Lst [i].nNt;  j++) {
         StrFmt (ln, "`s`s `s\n",
                  ind, dt (s1, Lst [i].nt [j].d), Lst [i].nt [j].s);
         f.Put (ln);
      }
      if (Lst [i].nNt)  f.Put ("\n");  // gimme a break...
   }
   f.Put (
"</div>\n\n"
   );


   f.Put (
"<a rel=\"exps4\">Sources</a><div id=\"s4\" class=\"sho\">\n"
   );
   for (i = 0; i < SLen; i++) {
      StrCp (s1, SLst [i]);   *s2 = '\0';
      if (p = StrSt (s1, "http:")) {
           StrCp (s2, p);   *p = '\0';
           StrFmt (ln,
" `>3d) `s<a rel=\"x\" href=\"`s\">`s</a><a id=\"S`d\"></a>\n",
              i+1, s1, s2, s2, i);

      }
      else StrFmt (ln,
" `>3d) `s<a id=\"S`d\"></a>\n",
              i+1, SLst [i], i);
      f.Put (ln);
   }
   f.Put (
"</div>\n"
"</pre></body></html>\n"
   );
   f.Shut ();

  DlgUp dlg;
   if (dlg.Ok (NULL))  {SongUL up;   up.Init ();   return App.EvPump ();}
                              RUN ("open", fno);   return 0;
TRC("} RepRpt  Go");
}

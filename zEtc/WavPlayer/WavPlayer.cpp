// WavPlayer.cpp

#include <ui.h>
#include <uiKey.h>
#include <MidiIO.h>
#include <Wav.h>
#include "rc\resource.h"

key    Ky [] = {
   {'1'},{'2'},{'3'},{'4'},{'5'},{'6'},{'7'},{'8'},{'9'},{'0'},
   {'a'},{'b'},{'c'},{'d'},{'e'},{'f'},{'g'},{'h'},{'i'},{'j'},
   {'k'},{'l'},{'m'},{'n'},{'o'},{'p'},{'q'},{'r'},{'s'},{'t'},
   {'u'},{'v'},{'w'},{'x'},{'y'},{'z'}
};
uword NKy = BITS (Ky);

TStr  Info [BITS (Ky)];

const uword PAGES = 200;
const uword WAVS = PAGES * BITS (Ky);

// WAV fns - extra slots to try to pull in whole dir for sorting
uword NFn;    TStr Fn [WAVS + 400];    // WAV fns
uword NMFn;   TStr MFn [100];          // MOD fns
uword NSFn;   TStr SFn [100];          // SF2 fns
bool  SFZ;                             // SFZ flag

class WavPlayer: public WindowTop {
public:
   WavPlayer (): WindowTop ("WavPlayer", IDI_APP)  {}
   void Open ();
   bool Msg  (LRESULT *r, HWND wnd, UINT msg, WPARAM w, LPARAM l);
   void Draw (HDC dc, PAINTSTRUCT *ps);
   TStr    _path;                      // top dir to search in
   uword   _page;                      // which page of 36 wavs (0..z)
   ubyte   _lastk;                     // most recently played wav
   GDIFont _font;
private:
   static void DoDir (void *ptr, char dfx, char *fn);
   void Edit (void), PgInfo (void), Find (void);
};


//______________________________________________________________________________
void WavPlayer::Edit ()                // kick Waver
{ uword p = _page*NKy + _lastk;
  TStr  s;
   if (! Fn [p][0])  return Hey ("No wav picked");
   App.Path (s);   StrAp (s, "\\Waver.exe");   BOOT (s, Fn [p]);
}


void WavPlayer::PgInfo ()
{ ubyte k;
  Wav   wv;
   MemSet (Info, 0, sizeof (Info));
   for (k = 0;  k < NKy;  k++)  if (Fn [_page*NKy+k][0])
      {StrCp (Info [k],    wv.Load (Fn [_page*NKy+k]));   wv.Wipe ();}
}


void WavPlayer::DoDir (void *ptr, char dfx, char *fn)
// put any .wav files in Fn[NFn];  any .sf2 into SFn[NSFn];  any .mod into MFn
{ ulong ln;
//DBG("WavPlayer::DoDir dfx=`c fn=`s", dfx, fn);
   if (((ln = StrLn (fn)) > 4) && (dfx == 'f')) {
      if      (! StrCm (& fn [ln-4], ".sfz"))  SFZ = true;
      else if (! StrCm (& fn [ln-4], ".sf2"))
         {if (NSFn < BITS (SFn))  StrCp (SFn [NSFn++], fn);}
/*    else if (! StrCm (& fn [ln-4], ".mod"))
         {if (NMFn < BITS (MFn))  StrCp (MFn [NMFn++], fn);}
*/
      else if (! StrCm (& fn [ln-4], ".wav"))
         {if (NFn  < BITS  (Fn))  StrCp ( Fn [NFn++],  fn);}
   }
}


int Cmp (void *p1, void *p2)
{ char *s1 = (char *)p1, *s2 = (char *)p2;   return StrCm (s1, s2);  }


void WavPlayer::Find ()
// set title;  convert any SFZ SF2 MOD to wav;  then load wavs
{ uword i;
  TStr  ts, cmd;
  File  f;
// title
   StrCp (ts, _path);   StrAp (ts, " - WavPlayer");   App.SetTitle (ts);

// convert SFZ SF2 MOD
   NFn = NMFn = NSFn = 0;   SFZ = false;
   f.DoDir (_path, this, (FDoDirFunc)(& WavPlayer::DoDir));
   Sort (SFn, NSFn, sizeof (SFn[0]), & Cmp);

   if (SFZ)
      RunWait (StrFmt (cmd, "`s\\UnSFZ `s", App.Path (ts), _path), false);

   if (NSFn)  for (i = 0;  i < NSFn;  i++)
      RunWait (StrFmt (cmd, "`s\\UnSF2 `s", App.Path (ts), SFn [i]), false);

/* no mod till i fix .song format sigh
   if (NMFn)  for (i = 0;  i < NMFn;  i++)
      RunWait (StrFmt (cmd, "`s\\UnMOD `s", App.Path (ts), MFn [i]), false);
*/

// ok now list dem wavs
   NSFn = NMFn = 0;                    // reset so no full errors
   _page = _lastk = 0;
   NFn = 0;   for (i = 0;  i < BITS (Fn);  i++)  Fn [i][0] = '\0';
   f.DoDir (_path, this, (FDoDirFunc)(& WavPlayer::DoDir));
//DBG("NFn=`d", NFn);
   Sort (Fn, NFn, sizeof (Fn [0]), & Cmp);
   PgInfo ();
}


//______________________________________________________________________________
#define HDR1  " frequency bit chan    samples  loopBegin    loopEnd note frac"
#define HDR2  "   Esc=Quit  F1=Refresh  F2=Edit  PgUp  PgDn  Play=0..9,a..z"

bool WavPlayer::Msg (LRESULT *r, HWND wnd, UINT msg, WPARAM w, LPARAM l)
{ KeyMap km;
  key    k;
  ubyte  i;
  TSt2   f2;
   if ((msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN) ||
       (msg == WM_KEYUP  ) || (msg == WM_SYSKEYUP  ))
   // if keydown-nonrepeat
      if (! (l & 0x40000000))  if (k = km.Map (w)) {
         switch (k) {
         case ESC_KEY:  App.Close ();   break;
         case F01_KEY:  Find ();        break;
         case F02_KEY:  Edit ();        break;
         case PDN_KEY:  if (++_page >= PAGES)  _page = PAGES-1;   PgInfo ();
                                        break;
         case PUP_KEY:  if (_page)  --_page;                      PgInfo ();
                                        break;
         default:
            for (i = 0;  i < NKy;  i++)  if ((k == Ky [i]) &&
                                             Fn [_page*NKy + i][0])
               {::PlaySound (StrCvt (f2, Fn [_page*NKy + (_lastk = i)]),
                             NULL, SND_FILENAME|SND_ASYNC|SND_NODEFAULT);
                break;}
         }
         App.Updt ();   return true;
      }
   return false;
}


void WavPlayer::Draw (HDC dc, PAINTSTRUCT *ps)
{ Canvas  c (App.wndo);
  HGDIOBJ oldf;
  uword   k;
  PStr    buf;
   oldf = c.Set (_font.Font ());
// crappy menu :/
   c.Text (0, 0*_font.HCh (), HDR1 HDR2);
   for (k = 0;  k < NKy;  k++)  if (Fn [_page*NKy+k][0]) {
      StrFmt (buf, "`<62s  `s`c `s",
                   Info [k],  (_lastk == k) ? ">" : " ",  (char)Ky [k],
                   & Fn [_page*NKy+k][StrLn (_path)+1]);
      c.Text (0, (1+k)*_font.HCh (), buf);
   }
   c.Set (oldf);
}


void WavPlayer::Open ()
{ Canvas c (App.wndo);
// make a Consolas 12 font and get it's char w,h
   _font.Init (& c, "Consolas", 12);
   Move (20, 1, _font.WCh () * StrLn (HDR1 HDR2 "   "),
                _font.HCh () * (3+NKy));
// init KyLst by searchin for .wav files, etc
   StrCp (_path, & App.parm [4]);      // skip "DIR "
   Find ();
}


int Go ()
{  if (MemCm (App.parm, "DIR ", 4))
      Die ("WavPlayer: use win explorer right click menu");
   if (! App.parm [4])
      Die ("WavPlayer: I need a dir of .WAVs to play...");
  WavPlayer wnd;
   wnd.Init ();
   return App.EvPump ();
}

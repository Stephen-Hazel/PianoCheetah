// upd.cpp - time ta update yet per website?

#include "pcheetah.h"

#define EVERY   (14)                   // only check for updates every N days
#define URLDIR  "http://PianoCheetah.app"
#define SETUP    "SETUP_PIANOCHEETAH.exe"   // installer


class DlgUpd: public Dialog {
public:
   DlgUpd (char *c, char *s, char *x)
   : Dialog (IDD_UPD, IDI_APP), _c (c), _s (s), _x (x)  {}
private:
   char *_c, *_s, *_x;
   Control _cs, _ss;

   void Open ()
   { TStr ts;
      *_x = ' ';
      _cs.Init (Wndo (), IDC_CL);
      _ss.Init (Wndo (), IDC_SV);
      StrCp (ts, "  /  /    ");
      MemCp (& ts [0], & _c [4], 2);   MemCp (& ts [3], & _c [6], 2);
                                       MemCp (& ts [6], & _c [0], 4);
      _cs.Set (ts);
      MemCp (& ts [0], & _s [4], 2);   MemCp (& ts [3], & _s [6], 2);
                                       MemCp (& ts [6], & _s [0], 4);
      _ss.Set (ts);
   }

   bool Do (int ctrl, int evnt, LPARAM l)
   {  if (evnt == BN_CLICKED) {
         switch (ctrl) {
            case IDC_RNT:  RUN ("open", URLDIR "/news");   return false;
            case IDC_UPD:  *_x = 'y';   break;
            case IDC_NAW:  *_x = ' ';   break;
            case IDC_OFF:  *_x = 'n';   break;
         }
         Post (WM_CLOSE, 0, 0);
         return true;
      }
      return false;
   }
};


bool Update ()
{ TStr  sVer, cVer, fn, fn2, x;
  ubyt4 ln;
  ubyt2 i;
  File  f;
  ubyte *buf;
  StrArr t ("cfg", 80, 80*sizeof(TStr));
   App.Path (fn, 'c');   StrAp (fn, "/cfg.txt");   t.Load (fn);
   for (i = 0;  i < t.NRow ();  i++)  if (! StrCm (t.str [i], "updt=y"))  break;
   if (i >= t.NRow ())
{DBG("Update  cfg says nope");         return false;}

// only hit the web every "EVERY" days
   App.Path (fn, 'c');   StrAp (fn, "/upd.cfg");
   if (f.Size (fn) == 0)  App.CfgPut ("upd", "x");    // make upd.cfg if none
   if (f.HrsOld (fn) < EVERY*24)
{DBG("Update  not 2 weeks yet");       return false;}

// get client version from local app\version.txt file
   App.Path (fn);   StrAp (fn, "/version.txt");
   *sVer = *cVer = '\0';
   ln = f.Load (fn, cVer, sizeof (cVer));   cVer [8] = '\0';
   App.CfgPut ("upd", "x");                           // reset writetime to NOW

// mark weblog with my (current) client version
  INet in;
   StrFmt (fn, "`s/version_`s", URLDIR, cVer);
   in.Get (fn, x, sizeof (x));

// get server version
   StrFmt (fn, "`s/version.txt", URLDIR);
   ln = in.Get (fn, sVer, sizeof (sVer));   sVer [8] = '\0';
   if (ln == 0)
{DBG("Update  no inet?");             return false;}

// if current version < server's, ask about updating
   if (StrCm (cVer, sVer) >= 0)
{DBG("Update  already up to date");   return false;}

DBG("{ DlgUpd");
  DlgUpd dlg (cVer, sVer, x);
   dlg.Ok (NULL);
DBG("} DlgUpd");
   if (*x == 'n') {                    // turn it off
DBG("Update - turning update off in cfg");
      App.Path (fn, 'c');   StrAp (fn, "/cfg.txt");
      if (f.Open (fn, "w")) {
         for (ubyt2 i = 0;  i < t.NRow ();  i++) {
            if (! StrCm (t.str [i], "updt=y"))  f.Put ("updt=n");
            else                                f.Put (t.str [i]);
            f.Put ("\n");
         }
         f.Shut ();
      }
   }
   if (*x == 'y') {
DBG("Update - DOIN IT");
      buf = new ubyte                  [8*1024*1024];
      StrFmt (fn, "`s/`s", URLDIR, SETUP);
      ln = in.Get (fn, buf, 8*1024*1024);
      App.WPath (fn2, "temp");   StrAp (fn2, SETUP);   f.Save (fn2, buf, ln);
      delete [] buf;
DBG(fn2);
      BOOT (fn2, "shh");
DBG("Update - did it :)");
      return true;
   }
DBG("Update  done");
   return false;
}

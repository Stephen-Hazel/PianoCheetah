// update.cpp - updater fer pcheetah

#include "update.h"

char  Buf [8000000];
ubyt4 Len;


void Update::Init ()
{ TStr s, fn, dtGot, dtNew;
  File f;
   TRC("Init");
   StrCp (s, CC("n"));                 // default ta don't kill yerse'f
   App.CfgGet (CC("d"), s);
   if (*s == '\0') {                   // uh oh!  need our data dir somewherez
     TStr d;
      Gui.Hey (
         "Oh hi.  Nice ta meetcha :)\n\n"
         "I need you to pick a directory for your midi files.\n"
         "Your home dir is fine.\n\n"
         "I'll make a pianocheetah dir there.");
      StrCp (d, getenv ("HOME"));
      if (Gui.AskDir (d, "Pick a directory to put the pianocheetah dir into")) {
         StrFmt (fn, "`s/pianocheetah.gz", d);
DBG("picked=`s", d);
         if (f.Size (fn)) {
            Gui.Hey (
               "Ummm, there's already a pianocheetah dir there.\n"
               "I don't wanna trash it so...\n"
               "run PianoCheetah again to give it another shot.");
            StrCp (s, CC("y"));
         }
         else {
            Len = WGet (Buf, sizeof (Buf),
               CC("https://pianocheetah.app/download/pianocheetah.gz"));
            f.Save (fn, Buf, Len);
            GUnZip (fn);
            App.CfgPut (CC("d"), fn);
DBG("Ok!  dir=`s", fn);
         }
      }
      else {
         Gui.Hey (
            "Unhh.  No directory no PianoCheetah :/\n"
            "Run PianoCheetah again to give it another shot");
         StrCp (s, CC("y"));
      }
   }

// check for update
   App.CfgGet (CC("update_dt"), dtGot);
   WGet (dtNew, sizeof (dtNew), CC("https://pianocheetah.app/download/dtNew"));
   dtGot [8] = dtNew [8] = '\0';
DBG("got=`s new=`s", dtGot, dtNew);
/*
   if (StrCm (dtGot, dtNew) < 0) {
      if (Gui.YNo (CC("There's a new version of PianoCheetah\n"
                      "Do ya want it?"))) {
      }
      else {
      }
   }
*/
   App.CfgPut (CC("update"), s);
TRC("Init end");
   Gui.Quit ();
}

void Update::Quit ()  {}

int main (int argc, char *argv [])
{ QApplication app (argc, argv);
  Update       win;
   App.Init (CC("pcheetah"), CC("update"), CC("Update"));
   Gui.Init (& app, & win);   win.Init ();
  int rc = Gui.Loop ();       win.Quit ();
   return rc;
}

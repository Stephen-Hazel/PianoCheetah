// initme.cpp - init fer pcheetah

#include "initme.h"

char  Buf [800000000];
ubyt4 Len;


void InitMe::Init ()
{ TStr s, dir, fn;
  File f;
DBG("Init bgn");
   Gui.Hey (
      "Oh hi :)\n\n"
      "I need you to pick a directory for your pianocheetah files.\n"
      "Your home dir is fine.\n\n"
      "I'll make a pianocheetah dir there.");
   StrCp (dir, getenv ("HOME"));
DBG("home=`s", dir);
   if (Gui.AskDir (dir, "Pick a dir to put the pianocheetah dir into")) {
      StrAp (dir, CC("/pianocheetah"));
DBG("picked=`s", dir);
      if (! f.Size (dir)) {
         Gui.Hey ("I need to download a BIG file of samples.\n"
                  "Then I'll start PianoCheetah's midi configuration.\n"
                  "This will take 3 minutes.  Please be patient :)\n"
                  "In the mean time, turn on all your midi devices.");
         Len = WGet (Buf, sizeof (Buf),
            CC("https://pianocheetah.app/download/pianocheetah.tar.gz"));
         StrFmt (fn, "`s.tar.gz", dir);
         f.Save (fn, Buf, Len);
DBG("Len=`d size=`d", Len, f.Size (fn));
         Zip (dir, 'x');
DBG("unzip complete");
      }
      App.CfgPut (CC("d"), dir);
DBG("dir=`s", dir);
   }
DBG("Init end");
   Gui.Quit ();
}

void InitMe::Quit ()  {}

int main (int argc, char *argv [])
{ QApplication app (argc, argv);
  InitMe       win;
DBGTH("InitMe");
   App.Init (CC("pcheetah"), CC("initme"), CC("InitMe"));
   Gui.Init (& app, & win);   win.Init ();
  int rc = Gui.Loop ();
   win.Quit ();
   return rc;
}

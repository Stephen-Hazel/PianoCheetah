// initme.cpp - init fer pcheetah

#include "initme.h"

char  Buf [8000000];
ubyt4 Len;


void InitMe::Init ()
{ TStr s, fn, dtGot, dtNew;
  File f;
TRC("Init");
   StrCp (s, CC("n"));                 // default ta don't kill yerse'f
   App.CfgGet (CC("d"), s);
   if (*s == '\0') {                   // uh oh!  need our data dir somewherez
     TStr d;
      Gui.Hey (
         "Oh hi :)\n\n"
         "I need you to pick a directory for your pianocheetah files.\n"
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
               CC("https://pianocheetah.app/download/pianocheetah.tar.gz"));
            f.Save (fn, Buf, Len);
            Zip (fn, 'x');
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

TRC("Init end");
   Gui.Quit ();
}

void InitMe::Quit ()  {}

int main (int argc, char *argv [])
{ QApplication app (argc, argv);
  InitMe       win;
   App.Init (CC("pcheetah"), CC("initme"), CC("InitMe"));
   Gui.Init (& app, & win);   win.Init ();
  int rc = Gui.Loop ();       win.Quit ();
   return rc;
}

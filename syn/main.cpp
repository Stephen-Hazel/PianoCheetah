// syn's main.cpp - back to basics .WAV based synth for PianoCheetah

#include "../../stv/os.h"
#include "../../stv/aud.h"
#include "../../stv/syn.h"


int main (int argc, char *argv [])
{  App.Init (CC("pcheetah"), CC("syn"), CC("syn"));
TRC("syn bgn");
  sbyt4 *m, w;
  ShMem  shm;
  AudO   auo;
  bool   quit = false;
  MSG    msg;
  ubyte  ch, v, v2;
  ubyt2  c;
  ubyt4  tp = 120;

// special-ish App.UpdTrc()
   { TStr s;
      App.CfgGet (CC("syntrace"), s);   App.trc = (*s == 'y') ? true : false;
   }
// am i already here?  if so, sneak out
   if (shm.Open ("syn.exe", sizeof (sbyt4)))
      {shm.Shut ();   return 99;}

// i ain't, so open up shop
   if (! (m = (sbyt4 *) shm.Init ("syn.exe", sizeof (sbyt4)))) {
DBG("shm.Init died");
      return 99;
   }
   *m = ::GetCurrentThreadId ();
TRC("shm ok");

// init audioOut and synth
   auo.Open ();                        // shouuuld keep tmpo up to date...:/
  Syn syn (& tp, (ubyt2)auo._len, auo._bits, auo._float, auo._frq);
          syn.PutAuO (auo.GetBuf ());   auo.PutBuf ();

TRC("init done - startin loop");
//  DurTimer t;
   while (! quit) {
      w = ::MsgWaitForMultipleObjects (1, & auo._ev, FALSE, INFINITE,
                                       QS_SENDMESSAGE|QS_ALLPOSTMESSAGE);
//DBG("w=`u wo0=`u dur=`u", w, WAIT_OBJECT_0, t.DurNow ());  t.Bgn ();
      if (w == WAIT_OBJECT_0)
         {syn.PutAuO (auo.GetBuf ());   auo.PutBuf ();}

      while (::PeekMessage (& msg, NULL, 0, 0, PM_REMOVE)) {
         switch (msg.message) {
            case MSG_CLOSE+3:          // dump
               if (App.trc)  syn.Dump ();
               break;
            case MSG_CLOSE+2:          // do some midi
               ch =  msg.wParam;   c = msg.lParam >> 16;
               v  = (msg.lParam >> 8) & 0xFF;
               v2 =  msg.lParam       & 0xFF;
               syn.Put (ch, c, v, v2);
               break;
            case MSG_CLOSE+1:          // load song's bank of sounds
TRC(" syn LoadSound bgn");
               auo.Pause (true);   syn.LoadSound ();   auo.Pause (false);
TRC(" syn LoadSound end");
               break;
            case MSG_CLOSE:
TRC(" syn exit");
               auo.Pause (true);   quit = true;
         }
      }
   }

//DBG("");
TRC("syn shuttin down");
   auo.Shut ();   shm.Shut ();
TRC("syn end");

   return 0;
}

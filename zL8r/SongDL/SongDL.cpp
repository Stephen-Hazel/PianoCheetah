// SongDL.cpp - download a songpack from web to 4_queue dir

#include "rc\resource.h"
#include "ui.h"

class Doer: public Thread {
public:
   Doer (CtlEdit *m): Thread (App.wndo), _msg (m)  {}
private:
   DWORD Go ();
   CtlEdit *_msg;
};

char  Buf [80*1024*1024];  // 80 meg max
ulong Len;

DWORD Doer::Go ()
{ MSG  msg;
  TStr nm, fn, ln, s, s2;
  File f;
  INet in;
DBG("{ Doer::Go");
   InitMsgQ ();   msg.message = 0;

// get name from cmdln n quit if any old junk in that dir
   StrCp (nm, App.parm);
   App.Path (fn, 'd');   StrAp (fn, "\\4_queue\\");   StrAp (fn, nm);
   if (f.PathGot (fn))  {PostDadW (MSG_CLOSE, 0, 0);   return 0;}

// do status;  hit weblog to make d/l stats per song pack
   _msg->Set (StrFmt (s, "downloading `s.zip", nm));
   StrCp (ln, "http://PianoCheetah.app/SongDL/");   StrAp (ln, nm);
   in.Get (ln, Buf, sizeof (Buf));     // just to hit weblog

// actually grab it now
   StrFmt (ln, "http://PianoCheetah.app/rep/`s/`s.zip", nm, nm);
   Len = in.Get (ln, Buf, sizeof (Buf));
DBG("len=`d", Len);

   _msg->Set (StrFmt (s, "saving `s.zip", nm));
   App.Path (fn, 'd');   StrAp (fn, "\\4_queue\\");   StrAp (fn, nm);
                                                      StrAp (fn, ".zip");
DBG("save fn=`s", fn);
   f.Save (fn, Buf, Len);

   _msg->Set (StrFmt (s, "unzipping `s", nm));
   StrCp (s2, fn);   Fn2Name (s2);
   Zip (s2, 'u');   f.Kill (fn);

   _msg->Set ("done :)");
   PostDadW (MSG_CLOSE, 0, 0);         // ok, you can close too, Pop
DBG("} Doer::Go");
   return 0;
}


//------------------------------------------------------------------------------
class SongDL: public DialogTop {
public:
   SongDL (): DialogTop (IDD_APP, IDI_APP), _doer (NULL)  {}
  ~SongDL ()                        {delete _doer;}
   void Open ()  {_msg.Init (App.wndo, IDC_MSG);   _doer = new Doer (& _msg);}
   bool Shut ()  {_msg.Set ("Ok, hold on...  closin' down...");
                  _doer->PostKid (WM_CLOSE, 0, 0);   return false;}
private:
   Doer   *_doer;
   CtlEdit _msg;
};


int Go ()
{
TRC("SongDL  Go");
  SongDL dlg;   dlg.Init ();   return App.EvPump ();
}

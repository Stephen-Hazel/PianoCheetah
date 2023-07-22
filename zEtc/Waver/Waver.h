// Waver.h

#include <ui.h>
#include <uiCtlTip.h>
#include <WaveIO.h>
#include <MidiIO.h>
#include "rc\resource.h"

#define EVEN(n)     ((n)&0xFFFFFFFE)
#define EVEN_UP(n)  EVEN((n)+1)
const slong MAXSL = 2147483648;        // max neg slong
                                       // sigh, this actually turns out neg:(
#define GREY(n)     (RGB(192+(n),192+(n),192+(n)))

typedef struct {
   ulong manuf;  ulong prod;  ulong per;  ulong key;  ulong cnt;
   ulong sfmt;   ulong sofs;  ulong num;  ulong dat;  ulong cue;
   ulong loop;   ulong bgn;   ulong end;  ulong frc;  ulong times;
} WAVESMPL;

class Waver;

class CtlWave: public CtlCstm {
public:
   void Init (HWND wPar, int id, Waver *w)
   {  _w = w;   CtlCstm::Init (wPar, id, "WAVESHOW", false, true, false, true);
   }
   virtual void Draw (Canvas *can, bool foc);
   virtual void MsMv (ulong btn, sword x, sword y);

private:
   void Brkt (Canvas *cnv, uword x, uword h, COLORREF c, char dir);
   Waver *_w;
};


typedef struct {
   Toolbar tbar;                     //... controls
   CtlScrl scrl;
   Control bgn, end, lBgn, lEnd, info, pos;
   CtlWave show;
   CtlEdit frq, key, cnt;
   CtlCmbo mag;
   CtlBttn loop, setLBgn, setLEnd;
} CtlDef;


//------------------------------------------------------------------------------
class Wave {
public:
   Wave (CtlDef *c);
  ~Wave ();
   void  Wipe (), Load (char *fn), Save (char *fn = NULL),
         Play (char p), Stop (),
         SetPos (ulong p), SetLp (), SetMrk (char typ), NewMrk (char typ),
         All (), Chop ();
   slong Smp    (ulong pos, ubyte lr);
   TStr    _name;
   MemFile _mf;
   WaveO   _wo;
   void   *_mem;                       // wav sample data - ptr sbyte/sword[1/2]
   bool    _mono, _loop;
   ubyte   _byts;                      // 1,2,3,4
   ubyte   _bits, _key;                // 8,16,24,32;  sampled key
   sbyte          _cnt;                                   // n cent
   ulong   _len,  _frq,                // num samples, frequency,
           _bgn, _end, _lBgn,  _lEnd,  // selection bgn/end loop point bgn/end
                       _lBgn1, _lEnd1, // as loaded
                       _lBgn2, _lEnd2, // as loaded the "wrong" way
           _pos;                       // current editing pos
   uword   _pX, _pH;                   // stored spot to un XOR
   WAVEFORMATEX _fmt;
   WAVESMPL    *_smp;
private:
   char *PosStr (ulong p);
   void  Puke   (char *m1, char *m2 = NULL);
   GDIFont _font;
   CtlDef *_c;
};


//------------------------------------------------------------------------------
class Waver: public DialogTop {
public:
   Waver (): DialogTop (IDD_APP, IDI_APP)  {}
   virtual void Open ();
   virtual bool Shut ();
   virtual bool Do   (int ctrl, int evnt, LPARAM l)
   {  return DoEvMap (this, _evMap, ctrl, evnt, l);  }
   void Size (char *state, uword w, uword h);
   bool Msg  (LRESULT *r, HWND wnd, UINT msg, WPARAM w, LPARAM l);
   void Redo (LPARAM l);

   CtlDef _c;
   Wave  *_wave;
private:
   TStr _midiPath;
   void Load (LPARAM l), Save (LPARAM l), Play (LPARAM l),
        PlayFr (LPARAM l), PlayTo (LPARAM l), Stop (LPARAM l), All (LPARAM l),
        Chop (LPARAM l),
        SetLp (LPARAM l),  FrqNew (LPARAM l),
        SetBgn (LPARAM l), SetEnd (LPARAM l), SetLBgn (LPARAM l),
                                              SetLEnd (LPARAM l),
        NewBgn (LPARAM l), NewEnd (LPARAM l), NewLBgn (LPARAM l),
                                              NewLEnd (LPARAM l);
   static EvMap<Waver> _evMap [];
};

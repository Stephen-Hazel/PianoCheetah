// ctlNt.h

#ifndef CTLNT_H
#define CTLNT_H

#include "glo.h"

class CtlNt: public QWidget {
   Q_OBJECT

public:
   explicit CtlNt (QWidget *parent = nullptr)
   : QWidget (parent)
   {  Up.pm = nullptr;   Up.w = Up.h = 0;
      setAttribute (Qt::WA_NoSystemBackground);  // =i= do bg
//DBG("autoBg=`b Opaque=`b NoBg=`b",
//autoFillBackground(),testAttribute(Qt::WA_OpaquePaintEvent),
//                     testAttribute(Qt::WA_NoSystemBackground));
      _pos = _drg = '\0';
   }

  ~CtlNt ()  {if (Up.pm)  delete Up.pm;}
   
   void Init (ubyt2 w, ubyt2 h)  
   {  Up.w = w;   Up.h = h;   RePM ();   emit sgReSz ();  }

signals:
   void sgReSz ();

protected:                             // our main bg pixmap is in Up.pm
   void RePM ();                       // upd pixmap in Up.tpm,tpos
   void paintEvent        (QPaintEvent  *e);
   void resizeEvent       (QResizeEvent *e);

   void keyPressEvent     (QKeyEvent    *e);
   void wheelEvent        (QWheelEvent  *e);
   void mouseMoveEvent    (QMouseEvent  *e);
   void mousePressEvent   (QMouseEvent  *e);
   void mouseReleaseEvent (QMouseEvent  *e);
// void mouseDoubleClickEvent (QMouseEvent *e);
// setMouseTracking(true) to get mousemoves even while no button held
// underMouse to see if mouse is over me

private:
   char  _pos, _got, _drg;             // CtlNt.cpp docs these
   ubyt4 _pg, _co, _tm, _sy, _p;       // page, column, time, symbol, trk ev pos
   sbyt2 _x1, _y1, _x2, _y2, _xp, _yp, _xo, _yo;
   ubyte _ct, _cp, _tr;                // control, ctl pos, track
   TStr  _str;
   bool  _pPoz;                        // pause prev on
   
   void DragRc ();
   char MsPos                    (sbyt2 x, sbyt2 y);
   void MsDn (Qt::MouseButtons b, sbyt2 x, sbyt2 y);
   void MsMv (Qt::MouseButtons b, sbyt2 x, sbyt2 y);
   void MsUp (Qt::MouseButtons b, sbyt2 x, sbyt2 y);   
};

#endif // CTLNT_H

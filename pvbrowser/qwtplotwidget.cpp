/***************************************************************************
                          qwtplotwidget.cpp  -  description
                             -------------------
    begin                : Mon Mai 26 2003
    copyright            : (C) 2003 by R. Lehrig
                         : Angel Maza
    email                : lehrig@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pvdefine.h"
#include <stdio.h>
#include "qwtplotwidget.h"
#include "qwt_symbol.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include <QMouseEvent>
#include "tcputil.h"
#include "pvserver.h"
#include "opt.h"

extern OPT opt;

QwtPlotWidget::QwtPlotWidget(int *sock, int ident, QWidget *parent, int numberCurves, int numberMarker) : QwtPlot(parent)
{
    int i;
    line = NULL;
    nCurves = numberCurves;
    nMarker = numberMarker;
    marker = new MyMarker[nMarker];
    curves = new MyCurve[nCurves];
    for(i=0; i<nMarker; i++) marker[i] = NULL;
    for(i=0; i<nCurves; i++) curves[i] = NULL;
    legend = NULL;

    QPen pen;
    pen.setStyle(Qt::DashLine);
    grid.setPen(pen);
    grid.attach(this);
    s = sock;
    id = ident;

    autolegend = 0;
    enablelegend = 0;
    legendframestyle = 0;

    //connect( this,  SIGNAL(plotMouseMoved(const QMouseEvent&)), SLOT(slotMouseMoved( const QMouseEvent&)));
    //connect( this,  SIGNAL(plotMousePressed(const QMouseEvent&)), SLOT(slotMousePressed( const QMouseEvent&)));
    //connect( this,  SIGNAL(plotMouseReleased(const QMouseEvent&)), SLOT(slotMouseReleased( const QMouseEvent&)));

/****************************************************************
    setTitle("Frequency Response of a Second-Order System");

    setCanvasBackground(darkBlue);

    // outline
    enableOutline(TRUE);
    setOutlinePen(green);

    // legend
    setAutoLegend(TRUE);
    enableLegend(TRUE);
    setLegendPos(Qwt::Bottom);
    setLegendFrameStyle(QFrame::Box|QFrame::Sunken);

    // grid
    enableGridXMin();
    setGridMajPen(QPen(white, 0, DotLine));
    setGridMinPen(QPen(gray, 0 , DotLine));

    // axes
    enableAxis(QwtPlot::yRight);
    setAxisTitle(QwtPlot::xBottom, "Normalized Frequency");
    setAxisTitle(QwtPlot::yLeft, "Amplitude [dB]");
    setAxisTitle(QwtPlot::yRight, "Phase [deg]");

    setAxisOptions(QwtPlot::xBottom, QwtAutoScale::Logarithmic);
    setAxisMaxMajor(QwtPlot::xBottom, 6);
    setAxisMaxMinor(QwtPlot::xBottom, 10);

    // curves
    curves[0] = insertCurve("Amplitude");
    setCurvePen(curves[0], QPen(yellow));
    setCurveYAxis(curves[0], QwtPlot::yLeft);

    curves[1] = insertCurve( "Phase");
    setCurvePen(curves[1], QPen(cyan));
    setCurveYAxis(curves[1], QwtPlot::yRight);

    // marker
    marker[0] = insertMarker();
    setMarkerLineStyle(marker[0], QwtMarker::VLine);
    setMarkerPos(marker[0], 0.0,0.0);
    setMarkerLabelAlign(marker[0], AlignRight|AlignBottom);
    setMarkerPen(marker[0], QPen(green, 0, DashDotLine));
    setMarkerFont(marker[0], QFont("Helvetica", 10, QFont::Bold));

    marker[1] = insertLineMarker("", QwtPlot::yLeft);
    setMarkerLabelAlign(marker[1], AlignRight|AlignBottom);
    setMaQwtPlot::LegendPositionrkerPen(marker[1], QPen(QColor(200,150,0), 0, DashDotLine));
    setMarkerFont(marker[1], QFont("Helvetica", 10, QFont::Bold));
    setMarkerSymbol(marker[1],
        QwtSymbol(QwtSymbol::Diamond, yellow, green, QSize(7,7)));

    // setDamp(0.0);
**********************************************************************/
}

QwtPlotWidget::~QwtPlotWidget()
{
  int i;
  for(i=0; i<nMarker; i++) if(marker[i] != NULL) delete marker[i];
  for(i=0; i<nCurves; i++) if(curves[i] != NULL) delete curves[i];
  delete [] curves;
  delete [] marker;
}

int QwtPlotWidget::getText(const char *c, QString &qtext)
{
  char text[1024], *t;
  int  i, maxlength;

  qtext = "";
  maxlength = ((int) sizeof(text)) - 1;
  t = &text[0];
  *t = '\0';
  while(*c != '\0' && *c != '"') c++; // search for first "
  if(*c == '\0')
  {
    return -1;
  }
  c++;
  i = 0;
  while(*c != '\0') // until terminating " is seen
  {
    if(i >= maxlength)
    {
      *t = '\0';
      return 1;
    }
    i++;
    if(*c == '\\')
    {
      c++;
      if     (*c == '"')  *t++ = '"';
      else if(*c == '\n') *t++ = '\n';
      else if(*c == '\t') *t++ = '\t';
      else if(*c == '\0') break;
      c++;
    }
    else if(*c == '"')
    {
      *t = '\0';
      break;
    }
    else
    {
      *t++ = *c++;
    }
  }
  *t = '\0';
  qtext = QString::fromUtf8(text);
  return 0;
}

int QwtPlotWidget::isCommand(const char *command)
{
  int i = 0;
  if(line == NULL) return 0;
  while(command[i] != '\0')
  {
    if(command[i] != line[i]) return 0;
    i++;
  }
  return 1;
}

int QwtPlotWidget::interpret(const char *command, double *x, double *y)
{
  if(command == NULL) return -1;
  line = command;

  if     (isCommand("setCurveData("))
  {
    if(x == NULL) return -1;
    if(y == NULL) return -1;
    int c,count;
    sscanf(command,"setCurveData(%d,%d",&c,&count);
    if(c<0 || c>=nCurves) return -1;
    if(curves[c] != NULL) curves[c]->setData(x,y,count);
  }
  else if(isCommand("replot("))
  {
    replot();
  }
  else if(isCommand("setTitle("))
  {
    QString text;
    if(getText(command,text) != 0) return -1;
    setTitle(text);
  }
  else if(isCommand("setCanvasBackground("))
  {
    int r,g,b;
    sscanf(command,"setCanvasBackground(%d,%d,%d",&r,&g,&b);
    setCanvasBackground(QColor(r,g,b));
  }
  else if(isCommand("enableOutline("))
  {
    int val;
    sscanf(command,"enableOutline(%d",&val);
    if( val == 0 || val == 1 )
    {
      //xx enableOutline(val);
      //xx setOutlineStyle(Qwt::Rect );
    }
    else
    {
      //xx enableOutline( 1 );
      //xx setOutlineStyle(Qwt::Cross );
    }
  }
  else if(isCommand("setOutlinePen("))
  {
    int r,g,b;
    sscanf(command,"setOutlinePen(%d,%d,%d",&r,&g,&b);
    //xx setOutlinePen(QColor(r,g,b));
  }
  else if(isCommand("setAutoLegend("))
  {
    int val;
    sscanf(command,"setAutoLegend(%d",&val);
    //xx setAutoLegend(val);
    autolegend = val;
  }
  else if(isCommand("enableLegend("))
  {
    int val;
    sscanf(command,"enableLegend(%d",&val);
    enablelegend = val;
  }
  else if(isCommand("setLegendPos("))
  {
    int val;
    sscanf(command,"setLegendPos(%d",&val);
    if(opt.arg_debug) printf("SetLegendPos begin\n");;
    if(legend == NULL) legend = new QwtLegend();
    if(opt.arg_debug) printf("SetLegendPos 1\n");;
    legend->setItemMode(QwtLegend::ClickableItem);
    if(opt.arg_debug) printf("SetLegendPos 2\n");;
    switch(val)
    {
      case PV::LeftLegend:
        insertLegend(legend, QwtPlot::LeftLegend);
        break;
      case PV::RightLegend:
        insertLegend(legend, QwtPlot::RightLegend);
        break;
      case PV::TopLegend:
        insertLegend(legend, QwtPlot::TopLegend);
        break;
      default:
        insertLegend(legend, QwtPlot::BottomLegend);
        break;
    }
    if(opt.arg_debug) printf("SetLegendPos end\n");;
  }
  else if(isCommand("setLegendFrameStyle("))
  {
    int val;
    sscanf(command,"setLegendFrameStyle(%d",&val);
    legendframestyle = val;
    if(legend != NULL) legend->setFrameStyle(legendframestyle);
  }
  else if(isCommand("enableGridXMin("))
  {
    grid.enableXMin(true);
  }
  else if(isCommand("setGridMajPen("))
  {
    int r,g,b,style;
    sscanf(command,"setGridMajPen(%d,%d,%d,%d",&r,&g,&b,&style);
    grid.setMajPen(QPen(QColor(r,g,b),0,(Qt::PenStyle) style));
  }
  else if(isCommand("setGridMinPen("))
  {
    int r,g,b,style;
    sscanf(command,"setGridMinPen(%d,%d,%d,%d",&r,&g,&b,&style);
    grid.setMinPen(QPen(QColor(r,g,b),0,(Qt::PenStyle) style));
  }
  else if(isCommand("enableAxis("))
  {
    int pos;
    sscanf(command,"enableAxis(%d",&pos);
    enableAxis(pos);
  }
  else if(isCommand("setAxisTitle("))
  {
    int pos;
    QString text;
    sscanf(command,"setAxisTitle(%d",&pos);
    if(getText(command,text) != 0) return -1;
    setAxisTitle(pos,text);
  }
  else if(isCommand("setAxisOptions("))
  {
    int pos,val;
    sscanf(command,"setAxisOptions(%d,%d",&pos,&val);
    //xx setAxisOptions(pos,val);
  }
  else if(isCommand("setAxisMaxMajor("))
  {
    int pos,val;
    sscanf(command,"setAxisMaxMajor(%d,%d",&pos,&val);
    setAxisMaxMajor(pos,val);
  }
  else if(isCommand("setAxisMaxMinor("))
  {
    int pos,val;
    sscanf(command,"setAxisMaxMinor(%d,%d",&pos,&val);
    setAxisMaxMinor(pos,val);
  }
  else if(isCommand("insertCurve("))
  {
    int pos;
    QString text;
    sscanf(command,"insertCurve(%d",&pos);
    if(getText(command,text) != 0) return -1;
    if(pos<0 || pos>=nCurves) return -1;
    //xx removeCurve( curves[pos] );
    //xx curves[pos] = insertCurve(text);
    if(curves[pos] != NULL) delete curves[pos];
    if(opt.arg_debug) printf("new QwtPlotCurve(%s)\n",(const char *) text.toUtf8());
    curves[pos] = new QwtPlotCurve(text);
    curves[pos]->attach(this);
    if(legend != NULL)
    {
      QWidget *w = legend->find(curves[pos]);
      if(w != NULL) 
      {
        if(opt.arg_debug) printf("setChecked(%d)\n",autolegend);
        ((QwtLegendItem *)w)->setChecked(autolegend);
      }
    }
    replot();

    //if(autolegend && legend != NULL) curves[pos]->updateLegend(legend);
  }
  else if(isCommand("removeCurve("))
  {
    int pos;
    sscanf(command,"removeCurve(%d",&pos);
    if(pos<0 || pos>=nCurves) return -1;
    //xx removeCurve( curves[pos] );
    if(curves[pos] != NULL) curves[pos]->detach();
    if(curves[pos] != NULL) delete curves[pos];
    curves[pos] = NULL;
  }
  else if(isCommand("setCurvePen("))
  {
    int pos,r,g,b,width,style;
    sscanf(command,"setCurvePen(%d,%d,%d,%d,%d,%d",&pos,&r,&g,&b,&width,&style);
    if(pos<0 || pos>=nCurves) return -1;
    //xx setCurvePen(curves[pos],QPen(QColor(r,g,b),width,(Qt::PenStyle) style));
    QPen pen(QColor(r,g,b));
    pen.setWidth(width);
    pen.setStyle((Qt::PenStyle) style);
    if(curves[pos] != NULL) curves[pos]->setPen(pen);
  }
  else if(isCommand("setCurveSymbol("))
  {
    int pos,symbol,r1,g1,b1,r2,g2,b2,w,h;
    sscanf(command,"setCurveSymbol(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",&pos,&symbol,
                    &r1,&g1,&b1,&r2,&g2,&b2,&w,&h);
    if(pos<0 || pos>=nCurves) return -1;
    //xx setCurveSymbol(curves[pos], QwtSymbol((QwtSymbol::Style) symbol, QColor(r1,g1,b1), QColor(r2,g2,b2), QSize(w,h)));
    if(curves[pos] != NULL) curves[pos]->setSymbol(QwtSymbol((QwtSymbol::Style) symbol, QColor(r1,g1,b1), QColor(r2,g2,b2), QSize(w,h)));
  }
  else if(isCommand("setCurveYAxis("))
  {
    int pos,pos2;
    sscanf(command,"setCurveYAxis(%d,%d",&pos,&pos2);
    if(pos<0 || pos>=nCurves) return -1;
    //xx setCurveYAxis(curves[pos],pos2); rl2013
    if(curves[pos] != NULL) curves[pos]->setYAxis(pos2);
  }
  else if(isCommand("insertMarker("))
  {
    int pos;
    sscanf(command,"insertMarker(%d",&pos);
    if(pos<0 || pos>=nMarker) return -1;
    //xx marker[pos] = insertMarker();
    if(marker[pos] != NULL) delete marker[pos];
    marker[pos] = new QwtPlotMarker();
    marker[pos]->attach(this);
  }
  else if(isCommand("setMarkerLineStyle("))
  {
    int pos,style;
    sscanf(command,"setMarkerLineStyle(%d,%d",&pos,&style);
    if(pos<0 || pos>=nMarker) return -1;
    //xx setMarkerLineStyle(marker[pos],(QwtMarker::LineStyle) style);
    if(marker[pos] != NULL) marker[pos]->setLineStyle((QwtPlotMarker::LineStyle) style);
  }
  else if(isCommand("setMarkerPos("))
  {
    int pos;
    float x,y;
    sscanf(command,"setMarkerPos(%d,%f,%f",&pos,&x,&y);
    if(pos<0 || pos>=nMarker) return -1;
    //xx setMarkerPos(marker[pos],x,y);
    if(marker[pos] != NULL) marker[pos]->setValue(x,y);
  }
  else if(isCommand("setMarkerLabelAlign("))
  {
    int pos,align;
    sscanf(command,"setMarkerLabelAlign(%d,%d",&pos,&align);
    if(pos<0 || pos>=nMarker) return -1;
    //xx setMarkerLabelAlign(marker[pos],align);
    if(marker[pos] != NULL) marker[pos]->setLabelAlignment((Qt::Alignment) align);
  }
  else if(isCommand("setMarkerLabel("))
  {
    int pos;
    QString text;
    sscanf( command, "setMarkerLabel(%d",&pos );
    if(getText(command, text) != 0 ) return -1;
    //xx setMarkerLabel(marker[pos], text );
    if(marker[pos] != NULL) marker[pos]->setLabel(text);
  }
  else if(isCommand("setMarkerPen("))
  {
    int pos,r,g,b,style;
    sscanf(command,"setMarkerPen(%d,%d,%d,%d,%d",&pos,&r,&g,&b,&style);
    if(pos<0 || pos>=nMarker) return -1;
    //xx setMarkerPen(marker[pos],QPen(QColor(r,g,b),0,(Qt::PenStyle) style));
    if(marker[pos] != NULL) marker[pos]->setLinePen(QPen(QColor(r,g,b)));
  }
  else if(isCommand("setMarkerFont("))
  {
    int pos,size,style;
    QString family;
    sscanf(command,"setMarkerFont(%d,%d,%d",&pos,&size,&style);
    if(pos<0 || pos>=nMarker) return -1;
    if(getText(command,family) != 0) return -1;
    //xx setMarkerFont(marker[pos],QFont(family,size,style));
  }
  else if(isCommand("setMarkerSymbol("))
  {
    int pos,symbol,r1,g1,b1,r2,g2,b2,w,h;
    sscanf(command,"setMarkerSymbol(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                  &pos,&symbol,&r1,&g1,&b1,&r2,&g2,&b2,&w,&h);
    if(pos<0 || pos>=nMarker) return -1;
    //xx setMarkerSymbol(marker[pos], QwtSymbol((QwtSymbol::Style) symbol, QColor(r1,g1,b1), QColor(r2,g2,b2), QSize(w,h)));
    if(marker[pos] != NULL) marker[pos]->setSymbol(QwtSymbol((QwtSymbol::Style) symbol, QColor(r1,g1,b1), QColor(r2,g2,b2), QSize(w,h)));
  }
  else if(isCommand("insertLineMarker("))
  {
    int pos,pos2;
    QString text;
    sscanf(command,"insertLineMarker(%d,%d",&pos,&pos2);
    if(getText(command,text) != 0) return -1;
    if(pos<0 || pos>=nMarker) return -1;
    //xx marker[pos] = insertLineMarker(text, pos2);
    if(marker[pos] != NULL) delete marker[pos];
    marker[pos] = new QwtPlotMarker();
    marker[pos]->attach(this);
    if(marker[pos] != NULL) marker[pos]->setLabel(text);
  }
  else if( isCommand("setAxisScaleDraw("))
  {
    int pos;
    QString qtext;
    char text[1024];
    sscanf( command, "setAxisScaleDraw(%d",&pos );
    if( getText(command, qtext) != 0 ) return -1;
    if(qtext.length() < ((int) sizeof(text) -1))
    {
      strcpy(text,qtext.toUtf8());
    }
    else
    {
      strcpy(text,"text too long");
    }
    if(opt.arg_debug) printf("setAxisScaleDraw(%s)\n",text);
    setAxisScaleDraw( pos, new UserScaleDraw( text ));
  }
  else if( isCommand("setAxisScale("))
  {
    int pos;
    float min,max,step;
    sscanf( command, "setAxisScale(%d,%f,%f,%f",&pos ,&min, &max, &step);
    if(opt.arg_debug) printf("setAxisScale(%d,%f,%f,%f)\n",pos,min,max,step);
    setAxisScale(pos,min,max,step);
  }
  else
  {
    return -1;
  }

  return 0;
}

void QwtPlotWidget::slotMouseMoved( const QMouseEvent &e)
{
  double x,y;
  char buf[100];

  x = this->invTransform(QwtPlot::xBottom, e.pos().x());
  y = this->invTransform(QwtPlot::yLeft, e.pos().y());
  sprintf( buf, "QPlotMouseMoved(%d,%f,%f)\n",id, x, y );
  tcp_send(s,buf,strlen(buf));
}

void QwtPlotWidget::slotMousePressed( const QMouseEvent &e)
{
  double x,y;
  char buf[100];

  x = this->invTransform(QwtPlot::xBottom, e.pos().x());
  y = this->invTransform(QwtPlot::yLeft, e.pos().y());
  sprintf( buf, "QPlotMousePressed(%d,%f,%f)\n",id, x, y );
  tcp_send(s,buf,strlen(buf));
}

void QwtPlotWidget::slotMouseReleased( const QMouseEvent &e)
{
  double x,y;
  char buf[100];

  x = this->invTransform(QwtPlot::xBottom, e.pos().x());
  y = this->invTransform(QwtPlot::yLeft, e.pos().y());
  sprintf( buf, "QPlotMouseReleased(%d,%f,%f)\n",id, x, y );
  tcp_send(s,buf,strlen(buf));
}

void QwtPlotWidget::mouseMoveEvent(QMouseEvent *event)
{
  slotMouseMoved(*event);
  QwtPlot::mouseMoveEvent(event);
}

void QwtPlotWidget::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    slotMousePressed(*event);
  }
  QwtPlot::mousePressEvent(event);
}

void QwtPlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    slotMouseReleased(*event);
  }
  QwtPlot::mouseReleaseEvent(event);
}

void QwtPlotWidget::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtPlot::enterEvent(event);
}

void QwtPlotWidget::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  tcp_send(s,buf,strlen(buf));
  QwtPlot::leaveEvent(event);
}

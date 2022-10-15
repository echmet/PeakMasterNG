// vim: sw=2 ts=2 sts=2 expandtab

#ifndef ANALYTESCOMPOSITIONWIDGET_H
#define ANALYTESCOMPOSITIONWIDGET_H

#include "compositioneditorwidget.h"

class AnalytesConstituentsModel;

class AnalytesCompositionWidget : public CompositionEditorWidget {
  Q_OBJECT
public:
  AnalytesCompositionWidget(AnalytesConstituentsModel *model, QWidget *parent = nullptr);
};

#endif // ANALYTESCOMPOSITIONWIDGET_H

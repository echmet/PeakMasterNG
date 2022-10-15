// vim: sw=2 ts=2 sts=2 expandtab

#ifndef BACKGROUNDCOMPOSITIONWIDGET_H
#define BACKGROUNDCOMPOSITIONWIDGET_H

#include "compositioneditorwidget.h"

class BackgroundConstituentsModel;

class BackgroundCompositionWidget : public CompositionEditorWidget {
  Q_OBJECT
public:
  BackgroundCompositionWidget(BackgroundConstituentsModel *model, QWidget *parent = nullptr);
};

#endif // BACKGROUNDCOMPOSITIONWIDGET_H

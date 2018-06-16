#ifndef HACKS_H
#define HACKS_H

#ifdef Q_OS_WIN
  #include <QSplitter>
  #include <QVBoxLayout>
#else
  class QSplitter;
#endif // Q_OS_

namespace hacks {

inline
void makeSplitterAppear(QSplitter *splitter)
{
#ifdef Q_OS_WIN
  QSplitterHandle* handle = splitter->handle(splitter->count()-1);
  QVBoxLayout* layout = new QVBoxLayout(handle);
  layout->setSpacing(5);
  layout->setMargin(0);

  QFrame* line = new QFrame(handle);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  layout->addWidget(line);
#else
  (void)splitter;
#endif
}

}

#endif // HACKS_H

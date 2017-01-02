#include "PyMeshToolbox.hh"


PyMeshToolbox::PyMeshToolbox(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

void PyMeshToolbox::showEvent ( QShowEvent * _event )
{
  QWidget::showEvent ( _event );
  Q_EMIT showing();
}

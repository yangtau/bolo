#include "listview.h"

ListView::ListView(QWidget *parent)
    : QListView(parent)
{

};

ListView::~ListView()
{

};

void ListView::mousePressEvent(QMouseEvent* event)
{

    QModelIndex index = this->indexAt(event->pos());
    if (!index.isValid())
    {
        setCurrentIndex(index);
    }
    else
    {
        QListView::mousePressEvent(event);
    }
};

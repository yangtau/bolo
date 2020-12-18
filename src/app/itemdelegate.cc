#include "itemdelegate.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

#include "itemdef.h"

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent){

      };

ItemDelegate::~ItemDelegate(){

};

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const {
  QStyleOptionViewItem viewOption(option);

  // get the data
  QVariant variant = index.data(Qt::UserRole);
  ItemData data = variant.value<ItemData>();

  // get the picture
  QPixmap file = QPixmap(":/images/File.jpg");
  QPixmap file_select = QPixmap(":/images/File1.jpg");
  QPixmap file_over = QPixmap(":/images/File2.jpg");

  // get the field of painting
  QRect image_rect = QRect(viewOption.rect.left(), viewOption.rect.top(), 100, 100);
  QRect fname_rect = QRect(viewOption.rect.left(), viewOption.rect.top() + 100, 100, 10);

  if (option.state.testFlag(QStyle::State_Selected)) {
    painter->setBrush(QColor(157, 197, 255));
    painter->drawRect(viewOption.rect);
    painter->drawPixmap(image_rect, file_select);

    painter->setPen(QPen(Qt::black));
    painter->setFont(QFont("Times", 12, QFont::Bold));
    painter->drawText(fname_rect, Qt::AlignCenter, data.file_name);
  } else if (option.state.testFlag(QStyle::State_MouseOver)) {
    painter->setBrush(QColor(207, 221, 250));
    painter->drawRect(viewOption.rect);
    painter->drawPixmap(image_rect, file_over);

    painter->setPen(QPen(Qt::black));
    painter->setFont(QFont("Times", 12, QFont::Bold));
    painter->drawText(fname_rect, Qt::AlignCenter, data.file_name);
  } else {
    painter->drawPixmap(image_rect, file);

    painter->setPen(QPen(Qt::black));
    painter->setFont(QFont("Times", 12, QFont::Bold));
    painter->drawText(fname_rect, Qt::AlignCenter, data.file_name);
    // qDebug() << data.file_name;
  }
};

bool ItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                               const QStyleOptionViewItem &option, const QModelIndex &index) {
  QRect clicked_rect = QRect(option.rect.left(), option.rect.top(), 100, 110);
  QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

  if (event->type() == QEvent::MouseButtonDblClick && clicked_rect.contains(mouseEvent->pos())) {
    emit RequireDetail(index);
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
};

#include "itemdelegate.h"

#include <QAbstractItemModel>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent){

      };

ItemDelegate::~ItemDelegate(){

};

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const {
  QStyleOptionViewItem viewOption(option);

  QPixmap file = QPixmap(":/images/File.jpg");
  QPixmap file_select = QPixmap(":/images/File1.jpg");
  QPixmap file_over = QPixmap(":/images/File2.jpg");

  QRect image_rect = QRect(viewOption.rect.left(), viewOption.rect.top(), 100, 100);

  if (option.state.testFlag(QStyle::State_Selected)) {
    painter->setBrush(QColor(157, 197, 255));
    painter->drawRect(viewOption.rect);
    painter->drawPixmap(image_rect, file_select);
  } else if (option.state.testFlag(QStyle::State_MouseOver)) {
    painter->setBrush(QColor(207, 221, 250));
    painter->drawRect(viewOption.rect);
    painter->drawPixmap(image_rect, file_over);
  } else
    painter->drawPixmap(image_rect, file);
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

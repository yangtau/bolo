#ifndef ITEMDEF_H
#define ITEMDEF_H

#include <QMetaType>

#include "bolo.h"

struct ItemData {
  QString file_name;
  bolo::BackupFile MyBackupFile;
};

Q_DECLARE_METATYPE(ItemData)

#endif  // ITEMDEF_H

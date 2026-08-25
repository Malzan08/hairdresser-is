#ifndef USER_H
#define USER_H

#include <QString>
#include <QVector>
#include "Role.h"

struct User {
    int idUser{0};
    QString login;
    QString passwordHash;
    QVector<Role> roles;
};

#endif

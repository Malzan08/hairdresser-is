#ifndef CLIENT_H
#define CLIENT_H

#include <QString>

struct Client {
    int idClient{0};
    int idUser{0};
    QString lastName;
    QString firstName;
    QString middleName;
    QString phone;

    QString fullName() const {
        QString result = lastName + " " + firstName;
        if (!middleName.isEmpty()) {
            result += " " + middleName;
        }
        return result;
    }
};

#endif

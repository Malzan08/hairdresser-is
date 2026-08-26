#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QString>

struct Employee {
    int idEmployee{0};
    int idUser{0};
    int idBranch{0};
    QString lastName;
    QString firstName;
    QString middleName;
    QString phone;
    QString position;
    QString branchName;
};

#endif

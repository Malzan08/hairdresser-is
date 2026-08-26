#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QString>
#include <QDateTime>

struct Document {
    int idDocument{0};
    int idBranch{0};
    int idEmployee{0};
    int idClient{0};
    int idHaircut{0};
    QDateTime serviceDate;
    double discountRate{0.0};
    double finalCost{0.0};

    QString branchName;
    QString employeeName;
    QString clientName;
    QString haircutName;
};

#endif

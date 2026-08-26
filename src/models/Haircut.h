#ifndef HAIRCUT_H
#define HAIRCUT_H

#include <QString>

struct Haircut {
    int idHaircut{0};
    QString haircutName;
    QString gender;
    double currentPrice{0.0};
};

#endif

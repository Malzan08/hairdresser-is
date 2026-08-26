#ifndef PRICEHISTORY_H
#define PRICEHISTORY_H

#include <QDateTime>

struct PriceHistory {
    int idPriceHistory{0};
    int idHaircut{0};
    double price{0.0};
    QDateTime changeDate;
};

#endif

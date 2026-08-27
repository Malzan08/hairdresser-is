#ifndef REPORTSTATS_H
#define REPORTSTATS_H

#include <QString>

struct ClientStats {
    int idClient{0};
    QString clientName;
    QString phone;
    int totalVisits{0};
    QString clientStatus;
    double totalSpent{0.0};
};

struct BranchReport {
    int idBranch{0};
    QString branchName;
    int servicesRendered{0};
    double totalRevenue{0.0};
};

#endif

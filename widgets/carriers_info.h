#pragma once
#include <QString>
#include <stdint.h>
#include <vector>
#include <QObject>

struct CarrierModuleInfo
{
    QString name;
    uint64_t purchase;
    uint64_t full_upkeep;
    uint64_t paused_upkeep;
    uint64_t cargo_use;

    operator const QString&() const
    {
        return name;
    }

    CarrierModuleInfo& operator+=(const CarrierModuleInfo& c)
    {
        purchase += c.purchase;
        full_upkeep += c.full_upkeep;
        paused_upkeep += c.paused_upkeep;
        cargo_use += c.cargo_use;
        return *this;
    }

    CarrierModuleInfo& operator-=(const CarrierModuleInfo& c)
    {
        purchase -= c.purchase;
        full_upkeep -= c.full_upkeep;
        paused_upkeep -= c.paused_upkeep;
        cargo_use -= c.cargo_use;
        return *this;
    }
};

inline const std::vector<CarrierModuleInfo> &getCarrierModulesInfoList()
{
    const static std::vector<CarrierModuleInfo> arr =
    {
        {QObject::tr("Refuel"), 40000000, 1500000, 750000, 500},
        {QObject::tr("Repair"), 50000000, 1500000, 750000, 180},
        {QObject::tr("Armoury"), 95000000, 1500000, 750000, 250},
        {QObject::tr("Redemption office"), 150000000, 1850000, 850000, 100},
        {QObject::tr("Shipyard"), 250000000, 6500000, 1800000, 3000},
        {QObject::tr("Outfitting"), 250000000, 5000000, 1500000, 1750},
        {QObject::tr("Secure warehouse(black market)"), 165000000, 2000000, 1250000, 250},
        {QObject::tr("Universal Cartographics"), 150000000, 1850000, 700000, 120},
    };

    return arr;
}

constexpr inline int max_carrier_cargo()
{
    return 25000;
}

constexpr inline int carrier_tank_size()
{
    return 1000;
}

constexpr inline float carrier_max_jump()
{
    return 500.f;
}

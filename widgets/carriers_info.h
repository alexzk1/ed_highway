#pragma once
#include <QObject>
#include <QString>

#include <cstdint>
#include <vector>

struct CarrierModuleInfo
{
    QString name;
    std::uint64_t purchase;
    std::uint64_t full_upkeep;
    std::uint64_t paused_upkeep;
    std::uint64_t cargo_use;

    operator const QString &() const // NOLINT
    {
        return name;
    }

    CarrierModuleInfo &operator+=(const CarrierModuleInfo &c)
    {
        purchase += c.purchase;
        full_upkeep += c.full_upkeep;
        paused_upkeep += c.paused_upkeep;
        cargo_use += c.cargo_use;
        return *this;
    }

    CarrierModuleInfo &operator-=(const CarrierModuleInfo &c)
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
    const static std::vector<CarrierModuleInfo> arr = {
      {QObject::tr("Refuel"), 40'000'000, 1'500'000, 750'000, 500},
      {QObject::tr("Repair"), 50'000'000, 1'500'000, 750'000, 180},
      {QObject::tr("Armoury"), 95'000'000, 1'500'000, 750'000, 250},
      {QObject::tr("Redemption Office"), 150'000'000, 1'850'000, 850'000, 100},
      {QObject::tr("Shipyard"), 250'000'000, 6'500'000, 1'800'000, 3'000},
      {QObject::tr("Outfitting"), 250'000'000, 5'000'000, 1'500'000, 1'750},
      {QObject::tr("Secure Warehouse(Black Market)"), 165'000'000, 2'000'000, 1'250'000, 250},
      {QObject::tr("Universal Cartographics"), 150'000'000, 1'850'000, 700'000, 120},

      // Odyssey addition
      {QObject::tr("Concourse Bar"), 200'000'000, 1'750'000, 1'250'000, 250},
      {QObject::tr("Vista Genomics"), 150'000'000, 1'500'000, 700'000, 120},
      {QObject::tr("Pioneer Supplies"), 250'000'000, 5'000'000, 1'500'000, 200},
    };

    return arr;
}

constexpr inline int max_carrier_cargo()
{
    return 25'000;
}

constexpr inline int carrier_tank_size()
{
    return 1'000;
}

constexpr inline float carrier_max_jump()
{
    return 500.f;
}

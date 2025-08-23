#pragma once

#include <QObject>
#include <QString>

#include <cmath>
#include <cstdint>
#include <optional>
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

///@brief Represents the type of carrier.
enum class ECarrierType : std::uint8_t {
    PersonalCarrier,
    SquadronCarrier,
};

///@brief
class CarrierJumpCalculator
{
  public:
    ///@param carrierType The type of carrier (Personal or Squadron).
    CarrierJumpCalculator() = delete;
    explicit CarrierJumpCalculator(ECarrierType carrierType) :
        carrierType(carrierType)
    {
    }

    ///@brief Computes the fuel use for a jump
    ///@param currentTotalMass The total mass of the ship and cargo on the carrier, excluding fuel
    /// in the tank.
    ///@param jumpDistanceLy The distance of the jump in light years.
    ///@return std::optional<int> The fuel use for the jump, if it is possible. If not possible,
    /// returns std::nullopt.
    [[nodiscard]]
    std::optional<std::uint32_t> compute_fuel_use(const std::uint32_t currentTotalMass,
                                                  const float jumpDistanceLy) const
    {
        if (currentTotalMass > get_carrier_mass_limit() || jumpDistanceLy > carrier_max_jump())
        {
            return std::nullopt;
        }
        return std::round(
          minimum_jump_cost
          + jumpDistanceLy * jd_mul
              * (1.f + static_cast<float>(currentTotalMass) / carrier_cargo_normilizer));
    }

    ///@returns The cargo limit for the given carrier type.
    [[nodiscard]]
    constexpr std::uint32_t get_carrier_mass_limit() const
    {
        return carrierType == ECarrierType::PersonalCarrier ? 25'000 : 60'000;
    }

    ///@returns The size of the fuel tank on a carrier.
    [[nodiscard]]
    constexpr std::uint32_t carrier_tank_size() const
    {
        return 1'000;
    }

    ///@returns The maximum jump distance a carrier can make.
    [[nodiscard]]
    constexpr float carrier_max_jump() const
    {
        return 500.f;
    }

  private:
    ECarrierType carrierType;

    constexpr static float minimum_jump_cost = 5.f;
    constexpr static float jd_mul = 1.f / 8.f;
    constexpr static int carrier_cargo_normilizer = 25'000;
};

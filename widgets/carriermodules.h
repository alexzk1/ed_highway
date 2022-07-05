#pragma once

#include <vector>
#include <QWidget>
#include "carriers_info.h"

namespace Ui
{
    class carriermodules;
}

class CarrierModules : public QWidget
{
    Q_OBJECT

public:

    explicit CarrierModules(QWidget *parent = nullptr);
    ~CarrierModules();
    const static std::vector<CarrierModuleInfo>& getModulesInfoList();
    const CarrierModuleInfo& getTotal() const;
protected:
    void changeEvent(QEvent *e);
private:
    Ui::carriermodules *ui;
    CarrierModuleInfo total;

    void buildGui();
    void showTotal();
};

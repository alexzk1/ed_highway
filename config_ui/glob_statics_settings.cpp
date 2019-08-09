//Oleksiy Zakharov, 2016, alexzkhr@gmail.com

#include "globalsettings.h"

#define DECL_SETT(CLASS,KEY,DEF,args...) {KEY, widgetted_pt(new CLASS(KEY,DEF,args))}
//-----------------------------------------------------------------------------------------------------------------------
//declare all global settings below, so it can be automatically added to setting's
//-----------------------------------------------------------------------------------------------------------------------

const StaticSettingsMap &StaticSettingsMap::getGlobalSetts()
{
    //do not change keys once used, because 1: key-string is  directly used in other code (must be same), 2: users will lose stored value on next run
    //visual order depends on string sort of the keys
    const static StaticSettingsMap list(
    {
        DECL_SETT(GlobalStorableInt, "SYS_NAME_DROP_DELAY", 100, tr("Dropdown system name min delay"),
                  tr("This value defines minimal delay before systems' names will be pulled from website.\nSetting will take effect after restart."), 25, 1200),
    });
    return list;
}

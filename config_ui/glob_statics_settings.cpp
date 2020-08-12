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
        DECL_SETT(GlobalStorableInt, "01_1Int_Revertlen", 10, tr("Amount to keep of from/to system's names (each)."),
                  tr("We store last routes built from/to fields. Those defines how many to keep of each.\nYou can easy revert back later to one of it."),
                  3, 30),

        DECL_SETT(GlobalStorableInt, "01_SYS_NAME_DROP_DELAY", 100, tr("Dropdown system name min delay (ms)."),
                  tr("This value defines minimal delay before systems' names will be pulled from website.\nSetting will take effect after restart."), 25, 1200),

        DECL_SETT(GlobalStorableInt, "02_Int_SHIP_LY", 70, tr("Default jump range (ly)"),
                  tr("This value defines rounded ship's jump range (ly) to be used as default for new plot."), 5, 500),

        DECL_SETT(GlobalStorableInt, "03_Int_PRECISE", 70, tr("Route plotter's precision (%%)."),
                  tr("Increase this to reduce how far off the direct route the system will plot to get to a neutron star (An efficiency of 100 will not deviate from the direct route in order to plot from A to B and will most likely break down the journey into 20000 LY blocks)."),
                  5, 100),
        DECL_SETT(GlobalStorableInt, "03_Int_UNDO", 20, tr("Undo limit for route optimizer."),
                  tr("Sets how many stages can be undone in route optimizer tab. Consumes RAM."),
                  5, 100),

        DECL_SETT(GlobalStorableInt, "04_Int_tritiumstep", 704, tr("Tritium +/- step."),
                  tr("The step of how much to add/remove tritium in FC calc per arrows click (cargo ship size). In tonnes."),
                  1, 2000),

#ifdef OCR_ADDED
        DECL_SETT(GlobalHotkeyStorable, "51_MapOcrHotkey", "CTRL+ALT+M", tr("OCR Galaxy Map and put star's name to clipboard."),
                  tr("Hotkey to OCR Elite Map and extract star's name.")),
#endif

        DECL_SETT(GlobalFileStorable, "70_LogsFolder", QDir::homePath(), tr("Elite's Logs Folder"),
                  tr("Set a folder where *.log files are stored by E.D."),
                  tr("Select logs' folder")),

        DECL_SETT(GlobalStorableBool, "71_TrackEnable", false, tr("Enable logs' tracking."),
                  tr("If enabled starts tracking of the log files for other abilities except routes.\nRestart is required.")),
    });
    return list;
}

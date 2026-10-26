class CfgPatches
{
    class DayZ_Minimal_Interface
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = { "DZ_Data", "DZ_Scripts" };
    };
};

class CfgMods
{
    class DayZ_Minimal_Interface
    {
        dir = "dmt";
        picture = "";
        action = "";
        hideName = 0;
        hidePicture = 1;
        name = "DayZ Minimal Interface";
        credits = "DayZ Minimal Interface";
        author = "DayZ Minimal Interface";
        authorID = "0";
        version = "0.1.0";
        type = "mod";
        dependencies[] = { "Game", "World", "Mission" };

        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {};
            };
            class missionScriptModule
            {
                value = "";
                files[] = { "dmt/Scripts/5_Mission" };
            };
        };
    };
};

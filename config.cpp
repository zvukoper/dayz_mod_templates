class CfgPatches
{
    class DMT_MinUI
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = { "DZ_Data", "DZ_Scripts" };
    };
};

class CfgMods
{
    class DMT_MinUI
    {
        dir = "dmt";
        picture = "";
        action = "";
        hideName = 0;
        hidePicture = 1;
        name = "DMT Minimal UI";
        credits = "DMT";
        author = "DMT";
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

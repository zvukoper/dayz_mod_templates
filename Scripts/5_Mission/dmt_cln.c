// DMT Minimal UI — client. Log/chat are state-transition based, not per-frame.

static const int DMT_RPC_TIME = 19740621;

class DMT_Client
{
    static Widget s_Root;
    static TextWidget s_Time;
    static int s_Year;
    static int s_Month;
    static int s_Day;
    static int s_Hour;
    static int s_Minute;
    static int s_Second;
    static int s_Elapsed;
    static int s_LastChatElapsed = -1;
    static bool s_HasTime;
    static bool s_Inventory;
    static bool s_Ready;
    static float s_Accumulator;
    static string s_LastText = "";

    static string Pad2(int v)
    {
        if (v < 10) return "0" + v.ToString();
        return v.ToString();
    }

    static string Pad4(int v)
    {
        if (v < 10) return "000" + v.ToString();
        if (v < 100) return "00" + v.ToString();
        if (v < 1000) return "0" + v.ToString();
        return v.ToString();
    }

    static void Log(int line, string msg)
    {
        Print("[DMT][dmt_cln:" + line.ToString() + "] " + msg);
    }

    static void Chat(string msg)
    {
        // Short system-channel debug message. Chat output is intentionally
        // limited to startup, UI transitions, and periodic sync milestones.
        Param1<string> p = new Param1<string>("[DMT] " + msg);
        GetGame().RPCSingleParam(null, ERPCs.RPC_SEND_NOTIFICATION, p, true, null);
    }

    static void Init()
    {
        Log(34, "INIT enter");
        if (s_Ready)
        {
            Log(37, "INIT already=1");
            return;
        }

        WorkspaceWidget ws = GetGame().GetWorkspace();
        if (!ws)
        {
            Log(44, "INIT workspace=0");
            return;
        }

        s_Root = ws.CreateWidgets("dmt/GUI/dmt.layout");
        Log(50, "LAYOUT root=" + (s_Root != null).ToString());
        if (!s_Root)
        {
            Log(53, "FATAL layout=0 path=dmt/GUI/dmt.layout");
            return;
        }

        s_Time = TextWidget.Cast(s_Root.FindAnyWidget("DMT_Time"));
        Log(58, "WIDGET DMT_Time=" + (s_Time != null).ToString());
        if (!s_Time)
        {
            s_Root.Unlink();
            s_Root = null;
            Log(63, "FATAL widget=0 name=DMT_Time");
            return;
        }

        s_Root.Show(true);
        s_Ready = true;
        Log(69, "HUD visible=1");
        Chat("HUD OK");
    }

    static void OnServerTime(ParamsReadContext ctx)
    {
        Param7<int, int, int, int, int, int, int> p = new Param7<int, int, int, int, int, int, int>(0, 0, 0, 0, 0, 0, 0);
        if (!ctx.Read(p))
        {
            Log(78, "RPC read=0");
            return;
        }

        s_Year = p.param1;
        s_Month = p.param2;
        s_Day = p.param3;
        s_Hour = p.param4;
        s_Minute = p.param5;
        s_Second = p.param6;
        s_Elapsed = p.param7;
        s_HasTime = true;

        if (s_Elapsed == 1 || (s_Elapsed % 30) == 0)
        {
            Log(91, "RPC ok n=" + s_Elapsed.ToString() + " t=" + Pad2(s_Hour) + ":" + Pad2(s_Minute) + ":" + Pad2(s_Second));
            Chat("RPC OK n=" + s_Elapsed.ToString());
        }
    }

    static void Update(float timeslice)
    {
        if (!s_Ready) return;
        if (!s_Root || !s_Time)
        {
            Log(100, "UPDATE widget lost");
            s_Ready = false;
            return;
        }
        if (!s_HasTime)
        {
            s_Time.SetText("SERVER --:--:--");
            return;
        }

        s_Accumulator += timeslice;
        // Render every frame from the last server packet. Server is authoritative;
        // no client system clock is used. Packet cadence is one second.
        int displaySecond = s_Second + Math.Floor(s_Accumulator);
        int displayMinute = s_Minute + (displaySecond / 60);
        int displayHour = s_Hour + (displayMinute / 60);
        displaySecond = displaySecond % 60;
        displayMinute = displayMinute % 60;
        displayHour = displayHour % 24;

        bool inventory = GetGame().IsInventoryOpen();
        if (inventory != s_Inventory)
        {
            s_Inventory = inventory;
            Log(124, "INV open=" + inventory.ToString());
            Chat("INV " + (inventory ? "OPEN" : "CLOSE"));
        }

        string text = Pad2(displayHour) + ":" + Pad2(displayMinute) + ":" + Pad2(displaySecond);
        if (inventory)
        {
            text = text + "\n" + Pad2(s_Day) + "." + Pad2(s_Month) + "." + Pad4(s_Year);
        }
        if (text != s_LastText)
        {
            s_LastText = text;
            s_Time.SetText(text);
        }
    }

    static int s_Day;
    static int s_Month;
    static int s_Year;
};

modded class MissionGameplay
{
    override void OnInit()
    {
        super.OnInit();
        DMT_Client.Log(148, "MissionGameplay.OnInit");
        DMT_Client.Init();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        DMT_Client.Update(timeslice);
    }
};

modded class DayZGame
{
    override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, target, rpc_type, ctx);
        if (rpc_type == DMT_RPC_TIME)
        {
            DMT_Client.OnServerTime(ctx);
        }
    }
};

// DMT Minimal UI — client. Every state transition is logged; no frame spam.
class DMT_Client
{
    static Widget s_Root;
    static TextWidget s_Time;
    static int s_Year, s_Month, s_Day, s_Hour, s_Minute, s_Second, s_Elapsed;
    static int s_LastRpcSequence;
    static float s_RpcAge;
    static bool s_HasTime, s_Inventory, s_Ready;
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
        // Don't depend on private Chat fields or fabricate a server RPC.
        // These are local system chat messages, not broadcast to other clients.
        MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
        if (!mission || !mission.m_Chat)
        {
            Log(40, "CHAT unavailable");
            return;
        }
        ChatMessageEventParams p = new ChatMessageEventParams(CCSystem, "", "[DMT] " + msg, "");
        mission.m_Chat.Add(p);
    }

    static void Init()
    {
        Log(50, "INIT enter");
        if (s_Ready)
        {
            Log(53, "INIT already=1");
            return;
        }

        WorkspaceWidget ws = GetGame().GetWorkspace();
        if (!ws)
        {
            Log(60, "INIT workspace=0");
            return;
        }

        s_Root = ws.CreateWidgets("dmt/GUI/dmt.layout");
        Log(66, "LAYOUT root=" + (s_Root != null).ToString());
        if (!s_Root)
        {
            Log(69, "FATAL layout=0 path=dmt/GUI/dmt.layout");
            Chat("LAYOUT FAIL");
            return;
        }

        s_Time = TextWidget.Cast(s_Root.FindAnyWidget("DMT_Time"));
        Log(75, "WIDGET DMT_Time=" + (s_Time != null).ToString());
        if (!s_Time)
        {
            s_Root.Unlink();
            s_Root = null;
            Log(80, "FATAL widget=0 name=DMT_Time");
            Chat("WIDGET FAIL");
            return;
        }

        s_Root.Show(true);
        s_Ready = true;
        Log(87, "HUD visible=1");
        Chat("HUD OK");
    }


    static void OnServerTime(ParamsReadContext ctx)
    {
        Log(108, "RPC enter");
        Param7<int, int, int, int, int, int, int> p = new Param7<int, int, int, int, int, int, int>(0, 0, 0, 0, 0, 0, 0);
        if (!ctx.Read(p))
        {
            Log(111, "RPC read=0");
            Chat("RPC READ FAIL");
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
        s_RpcAge = 0;
        s_Accumulator = 0;

        if (s_Elapsed != s_LastRpcSequence + 1 && s_LastRpcSequence > 0)
        {
            Log(126, "RPC GAP prev=" + s_LastRpcSequence.ToString() + " now=" + s_Elapsed.ToString());
            Chat("RPC GAP");
        }
        s_LastRpcSequence = s_Elapsed;

        if (s_Elapsed == 1 || (s_Elapsed % 30) == 0)
        {
            Log(134, "RPC ok n=" + s_Elapsed.ToString() + " t=" + Pad2(s_Hour) + ":" + Pad2(s_Minute) + ":" + Pad2(s_Second));
            Chat("RPC OK n=" + s_Elapsed.ToString());
        }
    }

    static void Update(float timeslice)
    {
        if (!s_Ready) return;
        if (!s_Root || !s_Time)
        {
            Log(126, "UPDATE widget lost");
            s_Ready = false;
            return;
        }
        if (!s_HasTime)
        {
            s_Time.SetText("SERVER --:--:--");
            return;
        }

        s_RpcAge += timeslice;
        if (s_RpcAge >= 5.0)
        {
            Log(101, "RPC stale age=" + s_RpcAge.ToString());
            Chat("RPC STALE");
            s_RpcAge = 0;
        }

        s_Accumulator += timeslice;
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
            Log(145, "INV open=" + inventory.ToString());
            Chat("INV " + (inventory ? "OPEN" : "CLOSE"));
        }

        string text = Pad2(displayHour) + ":" + Pad2(displayMinute) + ":" + Pad2(displaySecond);
        if (inventory)
            text = text + "\\n" + Pad2(s_Day) + "." + Pad2(s_Month) + "." + Pad4(s_Year);

        if (text != s_LastText)
        {
            s_LastText = text;
            s_Time.SetText(text);
        }
    }
};

modded class MissionGameplay
{
    override void OnInit()
    {
        super.OnInit();
        DMT_Client.Log(166, "MissionGameplay.OnInit");
        DMT_Client.Init();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        DMT_Client.Update(timeslice);
    }

    override void OnMissionFinish()
    {
        DMT_Client.Log(178, "MissionGameplay.OnMissionFinish");
        if (DMT_Client.s_Root)
        {
            DMT_Client.s_Root.Unlink();
            DMT_Client.s_Root = null;
        }
        DMT_Client.s_Time = null;
        DMT_Client.s_Ready = false;
        DMT_Client.s_HasTime = false;
        DMT_Client.s_Inventory = false;
        DMT_Client.s_RpcAge = 0;
        DMT_Client.s_LastRpcSequence = 0;
        DMT_Client.s_LastText = "";
        super.OnMissionFinish();
    }
};

modded class DayZGame
{
    override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        // Respect the vanilla dispatcher first, including Event_OnRPC.
        super.OnRPC(sender, target, rpc_type, ctx);
        if (rpc_type == DMT_RPC.TIME)
        {
            DMT_Client.OnServerTime(ctx);
        }
    }
};

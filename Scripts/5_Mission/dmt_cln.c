// DayZ Minimal Interface test interface — client. Log state changes without printing on every frame.
class DMT_Client
{
    static Widget s_Root;
    static TextWidget s_Time;
    static int s_Year, s_Month, s_Day, s_Hour, s_Minute, s_Second, s_Elapsed;
    static int s_LastRemoteProcedureCallSequence;
    static float s_RemoteProcedureCallAge;
    static bool s_RemoteProcedureCallTimedOut;
    static bool s_HasTime, s_Inventory, s_Ready;
    static float s_Accumulator;
    static string s_LastText = "";

    static string Pad2(int v)
    {
        if (v < 10)
        {
            return "0" + v.ToString();
        }
        return v.ToString();
    }

    static string Pad4(int v)
    {
        if (v < 10)
        {
            return "000" + v.ToString();
        }
        if (v < 100)
        {
            return "00" + v.ToString();
        }
        if (v < 1000)
        {
            return "0" + v.ToString();
        }
        return v.ToString();
    }

    static void Log(int line, string msg)
    {
        Print("[DayZ Minimal Interface][dmt_cln:" + line.ToString() + "] " + msg);
    }

    static void Chat(string msg)
    {
        // Do not depend on private chat fields or simulate a remote procedure call.
        // These messages appear only in this player's chat and are not sent to other players.
        MissionGameplay mission = MissionGameplay.Cast(GetGame().GetMission());
        if (!mission || !mission.m_Chat)
        {
            Log(52, "Chat messages are unavailable");
            return;
        }
        ChatMessageEventParams p = new ChatMessageEventParams(CCSystem, "", "[DayZ Minimal Interface] " + msg, "");
        mission.m_Chat.Add(p);
    }

    static void Init()
    {
        Log(61, "Initialization started");
        if (s_Ready)
        {
            Log(64, "Initialization already completed");
            return;
        }

        WorkspaceWidget ws = GetGame().GetWorkspace();
        if (!ws)
        {
            Log(71, "Widget workspace is unavailable");
            return;
        }

        s_Root = ws.CreateWidgets("dmt/GUI/dmt.layout");
        Log(76, "Layout root creation succeeded=" + (s_Root != null).ToString());
        if (!s_Root)
        {
            Log(79, "ERROR layout creation failed path=dmt/GUI/dmt.layout");
            Chat("Interface layout creation failed");
            return;
        }

        s_Time = TextWidget.Cast(s_Root.FindAnyWidget("DMT_Time"));
        Log(85, "Text widget DMT_Time found=" + (s_Time != null).ToString());
        if (!s_Time)
        {
            s_Root.Unlink();
            s_Root = null;
            Log(90, "ERROR required text widget is missing name=DMT_Time");
            Chat("Required text widget was not found");
            return;
        }

        s_Root.Show(true);
        s_Ready = true;
        Log(97, "Interface visibility enabled");
        Chat("Interface initialized successfully");
    }

    static void OnServerTime(ParamsReadContext ctx)
    {
        if (!s_HasTime)
        {
            Log(105, "First server time update received");
        }

        Param7<int, int, int, int, int, int, int> p = new Param7<int, int, int, int, int, int, int>(0, 0, 0, 0, 0, 0, 0);
        if (!ctx.Read(p))
        {
            Log(111, "Remote procedure call data could not be read");
            Chat("Remote procedure call data could not be read");
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
        s_RemoteProcedureCallAge = 0;
        s_RemoteProcedureCallTimedOut = false;
        s_Accumulator = 0;

        if (s_Elapsed <= s_LastRemoteProcedureCallSequence)
        {
            Log(130, "Remote procedure call arrived out of order: previous sequence=" + s_LastRemoteProcedureCallSequence.ToString() + " current sequence=" + s_Elapsed.ToString());
            Chat("Remote procedure call arrived out of order");
        }
        else if (s_LastRemoteProcedureCallSequence > 0 && s_Elapsed > s_LastRemoteProcedureCallSequence + 1)
        {
            Log(135, "Remote procedure call sequence skipped: previous sequence=" + s_LastRemoteProcedureCallSequence.ToString() + " current sequence=" + s_Elapsed.ToString());
            Chat("Remote procedure call sequence skipped");
        }
        s_LastRemoteProcedureCallSequence = s_Elapsed;

        if (s_Elapsed == 1 || (s_Elapsed % 30) == 0)
        {
            Log(142, "Server time received successfully: sequence=" + s_Elapsed.ToString() + " time=" + Pad2(s_Hour) + ":" + Pad2(s_Minute) + ":" + Pad2(s_Second));
            Chat("Server time received successfully: sequence=" + s_Elapsed.ToString());
        }
    }

    static void Update(float timeslice)
    {
        if (!GetGame() || GetGame().IsServer())
        {
            return;
        }
        if (!s_Ready)
        {
            return;
        }
        if (!s_Root || !s_Time)
        {
            Log(159, "Interface update stopped because a required widget is missing");
            s_Ready = false;
            return;
        }
        if (!s_HasTime)
        {
            s_Time.SetText("Waiting for server time...");
            return;
        }

        s_RemoteProcedureCallAge += timeslice;
        if (s_RemoteProcedureCallAge >= 5.0 && !s_RemoteProcedureCallTimedOut)
        {
            s_RemoteProcedureCallTimedOut = true;
            Log(173, "Server time updates have stopped: elapsed seconds=" + s_RemoteProcedureCallAge.ToString());
            Chat("Server time updates have stopped");
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
            if (inventory)
            {
                Log(191, "Inventory opened");
                Chat("Inventory opened");
            }
            else
            {
                Log(196, "Inventory closed");
                Chat("Inventory closed");
            }
        }

        string text = Pad2(displayHour) + ":" + Pad2(displayMinute) + ":" + Pad2(displaySecond);
        if (inventory)
        {
            text = text + "\\n" + Pad2(s_Day) + "." + Pad2(s_Month) + "." + Pad4(s_Year);
        }

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
        if (!GetGame() || GetGame().IsServer())
        {
            return;
        }
        DMT_Client.Log(224, "Mission gameplay initialization started");
        DMT_Client.Init();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        if (!GetGame() || GetGame().IsServer())
        {
            return;
        }
        DMT_Client.Update(timeslice);
    }

    override void OnMissionFinish()
    {
        DMT_Client.Log(240, "Mission gameplay cleanup started");
        if (DMT_Client.s_Root)
        {
            DMT_Client.s_Root.Unlink();
            DMT_Client.s_Root = null;
        }
        DMT_Client.s_Time = null;
        DMT_Client.s_Ready = false;
        DMT_Client.s_HasTime = false;
        DMT_Client.s_Inventory = false;
        DMT_Client.s_RemoteProcedureCallAge = 0;
        DMT_Client.s_RemoteProcedureCallTimedOut = false;
        DMT_Client.s_LastRemoteProcedureCallSequence = 0;
        DMT_Client.s_LastText = "";
        super.OnMissionFinish();
    }
};

modded class DayZGame
{
    override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        // Preserve the standard event dispatcher before handling the time update.
        super.OnRPC(sender, target, rpc_type, ctx);
        if (GetGame() && !GetGame().IsServer() && rpc_type == DMT_RemoteProcedureCallId.SERVER_TIME_UPDATE)
        {
            DMT_Client.OnServerTime(ctx);
        }
    }
};
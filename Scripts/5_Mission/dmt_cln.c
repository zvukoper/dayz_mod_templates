// DayZ Minimal Interface test interface — client. Log state changes without printing on every frame.
class DMT_Client
{
    static Widget s_Root;
    static TextWidget s_Time;
    static int s_Year, s_Month, s_Day, s_Hour, s_Minute, s_Second, s_Elapsed;
    static int s_LastRemoteProcedureCallSequence;
    static int s_LastProcessedNetworkUpdateCount;
    static int s_LastReportedReadErrorCount;
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
            Log(54, "Chat messages are unavailable");
            return;
        }
        ChatMessageEventParams p = new ChatMessageEventParams(CCSystem, "", "[DayZ Minimal Interface] " + msg, "");
        mission.m_Chat.Add(p);
    }

    static void Init()
    {
        Log(63, "Initialization started");
        if (s_Ready)
        {
            Log(66, "Initialization already completed");
            return;
        }

        WorkspaceWidget ws = GetGame().GetWorkspace();
        if (!ws)
        {
            Log(73, "Widget workspace is unavailable");
            return;
        }

        s_Root = ws.CreateWidgets("dmt/GUI/dmt.layout");
        Log(78, "Layout root creation succeeded=" + (s_Root != null).ToString());
        if (!s_Root)
        {
            Log(81, "ERROR layout creation failed path=dmt/GUI/dmt.layout");
            Chat("Interface layout creation failed");
            return;
        }

        s_Time = TextWidget.Cast(s_Root.FindAnyWidget("DMT_Time"));
        Log(87, "Text widget DMT_Time found=" + (s_Time != null).ToString());
        if (!s_Time)
        {
            s_Root.Unlink();
            s_Root = null;
            Log(92, "ERROR required text widget is missing name=DMT_Time");
            Chat("Required text widget was not found");
            return;
        }

        s_Root.Show(true);
        s_Ready = true;
        Log(99, "Interface visibility enabled");
        Chat("Interface initialized successfully");
    }

    static void OnServerTime(int year, int month, int day, int hour, int minute, int second, int elapsed)
    {
        if (!s_HasTime)
        {
            Log(107, "First server time update received");
        }

        s_Year = year;
        s_Month = month;
        s_Day = day;
        s_Hour = hour;
        s_Minute = minute;
        s_Second = second;
        s_Elapsed = elapsed;
        s_HasTime = true;
        s_RemoteProcedureCallAge = 0;
        s_RemoteProcedureCallTimedOut = false;
        s_Accumulator = 0;

        if (s_Elapsed <= s_LastRemoteProcedureCallSequence)
        {
            Log(124, "Remote procedure call arrived out of order: previous sequence=" + s_LastRemoteProcedureCallSequence.ToString() + " current sequence=" + s_Elapsed.ToString());
            Chat("Remote procedure call arrived out of order");
        }
        else if (s_LastRemoteProcedureCallSequence > 0 && s_Elapsed > s_LastRemoteProcedureCallSequence + 1)
        {
            Log(129, "Remote procedure call sequence skipped: previous sequence=" + s_LastRemoteProcedureCallSequence.ToString() + " current sequence=" + s_Elapsed.ToString());
            Chat("Remote procedure call sequence skipped");
        }
        s_LastRemoteProcedureCallSequence = s_Elapsed;

        if (s_Elapsed == 1 || (s_Elapsed % 30) == 0)
        {
            Log(136, "Server time received successfully: sequence=" + s_Elapsed.ToString() + " time=" + Pad2(s_Hour) + ":" + Pad2(s_Minute) + ":" + Pad2(s_Second));
            Chat("Server time received successfully: sequence=" + s_Elapsed.ToString());
        }
    }

    static void Update(float timeslice)
    {
        if (!GetGame() || GetGame().IsServer())
        {
            return;
        }
        if (DMT_NetworkTimeState.s_ReadErrorCount > s_LastReportedReadErrorCount)
        {
            s_LastReportedReadErrorCount = DMT_NetworkTimeState.s_ReadErrorCount;
            Log(150, "Remote procedure call data could not be read");
            Chat("Remote procedure call data could not be read");
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
        if (DMT_NetworkTimeState.s_ReceivedUpdateCount > s_LastProcessedNetworkUpdateCount)
        {
            s_LastProcessedNetworkUpdateCount = DMT_NetworkTimeState.s_ReceivedUpdateCount;
            OnServerTime( DMT_NetworkTimeState.s_Year, DMT_NetworkTimeState.s_Month, DMT_NetworkTimeState.s_Day, DMT_NetworkTimeState.s_Hour, DMT_NetworkTimeState.s_Minute, DMT_NetworkTimeState.s_Second, DMT_NetworkTimeState.s_Sequence );
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
            Log(178, "Server time updates have stopped: elapsed seconds=" + s_RemoteProcedureCallAge.ToString());
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
                Log(196, "Inventory opened");
                Chat("Inventory opened");
            }
            else
            {
                Log(201, "Inventory closed");
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
        DMT_Client.Log(229, "Mission gameplay initialization started");
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
        DMT_Client.Log(245, "Mission gameplay cleanup started");
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
        DMT_Client.s_LastProcessedNetworkUpdateCount = 0;
        DMT_Client.s_LastReportedReadErrorCount = 0;
        DMT_Client.s_LastText = "";
        DMT_NetworkTimeState.Reset();
        super.OnMissionFinish();
    }
};


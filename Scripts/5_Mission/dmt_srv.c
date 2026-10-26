// DMT Minimal UI — server. One compact log line per second.
static const int DMT_RPC_TIME = 19740621;

class DMT_Server
{
    static int s_Elapsed;\n    static int s_LastWorldMinuteKey;\n    static int s_SecondPhase;
    static int s_BaseYear, s_BaseMonth, s_BaseDay, s_BaseHour, s_BaseMinute;
    static bool s_Started;

    static string Pad2(int v)
    {
        if (v < 10) return "0" + v.ToString();
        return v.ToString();
    }

    static void Log(string file, int line, string msg)
    {
        Print("[DMT][" + file + ":" + line.ToString() + "] " + msg);
    }

    static void Start()
    {
        if (s_Started)
        {
            Log("dmt_srv", 24, "START already=1");
            return;
        }
        s_Started = true;
        s_Elapsed = 0;\n        s_LastWorldMinuteKey = -1;\n        s_SecondPhase = 0;

        if (GetGame() && GetGame().GetWorld())
        {
            GetGame().GetWorld().GetDate(s_BaseYear, s_BaseMonth, s_BaseDay, s_BaseHour, s_BaseMinute);
            Log("dmt_srv", 32, "BASE date=" + s_BaseYear.ToString() + "-" + Pad2(s_BaseMonth) + "-" + Pad2(s_BaseDay) + " time=" + Pad2(s_BaseHour) + ":" + Pad2(s_BaseMinute));
        }
        else
            Log("dmt_srv", 35, "BASE world=0");

        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 1000, true);
        Log("dmt_srv", 38, "START queue=1000ms");
    }

    static void Stop()
    {
        if (!s_Started) return;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
        s_Started = false;
        Log("dmt_srv", 46, "STOP");
    }

    static void Tick()
    {
        if (!GetGame() || !GetGame().GetWorld())
        {
            Log("dmt_srv", 52, "TICK world=0");
            return;
        }

        s_Elapsed++;
        int second = s_Elapsed % 60;
        int shownMinute = s_BaseMinute + (s_Elapsed / 60);
        int shownHour = s_BaseHour + (shownMinute / 60);
        int shownDay = s_BaseDay;
        int shownMonth = s_BaseMonth;
        int shownYear = s_BaseYear;
        shownMinute = shownMinute % 60;
        shownHour = shownHour % 24;

        // DayZ time acceleration/commands must be respected: derive each packet
        // from the current world hour/minute, and use elapsed seconds only
        // within the current minute. If world minute changes, reset second phase.
        int worldYear, worldMonth, worldDay, worldHour, worldMinute;
        GetGame().GetWorld().GetDate(worldYear, worldMonth, worldDay, worldHour, worldMinute);
        shownYear = worldYear;
        shownMonth = worldMonth;
        shownDay = worldDay;
        shownHour = worldHour;
        shownMinute = worldMinute;

        if (s_Elapsed == 1)
            s_LastWorldMinuteKey = worldYear * 100000000 + worldMonth * 1000000 + worldDay * 10000 + worldHour * 100 + worldMinute;

        int minuteKey = worldYear * 100000000 + worldMonth * 1000000 + worldDay * 10000 + worldHour * 100 + worldMinute;
        if (minuteKey != s_LastWorldMinuteKey)
        {
            s_LastWorldMinuteKey = minuteKey;
            s_SecondPhase = 0;
            Log("dmt_srv", 73, "WORLD minute=" + Pad2(worldHour) + ":" + Pad2(worldMinute));
        }
        else
            s_SecondPhase++;

        int second = s_SecondPhase % 60;

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        int sent = 0;
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity()) continue;

            Param7<int, int, int, int, int, int, int> payload = new Param7<int, int, int, int, int, int, int>(
                shownYear, shownMonth, shownDay, shownHour, shownMinute, second, s_Elapsed
            );
            GetGame().RPCSingleParam(player, DMT_RPC_TIME, payload, true, player.GetIdentity());
            sent++;
        }

        Log("dmt_srv", 102, "SYNC n=" + s_Elapsed.ToString() + " t=" + Pad2(shownHour) + ":" + Pad2(shownMinute) + ":" + Pad2(second) + " p=" + sent.ToString());
    }

    static int DaysInMonth(int year, int month)
    {
        if (month == 2)
        {
            if ((year % 400) == 0 || ((year % 4) == 0 && (year % 100) != 0)) return 29;
            return 28;
        }
        if (month == 4 || month == 6 || month == 9 || month == 11) return 30;
        return 31;
    }
};

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        DMT_Server.Log("dmt_srv", 125, "MissionServer.OnInit");
        DMT_Server.Start();
        DMT_Server.Log("dmt_srv", 128, "MissionServer.ready");
    }

    override void OnMissionFinish()
    {
        DMT_Server.Log("dmt_srv", 134, "MissionServer.OnMissionFinish");
        DMT_Server.Stop();
        super.OnMissionFinish();
    }
};

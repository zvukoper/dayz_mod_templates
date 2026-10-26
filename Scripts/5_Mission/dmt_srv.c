// DMT Minimal UI — server. One compact log line per second.
static const int DMT_RPC_TIME = 19740621;

class DMT_Server
{
    static int s_Elapsed;
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
        s_Elapsed = 0;

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

        // Roll date at midnight. DayZ world API exposes date to minute precision;
        // this compact clock is synchronized to the server mission start.
        if ((s_BaseHour + ((s_BaseMinute + (s_Elapsed / 60)) / 60)) >= 24)
        {
            int totalMinutes = s_BaseHour * 60 + s_BaseMinute + (s_Elapsed / 60);
            int daysPassed = totalMinutes / 1440;
            int baseDay = s_BaseDay + daysPassed;
            // Month lengths incl. leap years. Small loop runs only at a day rollover.
            while (baseDay > DaysInMonth(shownYear, shownMonth))
            {
                baseDay = baseDay - DaysInMonth(shownYear, shownMonth);
                shownMonth++;
                if (shownMonth > 12)
                {
                    shownMonth = 1;
                    shownYear++;
                }
            }
            shownDay = baseDay;
        }

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

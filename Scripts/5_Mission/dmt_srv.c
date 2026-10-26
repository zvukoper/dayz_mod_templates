// DMT Minimal UI — server. One compact log line per second.
static const int DMT_RPC_TIME = 19740621;

class DMT_Server
{
    static int s_Elapsed;
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
            Log("dmt_srv", 22, "START already=1");
            return;
        }
        s_Started = true;
        s_Elapsed = 0;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 1000, true);
        Log("dmt_srv", 29, "START queue=1000ms");
    }

    static void Stop()
    {
        if (!s_Started) return;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
        s_Started = false;
        Log("dmt_srv", 37, "STOP");
    }

    static void Tick()
    {
        if (!GetGame() || !GetGame().GetWorld())
        {
            Log("dmt_srv", 43, "TICK world=0");
            return;
        }

        int year, month, day, hour, minute;
        GetGame().GetWorld().GetDate(year, month, day, hour, minute);

        // DayZ world date exposes hours/minutes, not wall-clock seconds.
        // Build a continuous once-per-second server clock, anchored to the
        // world time at mission start. A new mission resets the elapsed clock.
        s_Elapsed++;
        int second = s_Elapsed % 60;
        int minuteCarry = s_Elapsed / 60;
        int shownMinute = minute + minuteCarry;
        int shownHour = hour + (shownMinute / 60);
        shownMinute = shownMinute % 60;
        shownHour = shownHour % 24;

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        int sent = 0;
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity()) continue;

            Param7<int, int, int, int, int, int, int> payload = new Param7<int, int, int, int, int, int, int>(
                year, month, day, shownHour, shownMinute, second, s_Elapsed
            );
            GetGame().RPCSingleParam(player, DMT_RPC_TIME, payload, true, player.GetIdentity());
            sent++;
        }

        Log("dmt_srv", 74, "SYNC n=" + s_Elapsed.ToString() + " t=" + Pad2(shownHour) + ":" + Pad2(shownMinute) + ":" + Pad2(second) + " p=" + sent.ToString());
    }
};

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        DMT_Server.Log("dmt_srv", 84, "MissionServer.OnInit");
        DMT_Server.Start();
        DMT_Server.Log("dmt_srv", 87, "MissionServer.ready");
    }

    override void OnMissionFinish()
    {
        DMT_Server.Log("dmt_srv", 93, "MissionServer.OnMissionFinish");
        DMT_Server.Stop();
        super.OnMissionFinish();
    }
};

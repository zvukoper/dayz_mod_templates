// DMT Minimal UI — server. Compact logs; authoritative clock packet once/sec.
static const int DMT_RPC_TIME = 19740621;

class DMT_Server
{
    static int s_Elapsed;
    static int s_LastWorldMinuteKey;
    static int s_SecondPhase;
    static bool s_Started;

    static string Pad2(int v)
    {
        if (v < 10) return "0" + v.ToString();
        return v.ToString();
    }

    static void Log(int line, string msg)
    {
        Print("[DMT][dmt_srv:" + line.ToString() + "] " + msg);
    }

    static void Start()
    {
        if (s_Started)
        {
            Log(22, "START already=1");
            return;
        }
        s_Started = true;
        s_Elapsed = 0;
        s_LastWorldMinuteKey = -1;
        s_SecondPhase = 0;

        Log(30, "START queue=1000ms");
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 1000, true);
    }

    static void Stop()
    {
        if (!s_Started) return;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
        s_Started = false;
        Log(40, "STOP");
    }

    static void Tick()
    {
        if (!GetGame() || !GetGame().GetWorld())
        {
            Log(46, "TICK world=0");
            return;
        }

        int year, month, day, hour, minute;
        GetGame().GetWorld().GetDate(year, month, day, hour, minute);
        int minuteKey = year * 100000000 + month * 1000000 + day * 10000 + hour * 100 + minute;

        if (minuteKey != s_LastWorldMinuteKey)
        {
            s_LastWorldMinuteKey = minuteKey;
            s_SecondPhase = 0;
            Log(56, "WORLD " + year.ToString() + "-" + Pad2(month) + "-" + Pad2(day) + " " + Pad2(hour) + ":" + Pad2(minute));
        }
        else
        {
            s_SecondPhase++;
        }

        int second = s_SecondPhase % 60;
        s_Elapsed++;

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        int sent = 0;
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity()) continue;

            Param7<int, int, int, int, int, int, int> payload = new Param7<int, int, int, int, int, int, int>(
                year, month, day, hour, minute, second, s_Elapsed
            );
            GetGame().RPCSingleParam(player, DMT_RPC_TIME, payload, true, player.GetIdentity());
            sent++;
        }

        if (s_Elapsed == 1 || s_Elapsed % 30 == 0 || sent == 0)
        {
            Log(79, "SYNC n=" + s_Elapsed.ToString() + " t=" + Pad2(hour) + ":" + Pad2(minute) + ":" + Pad2(second) + " p=" + sent.ToString());
        }
    }
};

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        DMT_Server.Log(90, "MissionServer.OnInit");
        DMT_Server.Start();
        DMT_Server.Log(93, "MissionServer.ready");
    }

    override void OnMissionFinish()
    {
        DMT_Server.Log(99, "MissionServer.OnMissionFinish");
        DMT_Server.Stop();
        super.OnMissionFinish();
    }
};

// DMT Minimal UI — server. Compact logs; authoritative clock packet once/sec.


class DMT_RPC
{
    static const int TIME = 19740621;
};

class DMT_Server
{
    static int s_Elapsed;
    static bool s_NoPlayers;
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
        s_NoPlayers = false;

        Log(30, "START queue=1000ms");
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 1000, true);
    }

    static void Tick()
    {
        if (!GetGame())
        {
            Log(46, "TICK game=0");
            return;
        }

        int year, month, day, hour, minute, second;
        GetYearMonthDay(year, month, day);
        GetHourMinuteSecond(hour, minute, second);
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
            GetGame().RPCSingleParam(player, DMT_RPC.TIME, payload, true, player.GetIdentity());
            sent++;
        }

        bool noPlayers = sent == 0;
        if (noPlayers != s_NoPlayers)
        {
            s_NoPlayers = noPlayers;
            Log(78, "PLAYERS none=" + noPlayers.ToString());
        }
        if (s_Elapsed == 1 || s_Elapsed % 30 == 0)
        {
            Log(83, "SYNC n=" + s_Elapsed.ToString() + " t=" + Pad2(hour) + ":" + Pad2(minute) + ":" + Pad2(second) + " p=" + sent.ToString());
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
};

// DMT Minimal UI — server. Keep log tags + source line numbers short.

static const int DMT_RPC_TIME = 19740621;

class DMT_Server
{
    static int s_Elapsed;
    static int s_LastSecond;
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
            Log("dmt_srv", 19, "START already=1");
            return;
        }
        s_Started = true;
        s_Elapsed = 0;
        s_LastSecond = -1;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 1000, true);
        Log("dmt_srv", 26, "START queue=1000ms");
    }

    static void Stop()
    {
        if (!s_Started) return;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
        s_Started = false;
        Log("dmt_srv", 34, "STOP");
    }

    static void Tick()
    {
        if (!GetGame() || !GetGame().GetWorld())
        {
            Log("dmt_srv", 40, "TICK world=0");
            return;
        }

        int year, month, day, hour, minute;
        GetGame().GetWorld().GetDate(year, month, day, hour, minute);

        // The game world API has minute precision. Seconds are an elapsed-server
        // clock anchored to mission start, not a claim about accelerated game time.
        s_Elapsed++;
        int second = s_Elapsed % 60;
        int shownMinute = minute + (s_Elapsed / 60);
        int shownHour = hour + (shownMinute / 60);
        shownMinute = shownMinute % 60;
        shownHour = shownHour % 24;

        if (second == s_LastSecond) return;
        s_LastSecond = second;

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        Param7<int, int, int, int, int, int, int> payload = new Param7<int, int, int, int, int, int, int>(
            year, month, day, shownHour, shownMinute, second, s_Elapsed
        );

        int sent = 0;
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity()) continue;
            GetGame().RPCSingleParam(player, DMT_RPC_TIME, payload, true, player.GetIdentity());
            sent++;
        }

        // One compact line per second, no player-loop spam.
        Log("dmt_srv", 72, "SYNC n=" + s_Elapsed.ToString() + " t=" + Pad2(shownHour) + ":" + Pad2(shownMinute) + ":" + Pad2(second) + " p=" + sent.ToString());
    }
};

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        DMT_Server.Log("dmt_srv", 82, "MissionServer.OnInit");
        DMT_Server.Start();
        DMT_Server.Log("dmt_srv", 84, "MissionServer.ready");
    }

    override void OnMissionFinish()
    {
        DMT_Server.Log("dmt_srv", 90, "MissionServer.OnMissionFinish");
        DMT_Server.Stop();
        super.OnMissionFinish();
    }
};

modded class DayZGame
{
    override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, target, rpc_type, ctx);
        // Server has no UI listener. Client receive logic is in dmt_cln.c.
    }
};

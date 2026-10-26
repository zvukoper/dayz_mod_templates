// DayZ Minimal Interface test interface — server. Send the server machine's current time once per second.

class DMT_RemoteProcedureCallId
{
    static const int SERVER_TIME_UPDATE = 19740621;
};

class DMT_Server
{
    static int s_ServerSequenceNumber;
    static bool s_NoConnectedPlayers;
    static bool s_ServerClockStarted;

    static string FormatTwoDigits(int value)
    {
        if (value < 10)
        {
            return "0" + value.ToString();
        }
        return value.ToString();
    }

    static void Log(int sourceLine, string message)
    {
        Print("[DayZ Minimal Interface][dmt_srv:" + sourceLine.ToString() + "] " + message);
    }

    static void Start()
    {
        if (s_ServerClockStarted)
        {
            Log(32, "Server clock already started");
            return;
        }
        if (!GetGame() || !GetGame().IsServer())
        {
            Log(37, "Server role unavailable; server clock was not started");
            return;
        }

        s_ServerClockStarted = true;
        s_ServerSequenceNumber = 0;
        s_NoConnectedPlayers = false;

        Log(45, "Server time update scheduled; interval milliseconds=1000");
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 1000, true);
    }

    static void Tick()
    {
        if (!GetGame())
        {
            Log(53, "Server game instance is unavailable during time update");
            return;
        }

        int year, month, day, hour, minute, second;
        GetYearMonthDay(year, month, day);
        GetHourMinuteSecond(hour, minute, second);
        s_ServerSequenceNumber++;

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        int sent = 0;
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity())
            {
                continue;
            }

            Param7<int, int, int, int, int, int, int> payload = new Param7<int, int, int, int, int, int, int>(
                year, month, day, hour, minute, second, s_ServerSequenceNumber
            );
            GetGame().RPCSingleParam(player, DMT_RemoteProcedureCallId.SERVER_TIME_UPDATE, payload, true, player.GetIdentity());
            sent++;
        }

        bool noPlayers = sent == 0;
        if (noPlayers != s_NoConnectedPlayers)
        {
            s_NoConnectedPlayers = noPlayers;
            if (noPlayers)
            {
                Log(86, "No connected players were found");
            }
            else
            {
                Log(90, "Connected players are available");
            }
        }

        if (s_ServerSequenceNumber == 1 || s_ServerSequenceNumber % 30 == 0)
        {
            Log(96, "Server time sent: sequence=" + s_ServerSequenceNumber.ToString() + " time=" + FormatTwoDigits(hour) + ":" + FormatTwoDigits(minute) + ":" + FormatTwoDigits(second) + " connected player count=" + sent.ToString());
        }
    }
};

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        DMT_Server.Log(106, "Server mission initialization started");
        DMT_Server.Start();
        DMT_Server.Log(108, "Server mission initialization completed");
    }
};
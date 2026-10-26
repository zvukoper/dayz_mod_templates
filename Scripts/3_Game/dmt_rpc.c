// DayZ Minimal Interface test interface — shared network handling.
// DayZGame is defined in the Game script module, so its network hook must live here.
class DMT_RemoteProcedureCallId
{
    static const int SERVER_TIME_UPDATE = 19740621;
};

class DMT_NetworkTimeState
{
    static int s_Year;
    static int s_Month;
    static int s_Day;
    static int s_Hour;
    static int s_Minute;
    static int s_Second;
    static int s_Sequence;
    static int s_ReceivedUpdateCount;
    static int s_ReadErrorCount;

    static void ReadServerTime(ParamsReadContext ctx)
    {
        Param7<int, int, int, int, int, int, int> payload = new Param7<int, int, int, int, int, int, int>(0, 0, 0, 0, 0, 0, 0);
        if (!ctx.Read(payload))
        {
            s_ReadErrorCount++;
            return;
        }

        s_Year = payload.param1;
        s_Month = payload.param2;
        s_Day = payload.param3;
        s_Hour = payload.param4;
        s_Minute = payload.param5;
        s_Second = payload.param6;
        s_Sequence = payload.param7;
        s_ReceivedUpdateCount++;
    }

    static void Reset()
    {
        s_Year = 0;
        s_Month = 0;
        s_Day = 0;
        s_Hour = 0;
        s_Minute = 0;
        s_Second = 0;
        s_Sequence = 0;
        s_ReceivedUpdateCount = 0;
        s_ReadErrorCount = 0;
    }
};

modded class DayZGame
{
    override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
    {
        // Handle this mod's time update before the engine forwards targeted RPCs to their objects.
        if (GetGame() && !GetGame().IsServer() && rpc_type == DMT_RemoteProcedureCallId.SERVER_TIME_UPDATE)
        {
            DMT_NetworkTimeState.ReadServerTime(ctx);
            return;
        }

        // Preserve the standard event dispatcher for all other network messages.
        super.OnRPC(sender, target, rpc_type, ctx);
    }
};
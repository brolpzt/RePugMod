#include "precompiled.h"

CStats gStats;

void CStats::ClientGetIntoGame(CBasePlayer* Player)
{
	if (gPugMod.GetState() == PUG_STATE_FIRST_HALF || gPugMod.GetState() == PUG_STATE_SECOND_HALF || gPugMod.GetState() == PUG_STATE_OVERTIME)
	{
		int EntityIndex = Player->entindex();

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			this->m_Stats[EntityIndex][i][PUG_STATS_HIT] = 0;
			this->m_Stats[EntityIndex][i][PUG_STATS_DMG] = 0;
		}
	}
}

void CStats::RoundStart()
{
	if (gPugMod.GetState() == PUG_STATE_FIRST_HALF || gPugMod.GetState() == PUG_STATE_SECOND_HALF || gPugMod.GetState() == PUG_STATE_OVERTIME)
	{
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			for (int j = 1; j <= gpGlobals->maxClients; ++j)
			{
				this->m_Stats[i][j][PUG_STATS_HIT] = 0;
				this->m_Stats[i][j][PUG_STATS_DMG] = 0;
			}
		}
	}
}

void CStats::RoundEnd(int winStatus, ScenarioEventEndRound event, float tmDelay)
{
	if (winStatus != WINSTATUS_NONE)
	{
		if (gPugMod.GetState() == PUG_STATE_FIRST_HALF || gPugMod.GetState() == PUG_STATE_SECOND_HALF || gPugMod.GetState() == PUG_STATE_OVERTIME)
		{
			CBasePlayer* Players[32] = {NULL};

			int Num = gPlayer.GetList(Players);

			for (int i = 0; i < Num; i++)
			{
				if(!Players[i]->IsBot())
				{
					int PlayerIndex = Players[i]->entindex();
					
					for (int j = 0; j < Num; j++)
					{
						int TargetIndex = Players[j]->entindex();

						if (this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_HIT] || this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_HIT])
						{
							gUtil.ClientPrint
							(
								Players[i]->edict(),
								PRINT_CONSOLE,
								"(%d dmg / %d hits) to (%d dmg / %d hits) from %s (%d HP)",
								this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_DMG],
								this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_HIT],
								this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_DMG],
								this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_HIT],
								STRING(Players[j]->edict()->v.netname),
								Players[j]->IsAlive() ? (int)Players[j]->edict()->v.health : 0
							);
						}
					}
				}
			}
		}
	}
}

void CStats::CBasePlayer_TakeDamage(CBasePlayer* pthis, entvars_t* pevInflictor, entvars_t* pevAttacker, float & flDamage, int bitsDamageType)
{
	if (g_pGameRules)
	{
		if (gPugMod.GetState() == PUG_STATE_FIRST_HALF || gPugMod.GetState() == PUG_STATE_SECOND_HALF || gPugMod.GetState() == PUG_STATE_OVERTIME)
		{
			auto pEdict = ENT(pevAttacker);

			if (pEdict)
			{
				auto pAttacker = (CBaseEntity*)GET_PRIVATE(pEdict);

				if (pAttacker)
				{
					int AttackerIndex = pAttacker->entindex();

					if (AttackerIndex >= 1 && AttackerIndex <= gpGlobals->maxClients)
					{
						int VictimIndex = pthis->entindex();

						if (AttackerIndex != VictimIndex)
						{
							if (CSGameRules()->FPlayerCanTakeDamage(pthis, pAttacker))
							{
								this->m_Stats[AttackerIndex][VictimIndex][PUG_STATS_HIT]++;

								this->m_Stats[AttackerIndex][VictimIndex][PUG_STATS_DMG] += (int)flDamage;
							}
						}
					}
				}
			}
		}
	}
}

bool CStats::HP(CBasePlayer* Player)
{
	if (g_pGameRules)
	{
		if (gPugMod.GetState() == PUG_STATE_FIRST_HALF || gPugMod.GetState() == PUG_STATE_SECOND_HALF || gPugMod.GetState() == PUG_STATE_OVERTIME)
		{
			if (!Player->IsAlive() || CSGameRules()->m_bRoundTerminating || CSGameRules()->IsFreezePeriod())
			{	
				if (Player->m_iTeam == TERRORIST || Player->m_iTeam == CT)
				{
					int StatsCount = 0;

					CBasePlayer* Players[32] = { NULL };

					int Num = gPlayer.GetList(Players);

					for (int i = 0; i < Num; i++)
					{
						if (Player->m_iTeam != Players[i]->m_iTeam)
						{
							if (Players[i]->IsAlive())
							{
								StatsCount++;

								gUtil.SayText
								(
									Player->edict(), 
									Players[i]->entindex(), 
									"\3%s\1 with %d HP (%d AP)", 
									STRING(Players[i]->edict()->v.netname),
									(int)Players[i]->edict()->v.health,
									(int)Players[i]->edict()->v.armorvalue
								);
							}
						}
					}

					if (!StatsCount)
					{
						gUtil.SayText(Player->edict(), PRINT_TEAM_DEFAULT, "No one is alive.");
					}

					return true;
				}
			}
		}
	}

	gUtil.SayText(Player->edict(), PRINT_TEAM_DEFAULT, "Unable to use this command now.");

	return false;
}

bool CStats::Damage(CBasePlayer * Player)
{
	if (g_pGameRules)
	{
		if (gPugMod.GetState() == PUG_STATE_FIRST_HALF || gPugMod.GetState() == PUG_STATE_SECOND_HALF || gPugMod.GetState() == PUG_STATE_OVERTIME)
		{
			if (!Player->IsAlive() || CSGameRules()->m_bRoundTerminating || CSGameRules()->IsFreezePeriod())
			{
				if (Player->m_iTeam == TERRORIST || Player->m_iTeam == CT)
				{
					int StatsCount = 0;

					int PlayerIndex = Player->entindex();

					CBasePlayer* Players[32] = { NULL };

					int Num = gPlayer.GetList(Players);

					for (int i = 0; i < Num; i++)
					{
						int TargetIndex = Players[i]->entindex();

						if (this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_HIT])
						{
							StatsCount++;

							gUtil.SayText
							(
								Player->edict(),
								TargetIndex,
								"Hit \3%s\1 %d time(s) (Damage %d)",
								STRING(Players[i]->edict()->v.netname),
								this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_HIT],
								this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_DMG]
							);
						}
					}

					if (!StatsCount)
					{
						gUtil.SayText(Player->edict(), PRINT_TEAM_DEFAULT, "You do not hit anyone in this round.");
					}

					return true;
				}
			}
		}
	}

	gUtil.SayText(Player->edict(), PRINT_TEAM_DEFAULT, "Unable to use this command now.");

	return false;
}

bool CStats::Received(CBasePlayer * Player)
{
	if (g_pGameRules)
	{
		if (gPugMod.GetState() == PUG_STATE_FIRST_HALF || gPugMod.GetState() == PUG_STATE_SECOND_HALF || gPugMod.GetState() == PUG_STATE_OVERTIME)
		{
			if (!Player->IsAlive() || CSGameRules()->m_bRoundTerminating || CSGameRules()->IsFreezePeriod())
			{
				if (Player->m_iTeam == TERRORIST || Player->m_iTeam == CT)
				{
					int StatsCount = 0;

					int PlayerIndex = Player->entindex();

					CBasePlayer* Players[32] = { NULL };

					int Num = gPlayer.GetList(Players);

					for (int i = 0; i < Num; i++)
					{
						int TargetIndex = Players[i]->entindex();

						if (this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_HIT])
						{
							StatsCount++;

							gUtil.SayText
							(
								Player->edict(),
								TargetIndex,
								"Hit by \3%s\1 %d time(s) (Damage %d)",
								STRING(Players[i]->edict()->v.netname),
								this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_HIT],
								this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_DMG]
							);
						}
					}

					if (!StatsCount)
					{
						gUtil.SayText(Player->edict(), PRINT_TEAM_DEFAULT, "You were not reached in this round.");
					}

					return true;
				}
			}
		}
	}

	gUtil.SayText(Player->edict(), PRINT_TEAM_DEFAULT, "Unable to use this command now.");

	return false;
}

bool CStats::Summary(CBasePlayer* Player)
{
	if (g_pGameRules)
	{
		if (gPugMod.GetState() == PUG_STATE_FIRST_HALF || gPugMod.GetState() == PUG_STATE_SECOND_HALF || gPugMod.GetState() == PUG_STATE_OVERTIME)
		{
			if (!Player->IsAlive() || CSGameRules()->m_bRoundTerminating || CSGameRules()->IsFreezePeriod())
			{
				if (Player->m_iTeam == TERRORIST || Player->m_iTeam == CT)
				{
					int StatsCount = 0;

					int PlayerIndex = Player->entindex();

					CBasePlayer* Players[32] = { NULL };

					int Num = gPlayer.GetList(Players);

					for (int i = 0; i < Num; i++)
					{
						int TargetIndex = Players[i]->entindex();

						if (this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_HIT] || this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_HIT])
						{
							StatsCount++;

							gUtil.SayText
							(
								Player->edict(),
								TargetIndex,
								"(%d dmg / %d hits) to (%d dmg / %d hits) from \3%s\1 (%d HP)",
								this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_DMG],
								this->m_Stats[PlayerIndex][TargetIndex][PUG_STATS_HIT],
								this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_DMG],
								this->m_Stats[TargetIndex][PlayerIndex][PUG_STATS_HIT],
								STRING(Players[i]->edict()->v.netname),
								Players[i]->IsAlive() ? (int)Players[i]->edict()->v.health : 0
							);
						}
					}

					if (!StatsCount)
					{
						gUtil.SayText(Player->edict(), PRINT_TEAM_DEFAULT, "No stats in this round.");
					}

					return true;
				}
			}
		}
	}

	gUtil.SayText(Player->edict(), PRINT_TEAM_DEFAULT, "Unable to use this command now.");

	return false;
}
/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "violet_hold.h"

enum Spells
{
    SPELL_ARCANE_VACUUM                         = 58694,
    SPELL_BLIZZARD                              = 58693,
    SPELL_MANA_DESTRUCTION                      = 59374,
    SPELL_TAIL_SWEEP                            = 58690,
    SPELL_UNCONTROLLABLE_ENERGY                 = 58688,
    SPELL_TRANSFORM                             = 58668
};

enum Yells
{
    SAY_AGGRO                                   = 0,
    SAY_SLAY                                    = 1,
    SAY_DEATH                                   = 2,
    SAY_SPAWN                                   = 3,
    SAY_DISRUPTION                              = 4,
    SAY_BREATH_ATTACK                           = 5,
    SAY_SPECIAL_ATTACK                          = 6
};

class boss_cyanigosa : public CreatureScript
{
public:
    boss_cyanigosa() : CreatureScript("boss_cyanigosa") { }

    struct boss_cyanigosaAI : public BossAI
    {
        boss_cyanigosaAI(Creature* creature) : BossAI(creature, DATA_CYANIGOSA)
        {
            Initialize();
        }

        void Initialize()
        {
            uiArcaneVacuumTimer = 10000;
            uiBlizzardTimer = 15000;
            uiManaDestructionTimer = 30000;
            uiTailSweepTimer = 20000;
            uiUncontrollableEnergyTimer = 25000;
        }

        uint32 uiArcaneVacuumTimer;
        uint32 uiBlizzardTimer;
        uint32 uiManaDestructionTimer;
        uint32 uiTailSweepTimer;
        uint32 uiUncontrollableEnergyTimer;

        void Reset() override
        {
            Initialize();
            BossAI::Reset();
        }

        void EnterCombat(Unit* who) override
        {
            BossAI::EnterCombat(who);
            Talk(SAY_AGGRO);
        }

        void MoveInLineOfSight(Unit* /*who*/) override { }

        void UpdateAI(uint32 diff) override
        {
            if (instance->GetData(DATA_REMOVE_NPC) == 1)
            {
                me->DespawnOrUnsummon();
                instance->SetData(DATA_REMOVE_NPC, 0);
            }

            if (!UpdateVictim())
                return;

            if (uiArcaneVacuumTimer <= diff)
            {
                DoCastAOE(SPELL_ARCANE_VACUUM);
                uiArcaneVacuumTimer = 10000;
            } else uiArcaneVacuumTimer -= diff;

            if (uiBlizzardTimer <= diff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    DoCast(target, SPELL_BLIZZARD);
                uiBlizzardTimer = 15000;
            } else uiBlizzardTimer -= diff;

            if (uiTailSweepTimer <= diff)
            {
                DoCastVictim(SPELL_TAIL_SWEEP);
                uiTailSweepTimer = 20000;
            } else uiTailSweepTimer -= diff;

            if (uiUncontrollableEnergyTimer <= diff)
            {
                DoCastVictim(SPELL_UNCONTROLLABLE_ENERGY);
                uiUncontrollableEnergyTimer = 25000;
            } else uiUncontrollableEnergyTimer -= diff;

            if (IsHeroic())
            {
                if (uiManaDestructionTimer <= diff)
                {
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        DoCast(target, SPELL_MANA_DESTRUCTION);
                    uiManaDestructionTimer = 30000;
                } else uiManaDestructionTimer -= diff;
            }

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* killer) override
        {
            BossAI::JustDied(killer);
            Talk(SAY_DEATH);
        }

        void KilledUnit(Unit* victim) override
        {
            if (victim->GetTypeId() == TYPEID_PLAYER)
                Talk(SAY_SLAY);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetInstanceAI<boss_cyanigosaAI>(creature);
    }
};

class achievement_defenseless : public AchievementCriteriaScript
{
    public:
        achievement_defenseless() : AchievementCriteriaScript("achievement_defenseless")
        {
        }

        bool OnCheck(Player* /*player*/, Unit* target) override
        {
            if (!target)
                return false;

            InstanceScript* instance = target->GetInstanceScript();
            if (!instance)
                return false;

            if (!instance->GetData(DATA_DEFENSELESS))
                return false;

            return true;
        }
};

void AddSC_boss_cyanigosa()
{
    new boss_cyanigosa();
    new achievement_defenseless();
}

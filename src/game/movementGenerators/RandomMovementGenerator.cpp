/*
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Creature.h"
#include "MapManager.h"
#include "RandomMovementGenerator.h"
#include "Map.h"
#include "Util.h"
#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

template<>
RandomMovementGenerator<Creature>::RandomMovementGenerator(const Creature & creature)
{
    float respX, respY, respZ, respO, wander_distance;
    creature.GetRespawnCoord(respX, respY, respZ, &respO, &wander_distance);
    i_nextMoveTime = ShortTimeTracker(0);
    i_x = respX;
    i_y = respY;
    i_z = respZ;
    i_radius = wander_distance;
    // TODO - add support for flying mobs using some distance
    i_verticalZ = 0.0f;
}

template<>
void RandomMovementGenerator<Creature>::_setRandomLocation(Creature &creature)
{
    const float angle = rand_norm_f() * (M_PI_F*2.0f);
    const float range = rand_norm_f() * i_radius;

    float destX,destY,destZ;
    creature.GetNearPoint(&creature, destX, destY, destZ, creature.GetObjectBoundingRadius(), range, angle);
    creature.UpdateAllowedPositionZ(destX, destY, destZ);

    float dx = i_x - destX;
    float dy = i_y - destY;
    // TODO: Limitation creatutre travel range.
    if (sqrt((dx*dx) + (dy*dy)) > i_radius)
    {
        destX = i_x;
        destY = i_y;
        destZ = i_z;
    }
    else if (creature.IsLevitating())
        destZ = i_z;

    creature.addUnitState(UNIT_STAT_ROAMING_MOVE);

    Movement::MoveSplineInit<Unit*> init(creature);
    init.MoveTo(destX, destY, destZ, true);

    if (!creature.IsLevitating() && !creature.IsSwimming())
        init.SetWalk(true);

    init.Launch();

    if (creature.CanFly())
        i_nextMoveTime.Reset(0);
    else
    {
        if (roll_chance_i(MOVEMENT_RANDOM_MMGEN_CHANCE_NO_BREAK))
            i_nextMoveTime.Reset(50);
        else
            i_nextMoveTime.Reset(urand(3000, 10000));       // keep a short wait time
    }
}

template<>
void RandomMovementGenerator<Creature>::Initialize(Creature &creature)
{
    if (!creature.isAlive())
        return;

    creature.addUnitState(UNIT_STAT_ROAMING|UNIT_STAT_ROAMING_MOVE);
    _setRandomLocation(creature);
}

template<>
void RandomMovementGenerator<Creature>::Reset(Creature &creature)
{
    Initialize(creature);
}

template<>
void RandomMovementGenerator<Creature>::Interrupt(Creature &creature)
{
    creature.InterruptMoving();
    creature.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    creature.SetWalk(!creature.hasUnitState(UNIT_STAT_RUNNING_STATE), false);
}

template<>
void RandomMovementGenerator<Creature>::Finalize(Creature &creature)
{
    creature.clearUnitState(UNIT_STAT_ROAMING | UNIT_STAT_ROAMING_MOVE);
    creature.SetWalk(!creature.hasUnitState(UNIT_STAT_RUNNING_STATE), false);
}

template<>
bool RandomMovementGenerator<Creature>::Update(Creature &creature, const uint32 &diff)
{
    if (creature.hasUnitState(UNIT_STAT_NOT_MOVE))
    {
        i_nextMoveTime.Reset(0);  // Expire the timer
        creature.clearUnitState(UNIT_STAT_ROAMING_MOVE);
        return true;
    }

    if (creature.movespline->Finalized())
    {
        i_nextMoveTime.Update(diff);
        if (i_nextMoveTime.Passed())
            _setRandomLocation(creature);
    }
    return true;
}
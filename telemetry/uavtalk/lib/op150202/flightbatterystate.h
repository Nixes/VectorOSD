/*
 * This file is part of MultiOSD <https://github.com/UncleRus/MultiOSD>
 *
 * MultiOSD is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UAVTALK_OP150202_FLIGHTBATTERYSTATE_H
#define UAVTALK_OP150202_FLIGHTBATTERYSTATE_H

#include "../common.h"

UT_NAMESPACE_OPEN

namespace op150202
{

#define UAVTALK_OP150202_FLIGHTBATTERYSTATE_OBJID 0x26962352

struct FlightBatteryState
{
    float Voltage;
    float Current;
    float BoardSupplyVoltage;
    float PeakCurrent;
    float AvgCurrent;
    float ConsumedEnergy;
    float EstimatedFlightTime;
    uint8_t NbCells;
    bool NbCellsAutodetected;
};

}  // namespace op150202

UT_NAMESPACE_CLOSE

#endif // UAVTALK_OP150202_FLIGHTBATTERYSTATE_H

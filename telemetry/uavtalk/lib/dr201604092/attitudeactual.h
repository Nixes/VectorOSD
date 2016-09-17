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
#ifndef UAVTALK_DR201604092_ATTITUDEACTUAL_H
#define UAVTALK_DR201604092_ATTITUDEACTUAL_H

#include "../common.h"

UT_NAMESPACE_OPEN

namespace dr201604092
{

#define UAVTALK_DR201604092_ATTITUDEACTUAL_OBJID 0x33DAD5E6

struct AttitudeActual
{
	float q1;
	float q2;
	float q3;
	float q4;
	float Roll;
	float Pitch;
	float Yaw;
};

}  // namespace dr201604092

UT_NAMESPACE_CLOSE

#endif // UAVTALK_DR201604092_ATTITUDEACTUAL_H

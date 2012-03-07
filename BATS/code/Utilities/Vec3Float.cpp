/**
* @file
* @author Jonathan Udd <jonis.x@gmail.com>
* @author Matteus Magnusson <senth.wallace@gmail.com>
* @version 1.0
* Copyright (©) A-Team.
*
* @section LICENSE
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details at
* http://www.gnu.org/copyleft/gpl.html
*/

#include "Vec3Float.h"
#include <cmath>
#include <climits>

using namespace utilities;

void Vec3Float::rotateAroundY(float radian)
{
	float tempX = cos(radian) * x - sin(radian) * z;
	z = sin(radian) * x + cos(radian) * z;
	x = tempX;
}

void Vec3Float::normalize()
{
	float length = this->length();
	if (length != 0.0f)
	{
		x /= length;
		y /= length;
		z /= length;
	}
}

float Vec3Float::dotProduct(float x, float y, float z)
{
	return this->x*x + this->y*y + this->z*z;
}

float Vec3Float::dotProduct(const Vec3Float& vec3) const
{
	return x*vec3.x + y*vec3.y + z*vec3.z;
}

float Vec3Float::length() const
{
	return sqrtf(x*x + y*y + z*z);
}

bool Vec3Float::longerThan(float length, bool useY) const
{
	if(useY)
	{
		return x*x + y*y + z*z > length * length;
	}
	else
	{
		return  x*x + z*z > length * length;
	}
}

bool Vec3Float::longerThan(const Vec3Float& vec3, bool useY) const
{
	if(useY)
	{
		return (x*x + y*y + z*z) > (vec3.x*vec3.x + vec3.y*vec3.y + vec3.z*vec3.z);
	}
	else
	{
		return  x*x + z*z > (vec3.x*vec3.x + vec3.z*vec3.z);
	}
}

Vec3Float Vec3Float::operator-(const Vec3Float& vec3) const
{
	Vec3Float diffVector = *this;
	diffVector.x -= vec3.x;
	diffVector.y -= vec3.y;
	diffVector.z -= vec3.z;
	return diffVector;
}

Vec3Float Vec3Float::operator +(const Vec3Float &vec3Float) const
{
	Vec3Float sumVec = *this;
	sumVec.x += vec3Float.x;
	sumVec.y += vec3Float.y;
	sumVec.z += vec3Float.z;
	return sumVec;
}

Vec3Float Vec3Float::operator *(float fValue) const
{
	Vec3Float productVec = *this;
	productVec.x *= fValue;
	productVec.y *= fValue;
	productVec.z *= fValue;
	return productVec;
}


std::istream& operator>>(std::istream &in, Vec3Float& vec3Float)
{
	in >> vec3Float.x >> vec3Float.y >> vec3Float.z;
	return in;
}
std::ostream& operator<<(std::ostream &out, const Vec3Float& vec3Float)
{
	out << vec3Float.x << " " << vec3Float.y << " " << vec3Float.z << " ";
	return out;
}

Vec3Float operator*(float fValue, const Vec3Float& vec3Float)
{
	Vec3Float productVec = vec3Float;
	productVec.x *= fValue;
	productVec.y *= fValue;
	productVec.z *= fValue;
	return productVec;
}

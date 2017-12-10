/*
  Parameters.h

  You can find the latest version of this code at :
  https://github.com/LoupHC/GreenhouseLib

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"

#ifndef Parameters_h
#define Parameters_h
//Each greenhouse parameter have 3 associated values, a minimumimum value, a maximumimum value and an actual value
class floatParameter
{
  public:
    floatParameter();
    ~floatParameter();
    void setValue(float value);
    void setLimits(float minimum, float maximum);
    void minimum(float minimum);
    void maximum(float maximum);
    float value();

  private:
    float _minimum;
    float _maximum;
    float _value;
};

class uShortParameter
{
  public:
    uShortParameter();
    ~uShortParameter();
    void setValue(unsigned short value);
    void setLimits(unsigned short minimum, unsigned short maximum);
    void minimum(unsigned short minimum);
    void maximum(unsigned short minimum);
    unsigned short value();

  private:
    unsigned short _minimum;
    unsigned short _maximum;
    unsigned short _value;
};

class shortParameter
{
  public:
    shortParameter();
    ~shortParameter();
    void setValue(short  value);
    void setLimits(short minimum, short maximum);
    void minimum(short  minimum);
    void maximum(short  maximum);
    short value();

  private:
    short _minimum;
    short _maximum;
    short _value;
};

class byteParameter
{
  public:
    byteParameter();
    ~byteParameter();
    void setValue(byte value);
    void setLimits(byte minimum, byte maximum);
    void minimum(byte minimum);
    void maximum(byte maximum);
    byte value();

  private:
    byte _minimum;
    byte _maximum;
    byte _value;
};


//Generic macros
byte negativeToByte(int data, byte mod);
int byteToNegative(int data, byte mod);
void convertDecimalToTime(int * heure, int * minut);

#endif

/*    
    Copyright (c) 2019 Patryk Cieslak. All rights reserved.

    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#version 330

out vec2 texcoord;

void main()
{
	int idx = gl_VertexID % 3;
	vec4 pos =  vec4((float(idx&1))*4.0-1.0, (float((idx>>1)&1))*4.0-1.0, 1.0, 1.0);
	gl_Position = pos;
	texcoord = pos.xy * 0.5 + 0.5;
}

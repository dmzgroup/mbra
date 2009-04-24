from math import pi, sin, log

# Python sample. How to ask Google for map tiles?

# This script is published under GPL (included below)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#



# Constants used for degree to radian conversion, and vice-versa.
DTOR = pi / 180.
RTOD = 180. / pi
TILE_SIZE = 256

class GoogleZoom(object):
   def locationCoord(self, lat, lon, zoom):
      if (abs(lat) > 85.0511287798066):
         raise "Invalid latitude"
      sin_phi = sin(float(lat) * DTOR)
      norm_x = lon / 180.
      norm_y = (0.5 * log((1 + sin_phi) / (1 - sin_phi))) / pi
      col = pow(2, zoom) * ((norm_x + 1) / 2);
      row = pow(2, zoom) * ((1 - norm_y) / 2);
      return (col, row, zoom);

gz = GoogleZoom ()
#col, row, zoom = gz.locationCoord (36.5973550921921, -121.876788139343, 12)
col, row, zoom = gz.locationCoord (52.5211, 13.4122, 10)
print "Google Tile URL: http://mt.google.com/mt?x=%d&y=%d&zoom=%d" % (col, row, 17-zoom)
print "X coordinate within the tile =", (col - long(col)) * TILE_SIZE
print "Y coordinate within the tile =", (row - long(row)) * 256

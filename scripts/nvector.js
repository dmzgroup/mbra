var util = require('dmz/types/util')
  , createError = util.createError
  , Epsilon = util.Epsilon
  , Vector
  ;

Vector = function (n) {
   var c1
     ;

   if ((n === 0) || (n === undefined)) {
      throw createError("Invalid nvector initialization value:", n);
   }

   this.length = n;
   this.v = new Array(n);
   for (c1 = 0; c1 < n; c1 += 1) {
      this.v[c1] = 0;
   }
}

exports.isTypeOf = function (value) {

   return Vector.prototype.isPrototypeOf(value) ? value : undefined;
};


exports.create = function (n, array) {

   var result = new Vector(n);
   if (array) {
      result = result.fromArray (array);
   }

   return result;
};


Vector.prototype.create = exports.create;

Vector.prototype.copy = function () {

   var c1
     , result = new Vector(this.length);
     ;

   result.v = this.v.slice(0);
   return result;
};


Vector.prototype.toString = function () {

   var string = "["
     , count
     ;

   for (count = 0; count < (this.length - 1); count += 1) {
      string += this.v[count] + ', ';
   }


   return string + this.v[this.length - 1] + ']';
};

Vector.prototype.toArray = function () {

   return this.v;
};

Vector.prototype.fromArray = function (array) {

   this.v = array.slice(0);
   this.length = array.length;
};

Vector.prototype.setElement = function (ele, val) {

   if ((ele < this.length) && (ele >= 0)) {
      this.v[ele] = val;
   }
   else {
      throw createError("Invalid vector index:", ele);
   }
};

Vector.prototype.magnitude = function () {

   var result = Math.sqrt(this.dot(this));
   return result > Epsilon ? result : 0.0;
};


Vector.prototype.normalize = function () {

   var mag = this.magnitude(),
      div = 0.0;

   if (mag > Epsilon) {

      div = 1 / mag;
   }

   return this.multiply (div);
};


Vector.prototype.add = function (vec) {

   var result
     , count
     ;

   if (Vector.prototype.isPrototypeOf(vec) && (this.length === vec.length)) {

      result = new Vector (this.length);
      for (count = 0; count < this.length; count += 1) {
         result.setElement (count, this.v[count] + vec.v[count]);
      }
   }
   else {

      throw createError("Invalid parameter for Vector.add() " + JSON.stringify(vec));
   }

   return result;
};


Vector.prototype.subtract = function (vec) {

   var result
     , count
     ;

   if (Vector.prototype.isPrototypeOf(vec) && (this.length === vec.length)) {

      result = new Vector (this.length);
      for (count = 0; count < this.length; count += 1) {
         result.setElement (count, this.v[count] - vec.v[count]);
      }
   }
   else {

      throw createError("Invalid parameter for Vector.subtract() " + JSON.stringify(vec));
   }

   return result;
};


Vector.prototype.multiply = function (k) {

   var result = new Vector (this.length)
     , count
     ;

   for (count = 0; count < this.length; count += 1) {
      result.setElement(count, this.v[count] * k);
   }

   return result;
};


Vector.prototype.multiplyConst = Vector.prototype.multiply;


Vector.prototype.dot = function (vec) {

   var result = 0
     , count
     ;

   if (Vector.prototype.isPrototypeOf(vec) && (this.length === vec.length)) {

      for (count = 0; count < this.length; count += 1) {
         result += this.v[count] * vec.v[count];
      }
   }
   else {

      throw createError("Invalid parameter for Vector.dot() " + JSON.stringify(vec));
   }

   return result;
};

Vector.prototype.isZero = function () {

   return util.isZero(this.magnitude());
};

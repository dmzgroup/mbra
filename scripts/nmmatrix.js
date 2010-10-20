var util = require ('dmz/types/util')
,   Epsilon = util.Epsilon
,   createError = util.createError
,   vector = require('nvector')
,   Matrix
;

Matrix = function (cols, rows) {
   var c1
     , c2
     ;

   if ((rows === 0) || (cols === 0) || (rows === undefined) || (cols === undefined)) {
      throw createError("Invalid nmmatrix initialization values:", rows, cols);
   }
   this.rows = rows;
   this.cols = cols;
   this.m = [];
   for (c1 = 0; c1 < cols; c1 += 1) {
      this.m[c1] = [];
      for (c2 = 0; c2 < rows; c2 += 1) {
         this.m[c1][c2] = 0;
      }
   }
}

exports.isTypeOf = function (value) {

   return Matrix.prototype.isPrototypeOf(value) ? value : undefined;
};


exports.create = function (cols, rows) {

   var result;

   if (!rows) {
      result = new Matrix(cols, cols);
   }
   if (!rows && !cols) {
      throw createError("Invalid matrix creation values:", rows, cols);
   }

   return result;
};

Matrix.prototype.create = exports.create;

Matrix.prototype.setElement = function (col, row, val) {
   this.m[col][row] = val;
};

Matrix.prototype.copy = function () {

   var result = new Matrix(this.cols, this.rows)
     , c1
     , c2
     ;

   for (c1 = 0; c1 < this.cols; c1 += 1) {
      for (c2 = 0; c2 < this.rows; c2 += 1) {
         result.m[c1][c2] = this.m[c1][c2];
      }
   }

   return result;
};

Matrix.prototype.transpose = function () {

   var result = new Matrix(this.rows, this.cols)
     , c1
     , c2
     ;

   for (c1 = 0; c1 < this.cols; c1 += 1) {
      for (c2 = 0; c2 < this.rows; c2 += 1) {
         result.m[c2][c1] = this.m[c1][c2];
      }
   }

   return result;
};

Matrix.prototype.toString = function () {

   var string = "\n["
     , c1
     , c2
     , length = 0
     , tL
     ;

   for (c1 = 0; c1 < this.cols; c1 += 1) {
      for (c2 = 0; c2 < this.rows; c2 += 1) {
         tL = this.m[c1][c2].toString().length;
         if (tL > length) {
            length = tL;
         }
      }
   }

   for (c2 = 0; c2 < this.rows; c2 += 1) {
      for (c1 = 0; c1 < this.cols; c1 += 1) {
         string += this.m[c1][c2] + ', ';
         tL = this.m[c1][c2].toString().length;
         while (length - tL) {
            string += ' ';
            tL += 1;
         }
      }
      string += "\n ";
   }

   return string + ']';
};

Matrix.prototype.toArray = function () {

   var count
     , result = []
     ;

   for (count = 0; count < this.rows; count += 1) {
      result = result.concat(this.m[count]);
   }

   return result;
};

Matrix.prototype.multiply = function () {

}

Matrix.prototype.diagonal = function () {
   var vec = vector.create(this.rows)
     , count
     ;

   for (count = 0; count < this.rows; count += 1) {
      vec.v[count] = this.m[count][count];
   }

   return vec;
}

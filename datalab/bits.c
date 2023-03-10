/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * xxxx_lyx_0208 
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  /* @brief take advantage of knowledge of Boolean algebra. */
  return ~(~x | ~y);
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  /* @brief x >> 8*n, x_8...x_0 is the answer. */
  int mask = 0xFF;
  int shiftValue = (n << 3);
  int extractByte = (x >> shiftValue);
  return mask&extractByte;
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  /* @brief
   * save the signal bit, clear it ans then execute logical shift. at the last add signal bit.
   * step 1, save the signal bit. 
   * step 2, generate mask: TMax(0x0111...) using Tmin - 1
   * step 3, to clear signal bit, ececute x & mask.
   * step 4, execute logical shift, and then add signal bit in the correct position.
   */
  int signalBit = (x >> 31) & 0x1;
  int mask = ~0 + (1 << 31);
  int num = x & mask;
  int shiftedNum = num >> n;
  int adder = signalBit << (31 + (~n + 1));

  return shiftedNum + adder;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  /* @brief: divide & conquer.
   * first, view the x as a array consists of num that only has 1bit.
   * we add adjacent num by 0x55 55 55 55 & x + (x >> 1) & 0x55 55 55 55.
   * so a 2bits num consists of two 1bit nums' 1 count.
   * Now, view the x as a array consists of num that only has 2bits.
   * we add adjacent num by 0x33 33 33 33 & x + (x >> 2) & 0x33 33 33 33.
   * so a 4bits num consists of two 2bits nums' 1 count.
   * ...
   * a 32bits num consists of two 16bits nums' 1 count.
   * */
  int cycleOneMask = 0x55 | (0x55 << 8);
  int cycleTwoMask = 0x33 | (0x33 << 8);
  int cycleFourMask= 0x0F | (0x0F << 8);
  int cycleEightMask=0xFF | (0xFF << 16);
  int cycleSixteenMask = 0xFF | (0xFF << 8);
  int ans = 1;
  cycleOneMask = cycleOneMask | (cycleOneMask << 16);
  cycleTwoMask = cycleTwoMask | (cycleTwoMask << 16);
  cycleFourMask = cycleFourMask | (cycleFourMask << 16);
  ans = (cycleOneMask & x) + ((x >> 1) & cycleOneMask);
  ans = (cycleTwoMask & ans) + ((ans >> 2) & cycleTwoMask);
  ans = (cycleFourMask & ans) + ((ans >> 4) & cycleFourMask);
  ans = (cycleEightMask & ans) + ((ans >> 8) & cycleEightMask);
  ans = (cycleSixteenMask & ans) + ((ans >> 16) & cycleSixteenMask);
  return ans;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  /* @brief: distinctive zero and non-zero 
   * If x is non-zero, the signBit of -x(~x + 1) | x is 1;
   * If x is zero, the signBit of -x(~x + 1) | x is 0;
   * To realize bang, let result ^ 0x01.
   * */
  int signal = (((~x+1) | x) >> 31) & 0x01;
  return signal ^ 0x01;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  /* @brief: the significant bit has negative weight in type int.*/
  return (1<<31);
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  /* @Question: why n == 32 is zero? It's wrong! */
  /* @Brief: sign extend 
   * If we want to extend a 3bits int to 32bits int, we will repeat its signBit to equalize them.
   * So, if a 32 bits int can be represented as a nbits int, 
   * the signBit will fill the gap between n and 32.
   * We use arithmetic right shift to repeat signBit. The result will be 0x00000000 or ~0x00000000;
   * And then, result + 1 will be zero or one. (result + 1) >> 1 will be zero.
   * If the result is not 0x0 or ~0x0, result + 1 != zero or one. 
   * */
  int num = x >> (n + ~0);
  return !((num + 1) >> 1) & !(n >> 5);
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  /* @brief
   * If x > 0, x / num will round toward zero, but (x+num-1)/num will round upper.
   * If x < 0, (x + num-1)/num round toward zero.
   * */
  int addValue = (x >> 31) & ((1 << n) + ~0);
  return (x + addValue) >> n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x+1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  /* @brief x != 0 and the signBit should be 0. */
  return !(x >> 31) & !!x;
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  /* @brief 
   * (-x) + y >= 0 
   * if no overflow, then sum >= 0 is true(the sign bit is zero), or false(the sign bit is 1);
   * else if positive overflow, then the result is true.
   * else if negative overflow, then the result is false.((-positive)+negative->negative overflow)
   * However, if x = TMin, then -x = Tmin, which will result in oppsite result. 
   * Fortunately, if x = TMin, then the result is always true.
   * */
  int ySign = y >> 31;
  int negateX = ~x + 1;
  int xSign = negateX >> 31; 
  int sum = y + negateX;
  int sumSign = (sum >> 31) & 0x01;
  int positiveOverflow = !xSign & !ySign & sumSign;
  int negativeOverflow = xSign & ySign & !sumSign;
  int ans = ((positiveOverflow | !sumSign) & !negativeOverflow) | !(x ^ (1 << 31));

  return ans;
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  /* @brief 
   * log_2(x) = a*2^4 + b*2^3 + c*2^2 + d*2^1 + e;  int_32 type x
   * so how to solve a-b?
   * If x >= 2^16, a can't be zero, or it's impossible. So if x >> 16 is not zero, a = 1.
   * and then, a only make result shift 16bits left, if (x >> 16) > 1, b&c&d&e is not zero.
   * If(x >> 16) >= 2^8, b can't be zero, or it is impossible. So if x >> 16 >> 8 is not zero, b = 1.
   * and then, a&b only make result shift 24bits left, if (x >> 24) > 1, c&d&e is not zero.
   * ...
   * If x < 2^16, a must be zero, due to a make result shift 16bits left. 
   * */
  int ans = 0; int a= 0, b = 0, c = 0, d = 0, e = 0;
  a = !!(x >> 16);  ans = a << 4;
  b = !!((x >> ans) >> 8);  ans = ans + (b << 3);
  c = !!((x >> ans) >> 4);  ans = ans + (c << 2);
  d = !!((x >> ans) >> 2);  ans = ans + (d << 1);
  e = !!((x >> ans) >> 1);  ans = ans + e;

  return ans;
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
  /* @brief
   * If uf is not special value, we only flip signBit.
   * else if uf is NaN, 0x7F8xxxxx, xxxxx is not zero, return NaN.
   * else if uf is infity, xxxxx is zero, we only flip signBit.
   */
  int flipSign = uf ^ (1 << 31);
  int leftShiftedFloat = uf << 1;
  int mask = 0xFF << 24;
  if((leftShiftedFloat&mask) ^ mask) return flipSign;
  else if(leftShiftedFloat & ~mask) return uf;
  else return flipSign;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  /* @brief 
   * float consists of signBit|eBits|fractionBits
   * We should calculate fraction and e by the absolute value of x.
   * However, the TMin has no relevant positive value, so return 0xCF000000 when x is TMin.
   * by view signBit and x, we can get the absolute value of x.
   * The float is normalized value exclusive zero, so return 0 when x is zero.
   * And, the base will be 1.fraction, so we should find the significant 1.
   * Then, we get the fraction section, it may has more bits then 23 bits. the rest bits need carry.
   * We round toward even num, and then offset the fraction to correct position.
   * remeber add eBias 127 to eBits. 
   * */
  int signBit = 0x00000000;
  int eBias = 0x7F;
  int eValue = 0;
  int num = 0;

  int fractionShiftValue = 0;
  int fraction = 0;
  int rest = 0;
  int restBeyond = 0;
  int restDelta = 0;
  int tmpFraction;

  
  if( x == 0 )  
    return 0;
  else if( x == 0x80000000 ) {
    return 0xCF000000;
  }
  if(x < 0){
    x = -x;
    signBit = 0x80000000;
  } 
    
  num = x;
  while( num ^ 0x01 ){
    eValue = eValue + 1;
    num = num >> 1;
  }
  /* The 1 << eValue is the significant 1 in x, so it equal to fraction = ~(allOnes << eValue) & x. */
  fraction = x - (1 << eValue);
  fractionShiftValue = eValue - 23;
  if(fractionShiftValue <= 0) {
    fraction = fraction << (-fractionShiftValue);
  }
  else {
    restDelta = (1 << fractionShiftValue);
    restBeyond = restDelta >> 1;
    rest = ~(0xFFFFFFFF << fractionShiftValue) & x;
    tmpFraction = fraction + restDelta;
    if(rest > restBeyond) {
      fraction = tmpFraction;
    }
    else if(rest == restBeyond) {
      if(fraction & restDelta) { 
        fraction = tmpFraction;
      }
    }
    fraction = fraction >> fractionShiftValue;
    eValue = eValue + (fraction >> 23);
  }
  eValue = (eValue + eBias) << 23;
  return signBit | eValue | (fraction & 0x007FFFFF);
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  /* @brief
   * The signBit will not change.
   * If eBits is zero, deNormalized value, we should change fraction section.
   * When fraction section will overflow, we should add 1 to eBits, expressing shift(twice).
   * If eBits is not zero, normalizd value, the value is signBit * 1.fraction * 2^eBits, so eBits+1
   * If eBits is allones, special value, return infinity when infinity, and NaN is the same.
   * */
  int fractionMask = 0x007FFFFF;  int eMask = 0x7F800000;
  int fractionMax = 0x00400000; int eDelta = 0x00800000;
  int eBits = uf & eMask;
  int fraction = uf & fractionMask;

  if(uf == 0) return 0;
  else if(uf == 0x80000000) return 0x80000000;

  if(eBits == 0){
    if(fraction & fractionMax){
      eBits = eBits + eDelta;
      fraction = (fraction - fractionMax) << 1;
    }
    else {
      fraction = fraction << 1;
    }
  }
  else if(eBits != 0x7F800000) {
    eBits = eBits + eDelta;
  } 

  return (uf & 0x80000000) | eBits | fraction;
}

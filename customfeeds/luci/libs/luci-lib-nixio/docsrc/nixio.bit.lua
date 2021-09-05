--- Bitfield operators and mainpulation functions.
-- Can be used as a drop-in replacement for bitlib.
module "nixio.bit"

--- Bitwise OR several numbers.
-- @class function
-- @name bor
-- @param oper1	First Operand
-- @param oper2 Second Operand
-- @param ...	More Operands
-- @return number

--- Invert given number.
-- @class function
-- @name bnot
-- @param oper Operand
-- @return number

--- Bitwise AND several numbers.
-- @class function
-- @name band
-- @param oper1	First Operand
-- @param oper2 Second Operand
-- @param ...	More Operands
-- @return number

--- Bitwise XOR several numbers.
-- @class function
-- @name bxor
-- @param oper1	First Operand
-- @param oper2 Second Operand
-- @param ...	More Operands
-- @return number

--- Left shift a number.
-- @class function
-- @name lshift
-- @param oper	number
-- @param shift	bits to shift 
-- @return number

--- Right shift a number.
-- @class function
-- @name rshift
-- @param oper	number
-- @param shift	bits to shift 
-- @return number

--- Arithmetically right shift a number.
-- @class function
-- @name arshift
-- @param oper	number
-- @param shift	bits to shift 
-- @return number

--- Integer division of 2 or more numbers.
-- @class function
-- @name div
-- @param oper1	Operand 1
-- @param oper2 Operand 2
-- @param ... More Operands
-- @return number

--- Cast a number to the bit-operating range.
-- @class function
-- @name cast
-- @param oper	number
-- @return number

--- Sets one or more flags of a bitfield.
-- @class function
-- @name set
-- @param bitfield	Bitfield
-- @param flag1 First Flag
-- @param ...	More Flags
-- @return altered bitfield

--- Unsets one or more flags of a bitfield.
-- @class function
-- @name unset
-- @param bitfield	Bitfield
-- @param flag1 First Flag
-- @param ...	More Flags
-- @return altered bitfield

--- Checks whether given flags are set in a bitfield.
-- @class function
-- @name check
-- @param bitfield	Bitfield
-- @param flag1 First Flag
-- @param ...	More Flags
-- @return true when all flags are set, otherwise false
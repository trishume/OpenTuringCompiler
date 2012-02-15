=TO TEST=

==Parser==
 * All constructs
 * Special cases:
 	* Chained field references
 	* Brackets
 	* 0 and 2+ arguments passed in call
 	* elses and elsifs
 	* lotsa different whitespace
 	* whitespace in records

==Compiler==
 * Variables
    * Ensure variables in local scopes are declared in correct order.
 * Arrays
    * Returning an array


=TO FIX=
 * Runtime check range end >= start
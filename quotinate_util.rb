#!ruby -w
#  quotinate_util.rb
#  Turing Compiler
#  quotes all input. Used to properly quote llvm-config output.
#
#  Created by Tristan Hume on 12-02-01.
#  Copyright 2012 15 Norwich Way. All rights reserved.
#

print STDIN.read.split.map {|s| "\"" + s + "\""}.join(" ")



# -*- Mode: Makefile -*-
DIM=2
# trace the chain of include makefiles
makefiles += releasedExamples_EBAMRPoisson_execPoisson

# This is an embedded boundary example
USE_EB=TRUE

## Define the variables needed by Make.example

# the base name(s) of the application(s) in this directory
ebase = ebadvectDriver

# the location of the Chombo "lib" directory
CHOMBO_HOME = ../../../lib

# names of Chombo libraries needed by this program, in order of search.
LibNames = EBAMRTimeDependent EBAMRTools Workshop EBTools  AMRTools BoxTools

# the locations of the source code directories
base_dir = .
src_dirs =  ../src

# input file for 'run' target
INPUT = plane.inputs

# shared code for building example programs
include $(CHOMBO_HOME)/mk/Make.example

# application-specific variables

# application-specific targets

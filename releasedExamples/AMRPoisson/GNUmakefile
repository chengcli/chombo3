# -*- Mode: Makefile -*- 

## This makefile builds one or more subdirectories.

# trace the chain of included makefiles
makefiles += releasedExamples_AMRPoisson

## Define the variables needed by Make.example.subdirs

# the location of the Chombo "lib" directory
CHOMBO_HOME = ../../lib

# the subdirectories to build
ifeq ($USE_PETSC),TRUE)
exec_dirs = execCell          \
            execNode          \
            execVariableCoefficient   \
            execViscousTensor \
            execAMRFAS \
            execPETSc
else 
exec_dirs = execCell          \
            execNode          \
            execVariableCoefficient   \
            execViscousTensor \
            execAMRFAS 
endif

# the shared code for building in example subdirectories
include $(CHOMBO_HOME)/mk/Make.example.subdirs

# application-specific variables

# application-specific targets

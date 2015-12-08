##===- TEST.unroll.Makefile ------------------------------*- Makefile -*-===##
#
# This recursively traverses the programs, and runs the -simplify-unroll pass
# on each *.linked.rbc bytecode file with -stats set so that it is possible to
# determine which unroll are being optimized in which programs.
# 
# Usage: 
#     make TEST=unroll summary (short summary)
#     make TEST=unroll (detailed list with time passes, etc.)
#     make TEST=unroll report
#     make TEST=unroll report.html
#
##===----------------------------------------------------------------------===##

CURDIR  := $(shell cd .; pwd)
PROGDIR := $(PROJ_SRC_ROOT)
RELDIR  := $(subst $(PROGDIR),,$(CURDIR))

$(PROGRAMS_TO_TEST:%=test.$(TEST).%): \
test.$(TEST).%: Output/%.$(TEST).report.txt
	@cat $<

$(PROGRAMS_TO_TEST:%=Output/%.$(TEST).report.txt):  \
Output/%.$(TEST).report.txt: Output/%.linked.rbc $(LOPT) \
	$(PROJ_SRC_ROOT)/TEST.unroll.Makefile 
	$(VERB) $(RM) -f $@
	@echo "---------------------------------------------------------------" >> $@
	@echo ">>> ========= '$(RELDIR)/$*' Program" >> $@
	@echo "---------------------------------------------------------------" >> $@
	@-$(LOPT) -loop-simplify $< 2>>$@ 
summary:
	@$(MAKE) TEST=unroll | egrep '======|simplify-unroll -'

.PHONY: summary
REPORT_DEPENDENCIES := $(LOPT)

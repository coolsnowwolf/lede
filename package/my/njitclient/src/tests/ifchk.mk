
local_source_files := ifchk.c
local_object_files := $(local_source_files:.c=.o)
local_executable_files := ifchk
local_executable_files := $(addsuffix $(EXEEXT), $(local_executable_files))

$(local_executable_files): $(local_object_files)
#-------------------------------------------------
ifchk.pot: $(local_source_files)
$(po_files): ifchk.pot
	$(MSGMERGE) --quiet --directory=$(podir) --update $(notdir $@) $(notdir $<)
#-------------------------------------------------

executable_files += $(local_executable_files)
source_files += $(local_source_files)
object_files += $(local_object_files)


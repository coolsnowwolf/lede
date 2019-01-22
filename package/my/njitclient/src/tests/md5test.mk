
local_source_files := md5test.c
local_object_files := $(local_source_files:.c=.o)
local_executable_files := md5test
local_executable_files := $(addsuffix $(EXEEXT), $(local_executable_files))

$(local_executable_files): $(local_object_files)
md5test.o: CFLAGS+=$(shell pkg-config libcrypto --cflags)
#md5test: LDLIBS+=-lcrypto
md5test: LDLIBS+=$(shell pkg-config libcrypto --libs)
#-------------------------------------------------

executable_files += $(local_executable_files)
source_files += $(local_source_files)
object_files += $(local_object_files)


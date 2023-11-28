MODULE_NAME := nsid3v2

CPPSOURCES := frameheader.cpp header.cpp tag.cpp util.cpp values.cpp extendedheader.cpp frame.cpp nsid3v2_common.cpp frame_text.cpp frame_usertext.cpp frame_private.cpp frame_object.cpp frame_popm.cpp frame_apic.cpp frame_utils.cpp frame_comments.cpp frame_id.cpp
CSOURCES := frames.c 

LIBRARY_FILENAME := lib$(MODULE_NAME).a
OUTPUT_PATH := ../build/$(MODULE_NAME)
ARCHIVE_PATH := ../build/lib
LIBRARY_FILEPATH := ../build/lib/$(LIBRARY_FILENAME)

CPPOBJS := $(patsubst %.cpp,$(OUTPUT_PATH)/%.o,$(CPPSOURCES)) 
CPPDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(CPPOBJS))
COBJS := $(patsubst %.c,$(OUTPUT_PATH)/%.o,$(CSOURCES)) 
CDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(COBJS))

OBJS := $(CPPOBJS) $(COBJS)
DEPS := $(CPPDEPS) $(CDEPS)

CFLAGS=-I.. -fPIC -fvisibility=hidden
CPPFLAGS := ${CFLAGS}



build: build-dir $(LIBRARY_FILEPATH)

build-dir: 
	@mkdir -p $(OUTPUT_PATH)/linux > /dev/null 2> /dev/null
	@mkdir -p $(OUTPUT_PATH) > /dev/null 2> /dev/null
	@mkdir -p $(ARCHIVE_PATH) > /dev/null 2> /dev/null

dep:
	@rm ${DEPS}

$(OUTPUT_PATH)/%.o: %.cpp
	@echo Compiling $*.cpp
	@$(CXX) $(CPPFLAGS) -MMD -MF $(OUTPUT_PATH)/$*.d -MT $(OUTPUT_PATH)/$*.o -c $*.cpp -o $(OUTPUT_PATH)/$*.o

$(OUTPUT_PATH)/%.o: %.c
	@echo Compiling $*.cpp
	@$(CC) $(CFLAGS) -MMD -MF $(OUTPUT_PATH)/$*.d -MT $(OUTPUT_PATH)/$*.o -c $*.c -o $(OUTPUT_PATH)/$*.o

$(LIBRARY_FILEPATH): ${OBJS}
	@$(AR) rcs $@ ${OBJS}

clean:
	-rm -f ${OBJS} $(LIBRARY_FILENAME) ${DEPS} 

-include $(DEPS)

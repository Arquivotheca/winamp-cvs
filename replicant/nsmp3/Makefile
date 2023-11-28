MODULE_NAME := nsmp3

CPPSOURCES :=  mp3decode.cpp mp3quant.cpp mp3read.cpp mp3ssc.cpp \
mp3tools.cpp mpegbitstream.cpp mpegheader.cpp mpgadecoder.cpp polyphase.cpp \
bitstream.cpp conceal.cpp crc16.cpp huffdec.cpp huffmanbitobj.cpp huffmandecoder.cpp huffmantable.cpp \
l3table.cpp mdct.cpp meanvalue.cpp mp2decode.cpp mp3ancofl.cpp 

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

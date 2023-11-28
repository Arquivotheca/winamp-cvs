MODULE_NAME := mp3

CPPSOURCES :=  giofile_crt.cpp MP3Playback.cpp MP3PlaybackService.cpp MP3RawReader.cpp MP3Decoder.cpp MP3DecoderCallback.cpp MP3MetadataService.cpp main.cpp GioReplicant.cpp  CVbriHeader.cpp LAMEInfo.cpp MPEGHeader.cpp OFL.cpp MP4DecoderService.cpp MP4MP3Decoder.cpp 

LDFLAGS =-lc -lpthread -lrt -lz -fvisibility=hidden

LIBRARY_FILENAME := lib$(MODULE_NAME).w6c
OUTPUT_PATH := ../build/$(MODULE_NAME)
ARCHIVE_PATH := ../build/lib
LIBRARY_FILEPATH := ../build/lib/$(LIBRARY_FILENAME)

CPPOBJS := $(patsubst %.cpp,$(OUTPUT_PATH)/%.o,$(CPPSOURCES)) 
CPPDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(CPPOBJS))
COBJS := $(patsubst %.c,$(OUTPUT_PATH)/%.o,$(CSOURCES)) 
CDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(COBJS))

OBJS := $(CPPOBJS) $(COBJS)
DEPS := $(CPPDEPS) $(CDEPS)

CFLAGS=-I.. -DREPLICANT_NO_ICY -DREPLICANT_NO_ULTRAVOX -DREPLICANT_NO_HTTP -fPIC -fvisibility=hidden
CPPFLAGS := ${CFLAGS}

build: build-dir $(LIBRARY_FILEPATH)

build-dir: 
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
	@$(CC) -Wl,-L,'../build/lib' -Wl,-rpath,'$$ORIGIN' -shared -Wl,-soname,libmp3.w6c -o $(LIBRARY_FILEPATH) ${OBJS} ${LDFLAGS} -lnswasabi -lnsmp3 -lnsid3v1  -lnsid3v2  -lnsapev2  -lnx  -lnu

clean:
	-rm -f ${OBJS} $(LIBRARY_FILENAME) ${DEPS} 

-include $(DEPS)

MODULE_NAME := jnet

CPPSOURCES := asyncdns.cpp connection.cpp httpget.cpp util.cpp multicastlisten.cpp udpconnection.cpp jnetlib.cpp

LDFLAGS = -fvisibility=hidden

LIBRARY_FILENAME := lib$(MODULE_NAME).so
OUTPUT_PATH := ../build/$(MODULE_NAME)
ARCHIVE_PATH := ../build/lib
LIBRARY_FILEPATH := ../build/lib/$(LIBRARY_FILENAME)

CPPOBJS := $(patsubst %.cpp,$(OUTPUT_PATH)/%.o,$(CPPSOURCES)) 
CPPDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(CPPOBJS))
COBJS := $(patsubst %.c,$(OUTPUT_PATH)/%.o,$(CSOURCES)) 
CDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(COBJS))

OBJS := $(CPPOBJS) $(COBJS)
DEPS := $(CPPDEPS) $(CDEPS)

CFLAGS=-I.. -DREPLICANT_NO_ICY -DREPLICANT_NO_ULTRAVOX -DREPLICANT_NO_HTTP -D_LARGEFILE64_SOURCE -fPIC -fvisibility=hidden
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
	@$(CC) -Wl,-L,'../build/lib' -shared -Wl,-soname,libjnet.so -o $(LIBRARY_FILEPATH) ${OBJS} ${LDFLAGS} -lnu

clean:
	-rm -f ${OBJS} $(LIBRARY_FILENAME) ${DEPS} 

-include $(DEPS)


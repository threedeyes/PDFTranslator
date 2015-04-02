
BINARY := ./build/PDFTranslator

OBJS := shared/TranslatorSettings.o \
		shared/TranslatorWindow.o \
		shared/BaseTranslator.o \
		shared/StreamBuffer.o \
		PDFTranslator.o \
		PDFLoader.o \
		ConfigView.o \
		PDFTranslatorApp.o

OBJDIR := build

RDEFS := ./res/PDFTranslator.rdef
RSRCS := ./build/PDFTranslator.rsrc

OBJS	:= $(addprefix $(OBJDIR)/,$(OBJS))

CC := g++
LD := $(CC)

LIBS := -lbe -ltranslation -lmupdf
CFLAGS := -O3 -I./src -I./src/shared 
LDFLAGS := 

.PHONY : clean build install

default : build

build : $(BINARY)
	
$(BINARY) : $(OBJDIR) $(OBJS) $(RSRCS)	
	$(LD) $(CFLAGS) $(OBJS) -o $(BINARY) $(LDFLAGS) $(LIBS)
	strip $(BINARY)
	xres -o $(BINARY) $(RSRCS)
	mimeset -f $(BINARY)

clean:
	rm -rf $(OBJDIR)

build/%.o : src/%.cpp	
	@mkdir -p $(OBJDIR)/shared
	$(CC) $(CFLAGS) -c $< -o $@

build/%.rsrc : res/%.rdef
	rc -o $@ $<

install:
	mkdir -p /boot/home/config/non-packaged/add-ons/Translators
	cp $(BINARY) /boot/home/config/non-packaged/add-ons/Translators

TOPDIR = .

include $(TOPDIR)/config

SRCDIR = src

.PHONY: all package run clean

all: $(BINARY)

$(BINARY):
	@echo "\nCompiling sources in '$(SRCDIR)'...\n"
	@cd $(SRCDIR) && $(MAKE)
	@echo "\n...Done building."

package: $(BINARY) $(RESDIR)/mac/* $(RESDIR)/common/*
	@echo "\nPackaging $(BINARY)..."
	@mkdir -p $(MACAPP)/Contents
	@cp $(RESDIR)/mac/Info.plist $(MACAPP)/Contents
	@mkdir -p $(MACAPP)/Contents/MacOS
	@cp $(RESDIR)/mac/$(BINARY).sh $(MACAPP)/Contents/MacOS
	@cp $(BINARY) $(MACAPP)/Contents/MacOS
	@cp -r $(RESDIR)/common/font $(MACAPP)/Contents/MacOS
	@echo "Packaged $(BINARY) into $(MACAPP)\n"

run: package
	@echo "Running $(BINARY)..."
	open $(MACAPP)

clean:
	@echo Cleaning up...
	-@rm $(BINARY) 2> /dev/null || echo "There was no binary..."
	-@rm -r $(MACAPP) 2> /dev/null || echo "There was no app..."
	@echo Done.

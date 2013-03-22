TOPDIR = .

include $(TOPDIR)/config

SRCDIR = src

all:
	@echo "\nCompiling sources in '$(SRCDIR)'...\n"
	@cd $(SRCDIR) && $(MAKE)
	@echo "\n...Done building."

package: all
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
	@rm $(BINARY)
	@rm -r $(MACAPP)
	@echo Done.

########################################################################
# Makefile for Nanotech Construction Kit, an interactive molecular
# dynamics simulation.
# Copyright (c) 2008-2011 Oliver Kreylos
#
# This file is part of the WhyTools Build Environment.
# 
# The WhyTools Build Environment is free software; you can redistribute
# it and/or modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
# 
# The WhyTools Build Environment is distributed in the hope that it will
# be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the WhyTools Build Environment; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA
########################################################################

# Root directory of the Vrui software installation. This must match the
# same setting in Vrui's makefile. By default the directories match; if
# the installation directory was adjusted during Vrui's installation, it
# must be adjusted here as well.
VRUIDIR = $(HOME)/Vrui-2.1

# Base installation directory for Nanotech Construction Kit. If this is
# set to the default of $(PWD), Nanotech Construction Kit does not have
# to be installed to be run. Nanotech Construction Kit's executable will
# be installed in the bin directory underneath the given base directory,
# and its data files will be installed in the share directory.
INSTALLDIR = $(shell pwd)

########################################################################
# Nothing underneath here needs to be changed.
########################################################################

# Version number for installation subdirectories. This is used to keep
# subsequent release versions of the Nanotech Construction Kit from
# clobbering each other. The value should be identical to the
# major.minor version number found in VERSION in the root package
# directory.
VERSION = 1.5

# Set up destination directories for compilation products:
OBJDIRBASE = o
BINDIRBASE = bin

# Set up resource directories: */
CONFIGDIR = etc/NCK-$(VERSION)

# Set up additional flags for the C++ compiler:
CFLAGS = 

# Create debug or fully optimized versions of the software:
VRUIMAKEDIR = $(VRUIDIR)/share
ifdef DEBUG
  # Include the debug version of the Vrui application makefile fragment:
  include $(VRUIMAKEDIR)/Vrui.debug.makeinclude
  # Enable debugging and disable optimization:
  CFLAGS += -g3 -O0
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)/debug
  BINDIR = $(BINDIRBASE)/debug
else
  # Include the release version of the Vrui application makefile fragment:
  include $(VRUIMAKEDIR)/Vrui.makeinclude
  # Disable debugging and enable optimization:
  CFLAGS += -g0 -O3 -DNDEBUG
  # Set destination directories for created objects:
  OBJDIR = $(OBJDIRBASE)
  BINDIR = $(BINDIRBASE)
endif

# Set up installation directory structure:
BININSTALLDIR = $(INSTALLDIR)/$(BINDIRBASE)
ETCINSTALLDIR = $(INSTALLDIR)/$(CONFIGDIR)

# Pattern rule to compile C++ sources:
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)/$(*D)
	@echo Compiling $<...
	@g++ -c -o $@ $(VRUI_CFLAGS) $(CFLAGS) $<

# Rule to build all Nanotech Construction Kit components:
ALL = $(BINDIR)/NanotechConstructionKit
.PHONY: all
all: $(ALL)

# Rule to remove build results:
clean:
	-rm -f $(OBJDIR)/*.o
	-rm -f $(ALL)
	-rmdir $(BINDIR)

# Rule to clean the source directory for packaging:
distclean:
	-rm -rf $(OBJDIRBASE)
	-rm -rf $(BINDIRBASE)

NANOTECHCONSTRUCTIONKIT_SOURCES = StructuralUnit.cpp \
                                  Triangle.cpp \
                                  Tetrahedron.cpp \
                                  Octahedron.cpp \
                                  Sphere.cpp \
                                  Cylinder.cpp \
                                  UnitManager.cpp \
                                  SpaceGridCell.cpp \
                                  GhostUnit.cpp \
                                  SpaceGrid.cpp \
                                  ReadUnitFile.cpp \
                                  ReadCarFile.cpp \
                                  Polyhedron.cpp \
                                  UnitDragger.cpp \
                                  NanotechConstructionKit.cpp

$(OBJDIR)/NanotechConstructionKit.o: CFLAGS += -DNANOTECHCONSTRUCTIONKIT_CFGFILENAME='"$(ETCINSTALLDIR)/NCK.cfg"'

$(BINDIR)/NanotechConstructionKit: $(NANOTECHCONSTRUCTIONKIT_SOURCES:%.cpp=$(OBJDIR)/%.o)
	@mkdir -p $(BINDIR)
	@echo Linking $@...
	@g++ -o $@ $^ $(VRUI_LINKFLAGS)
.PHONY: NanotechConstructionKit
NanotechConstructionKit: $(BINDIR)/NanotechConstructionKit

install: $(ALL)
	@echo Installing Nanotech Construction Kit in $(INSTALLDIR)...
	@install -d $(INSTALLDIR)
	@install -d $(BININSTALLDIR)
	@install $(ALL) $(BININSTALLDIR)
	@install -d $(ETCINSTALLDIR)
	@install $(CONFIGDIR)/NCK.cfg $(ETCINSTALLDIR)

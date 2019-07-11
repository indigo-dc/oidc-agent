UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	MAC_OS = 1
endif

# Executable names
AGENT    = oidc-agent
GEN			 = oidc-gen
ADD      = oidc-add
CLIENT	 = oidc-token

VERSION   ?= $(shell cat VERSION)
# DIST      = $(lsb_release -cs)
LIBMAJORVERSION ?= $(shell echo $(VERSION) | cut -d '.' -f 1)
# Generated lib version / name
LIBVERSION = $(VERSION)
ifdef MAC_OS
SONAME = liboidc-agent.$(LIBMAJORVERSION).dylib
SHARED_LIB_NAME_FULL = liboidc-agent.$(LIBVERSION).dylib
SHARED_LIB_NAME_SO = $(SONAME)
SHARED_LIB_NAME_SHORT = liboidc-agent.dylib
else
SONAME = liboidc-agent.so.$(LIBMAJORVERSION)
SHARED_LIB_NAME_FULL = liboidc-agent.so.$(LIBVERSION)
SHARED_LIB_NAME_SO = $(SONAME)
SHARED_LIB_NAME_SHORT = liboidc-agent.so
endif

# These are needed for the RPM build target:
#BASEDIR   = $(PWD)
BASEDIR   = $(shell pwd)
BASENAME := $(notdir $(BASEDIR))
SRC_TAR   = oidc-agent.tar
PKG_NAME  = oidc-agent

# Local dir names
SRCDIR   = src
OBJDIR   = obj
PICOBJDIR= pic-obj
BINDIR   = bin
LIBDIR   = lib
APILIB   = $(LIBDIR)/api
MANDIR 	 = man
CONFDIR  = config
PROVIDERCONFIG = issuer.config
PUBCLIENTSCONFIG = pubclients.config

TESTSRCDIR = test/src
TESTBINDIR = test/bin

ifdef HAS_CJSON
	DEFINE_HAS_CJSON = -DHAS_CJSON
endif

# Compiler options
CC       = gcc
# compiling flags here
CFLAGS   = -g -std=c99 -I$(SRCDIR) -I$(LIBDIR)  #-Wall -Wextra
ifndef MAC_OS
	CFLAGS += $(shell pkg-config --cflags libsecret-1)
endif
TEST_CFLAGS = $(CFLAGS) -I.

# Linker options
LINKER   = gcc
ifdef MAC_OS
LFLAGS   = -lsodium -largp
else
LFLAGS   = -l:libsodium.a -lseccomp
endif
ifdef HAS_CJSON
	LFLAGS += -lcjson
endif
AGENT_LFLAGS = -lcurl -lmicrohttpd $(LFLAGS)
ifndef MAC_OS
	AGENT_LFLAGS += -lsecret-1 -lglib-2.0
endif
GEN_LFLAGS = $(LFLAGS) -lmicrohttpd
ADD_LFLAGS = $(LFLAGS)
ifdef MAC_OS
CLIENT_LFLAGS = -L$(APILIB) -largp -loidc-agent.$(LIBVERSION)
else
CLIENT_LFLAGS = -L$(APILIB) -l:$(SHARED_LIB_NAME_FULL) -lseccomp
endif
ifdef HAS_CJSON
	CLIENT_LFLAGS += -lcjson
endif
TEST_LFLAGS = $(LFLAGS) $(shell pkg-config --cflags --libs check)

# Install paths
ifndef MAC_OS
PREFIX                    ?=
BIN_PATH             			?=$(PREFIX)/usr# /bin is appended later
BIN_AFTER_INST_PATH				?=$(BIN_PATH)# needed for debian package and desktop file exec
LIB_PATH 	           			?=$(PREFIX)/usr/lib/x86_64-linux-gnu
LIBDEV_PATH 	       			?=$(PREFIX)/usr/lib/x86_64-linux-gnu
INCLUDE_PATH         			?=$(PREFIX)/usr/include/x86_64-linux-gnu
MAN_PATH             			?=$(PREFIX)/usr/share/man
CONFIG_PATH          			?=$(PREFIX)/etc
BASH_COMPLETION_PATH 			?=$(PREFIX)/usr/share/bash-completion/completions
DESKTOP_APPLICATION_PATH 	?=$(PREFIX)/usr/share/applications
XSESSION_PATH							?=$(PREFIX)/etc/X11
else
PREFIX                    ?=/usr/local
BIN_PATH             			?=$(PREFIX)# /bin is appended later
BIN_AFTER_INST_PATH				?=$(BIN_PATH)# needed for debian package and desktop file exec
LIB_PATH 	           			?=$(PREFIX)/lib
LIBDEV_PATH 	       			?=$(PREFIX)/lib
INCLUDE_PATH         			?=$(PREFIX)/include
MAN_PATH             			?=$(PREFIX)/share/man
CONFIG_PATH          			?=$(PREFIX)/etc
endif

# Define sources
SRC_SOURCES := $(shell find $(SRCDIR) -name "*.c")
LIB_SOURCES := $(LIBDIR)/list/list.c $(LIBDIR)/list/list_iterator.c $(LIBDIR)/list/list_node.c
ifndef HAS_CJSON
	LIB_SOURCES += $(LIBDIR)/cJSON/cJSON.c
endif
SOURCES  := $(SRC_SOURCES) $(LIB_SOURCES)
INCLUDES := $(shell find $(SRCDIR) -name "*.h") $(LIBDIR)/cJSON/cJSON.h $(LIBDIR)/list/list.h

GENERAL_SOURCES := $(shell find $(SRCDIR)/utils -name "*.c") $(shell find $(SRCDIR)/account -name "*.c") $(shell find $(SRCDIR)/ipc -name "*.c")
ifndef MAC_OS
	GENERAL_SOURCES += $(shell find $(SRCDIR)/privileges -name "*.c")
endif
AGENT_SOURCES_TMP := $(shell find $(SRCDIR)/$(AGENT) -name "*.c")
ifdef MAC_OS
	AGENT_SOURCES= $(filter-out $(SRCDIR)/$(AGENT)/oidcp/passwords/keyring.c, $(AGENT_SOURCES_TMP))
else
	AGENT_SOURCES = $(AGENT_SOURCES_TMP)
endif
GEN_SOURCES := $(shell find $(SRCDIR)/$(GEN) -name "*.c")
ADD_SOURCES := $(shell find $(SRCDIR)/$(ADD) -name "*.c")
CLIENT_SOURCES := $(shell find $(SRCDIR)/$(CLIENT) -name "*.c")
TEST_SOURCES :=  $(filter-out $(TESTSRCDIR)/main.c, $(shell find $(TESTSRCDIR) -name "*.c"))

# Define objects
ALL_OBJECTS  := $(SRC_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS  := $(AGENT_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
GEN_OBJECTS  := $(GEN_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(OBJDIR)/oidc-agent/httpserver/termHttpserver.o $(OBJDIR)/oidc-agent/httpserver/running_server.o $(OBJDIR)/oidc-agent/oidc/device_code.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
ADD_OBJECTS  := $(ADD_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
CLIENT_OBJECTS := $(OBJDIR)/$(CLIENT)/$(CLIENT).o $(OBJDIR)/utils/disableTracing.o $(OBJDIR)/utils/stringUtils.o
ifndef MAC_OS
	CLIENT_OBJECTS += $(OBJDIR)/privileges/privileges.o $(OBJDIR)/privileges/token_privileges.o $(OBJDIR)/utils/file_io/file_io.o
endif
API_OBJECTS := $(OBJDIR)/$(CLIENT)/api.o $(OBJDIR)/ipc/ipc.o $(OBJDIR)/ipc/communicator.o $(OBJDIR)/utils/json.o $(OBJDIR)/utils/memory.o $(OBJDIR)/utils/stringUtils.o  $(OBJDIR)/utils/colors.o $(OBJDIR)/utils/printer.o $(OBJDIR)/utils/listUtils.o $(OBJDIR)/utils/logger.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
PIC_OBJECTS := $(API_OBJECTS:$(OBJDIR)/%=$(PICOBJDIR)/%)
ifdef MAC_OS
	PIC_OBJECTS += $(OBJDIR)/utils/file_io/file_io.o $(OBJDIR)/utils/file_io/oidc_file_io.o
endif
rm       = rm -f

# RULES

.PHONY: all
all: build man

# Compiling

.PHONY: build
build: create_obj_dir_structure $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)

## pull in dependency info for *existing* .o files
-include $(ALL_OBJECTS:.o=.d)

## Compile and generate depencency info
$(OBJDIR)/$(CLIENT)/$(CLIENT).o : $(APILIB)/$(SHARED_LIB_NAME_FULL)
$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\" -DCONFIG_PATH=\"$(CONFIG_PATH)\" $(DEFINE_HAS_CJSON)
	@# Create dependency infos
	@{ \
	set -e ;\
	depFileName=$(OBJDIR)/$*.d ;\
	$(CC) -MM $(CFLAGS) $< -o $${depFileName} ;\
	mv -f $${depFileName} $${depFileName}.tmp ;\
	sed -e 's|.*:|$@:|' < $${depFileName}.tmp > $${depFileName} ;\
	cp -f $${depFileName} $${depFileName}.tmp ;\
	sed -e 's/.*://' -e 's/\\$$//' < $${depFileName}.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $${depFileName} ;\
	rm -f $${depFileName}.tmp ;\
	}
	@echo "Compiled "$<" successfully!"

## Compile lib sources
$(OBJDIR)/%.o : $(LIBDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

## Compile position independent code
$(PICOBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -fpic -c $< -o $@ -DVERSION=\"$(VERSION)\" -DCONFIG_PATH=\"$(CONFIG_PATH)\"
	@echo "Compiled "$<" with pic successfully!"

$(PICOBJDIR)/%.o : $(LIBDIR)/%.c
	@$(CC) $(CFLAGS) -fpic -c $< -o $@
	@echo "Compiled "$<" with pic successfully!"


# Linking

$(BINDIR)/$(AGENT): create_obj_dir_structure $(AGENT_OBJECTS) $(BINDIR)
	@$(LINKER) $(AGENT_OBJECTS) $(AGENT_LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(GEN): create_obj_dir_structure $(GEN_OBJECTS) $(BINDIR)
	@$(LINKER) $(GEN_OBJECTS) $(GEN_LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(ADD): create_obj_dir_structure $(ADD_OBJECTS) $(BINDIR)
	@$(LINKER) $(ADD_OBJECTS) $(ADD_LFLAGS) -o $@
	@echo "Linking "$@" complete!"

$(BINDIR)/$(CLIENT): create_obj_dir_structure $(CLIENT_OBJECTS) $(APILIB)/$(SHARED_LIB_NAME_FULL) $(BINDIR)
	@$(LINKER) $(CLIENT_OBJECTS) $(CLIENT_LFLAGS) -o $@
	@echo "Linking "$@" complete!"

# Phony Installer

.PHONY: install
ifndef MAC_OS
install: install_bin install_man install_conf install_bash install_priv install_scheme_handler install_xsession_script
else
install: install_bin install_man install_conf install_scheme_handler
endif
	@echo "Installation complete!"

.PHONY: install_bin
install_bin: $(BIN_PATH)/bin/$(AGENT) $(BIN_PATH)/bin/$(GEN) $(BIN_PATH)/bin/$(ADD) $(BIN_PATH)/bin/$(CLIENT)
	@echo "Installed binaries"

.PHONY: install_conf
install_conf: $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG) $(CONFIG_PATH)/oidc-agent/$(PUBCLIENTSCONFIG)
	@echo "Installed issuer.config"

.PHONY: install_priv
install_priv: $(CONFDIR)/privileges/
	@install -d $(CONFIG_PATH)/oidc-agent/privileges/
# ifdef MAC_OS
	@install -m 644 $(CONFDIR)/privileges/* $(CONFIG_PATH)/oidc-agent/privileges/
# else
# 	@install -m 644 -D $(CONFDIR)/privileges/* $(CONFIG_PATH)/oidc-agent/privileges/
# endif
	@echo "installed privileges files"

.PHONY: install_bash
install_bash: $(BASH_COMPLETION_PATH)/$(AGENT) $(BASH_COMPLETION_PATH)/$(GEN) $(BASH_COMPLETION_PATH)/$(ADD) $(BASH_COMPLETION_PATH)/$(CLIENT)
	@echo "Installed bash completion"

.PHONY: install_man
install_man: $(MAN_PATH)/man1/$(AGENT).1 $(MAN_PATH)/man1/$(GEN).1 $(MAN_PATH)/man1/$(ADD).1 $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Installed man pages!"

.PHONY: install_lib
install_lib: $(LIB_PATH)/$(SHARED_LIB_NAME_FULL) $(LIB_PATH)/$(SHARED_LIB_NAME_SO)
	@echo "Installed library"

.PHONY: install_lib-dev
install_lib-dev: $(LIB_PATH)/$(SHARED_LIB_NAME_FULL) $(LIB_PATH)/$(SHARED_LIB_NAME_SO) $(LIBDEV_PATH)/$(SHARED_LIB_NAME_SHORT) $(LIBDEV_PATH)/liboidc-agent.a $(INCLUDE_PATH)/oidc-agent/api.h $(INCLUDE_PATH)/oidc-agent/ipc_values.h $(INCLUDE_PATH)/oidc-agent/oidc_error.h
	@echo "Installed library dev"

.PHONY: install_scheme_handler
ifndef MAC_OS
install_scheme_handler: $(DESKTOP_APPLICATION_PATH)/oidc-gen.desktop
	@echo "Installed scheme handler"
else
install_scheme_handler:
	@osacompile -o oidc-gen.app config/scheme_handler/oidc-gen.apple
	@(awk 'n>=2 {print a[n%2]} {a[n%2]=$$0; n=n+1}' oidc-gen.app/Contents/Info.plist ; cat $(CONFDIR)/scheme_handler/Info.plist.template ; tail -2 oidc-gen.app/Contents/Info.plist) > oidc-gen.app/Contents/Info.plist
endif

.PHONY: install_xsession_script
install_xsession_script: $(XSESSION_PATH)/Xsession.d/91oidc-agent
	@echo "Installed xsession_script"

.PHONY: post_install
post_install:
ifndef MAC_OS
	@ldconfig
	@update-desktop-database
	@grep -Fxq "use-oidc-agent" $(XSESSION_PATH)/Xsession.options || echo "use-oidc-agent" >> $(XSESSION_PATH)/Xsession.options
else
	@open -a oidc-gen #open the app one time so the handler is registered
endif
	@echo "Post install completed"

# Install files
## Binaries
$(BIN_PATH)/bin/$(AGENT): $(BINDIR)/$(AGENT) $(BIN_PATH)/bin
	@install $< $@

$(BIN_PATH)/bin/$(GEN): $(BINDIR)/$(GEN) $(BIN_PATH)/bin
	@install $< $@

$(BIN_PATH)/bin/$(ADD): $(BINDIR)/$(ADD) $(BIN_PATH)/bin
	@install $< $@

$(BIN_PATH)/bin/$(CLIENT): $(BINDIR)/$(CLIENT) $(BIN_PATH)/bin
	@install $< $@

## Config
$(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG): $(CONFDIR)/$(PROVIDERCONFIG) $(CONFIG_PATH)/oidc-agent
	@install -m 644 $< $@

$(CONFIG_PATH)/oidc-agent/$(PUBCLIENTSCONFIG): $(CONFDIR)/$(PUBCLIENTSCONFIG) $(CONFIG_PATH)/oidc-agent
	@install -m 644 $< $@

## Bash completion
$(BASH_COMPLETION_PATH)/$(AGENT): $(CONFDIR)/bash-completion/oidc-agent $(BASH_COMPLETION_PATH)
	@install -m 744 $< $@

$(BASH_COMPLETION_PATH)/$(GEN): $(BASH_COMPLETION_PATH)
	@ln -s $(AGENT) $@

$(BASH_COMPLETION_PATH)/$(ADD): $(BASH_COMPLETION_PATH)
	@ln -s $(AGENT) $@

$(BASH_COMPLETION_PATH)/$(CLIENT): $(BASH_COMPLETION_PATH)
	@ln -s $(AGENT) $@

## Man pages
$(MAN_PATH)/man1/$(AGENT).1: $(MANDIR)/$(AGENT).1 $(MAN_PATH)/man1
	@install $< $@
$(MAN_PATH)/man1/$(GEN).1: $(MANDIR)/$(GEN).1 $(MAN_PATH)/man1
	@install $< $@
$(MAN_PATH)/man1/$(ADD).1: $(MANDIR)/$(ADD).1 $(MAN_PATH)/man1
	@install $< $@
$(MAN_PATH)/man1/$(CLIENT).1: $(MANDIR)/$(CLIENT).1 $(MAN_PATH)/man1
	@install $< $@

## Lib
$(LIB_PATH)/$(SHARED_LIB_NAME_FULL): $(APILIB)/$(SHARED_LIB_NAME_FULL) $(LIB_PATH)
	@install $< $@

$(LIB_PATH)/$(SHARED_LIB_NAME_SO): $(LIB_PATH)
	@ln -s $(SHARED_LIB_NAME_FULL) $@

$(LIBDEV_PATH)/$(SHARED_LIB_NAME_SHORT): $(LIBDEV_PATH)
	@ln -s $(SHARED_LIB_NAME_SO) $@

$(INCLUDE_PATH)/oidc-agent/api.h: $(SRCDIR)/$(CLIENT)/api.h $(INCLUDE_PATH)/oidc-agent
	@install $< $@

$(INCLUDE_PATH)/oidc-agent/ipc_values.h: $(SRCDIR)/defines/ipc_values.h $(INCLUDE_PATH)/oidc-agent
	@install $< $@

$(INCLUDE_PATH)/oidc-agent/oidc_error.h: $(SRCDIR)/utils/oidc_error.h $(INCLUDE_PATH)/oidc-agent
	@install $< $@

$(LIBDEV_PATH)/liboidc-agent.a: $(APILIB)/liboidc-agent.a $(LIBDEV_PATH)
	@install $< $@

## scheme handler
$(DESKTOP_APPLICATION_PATH)/oidc-gen.desktop: $(CONFDIR)/scheme_handler/oidc-gen.desktop
	@install -D $< $@
	@echo "Exec=x-terminal-emulator -e bash -c \"$(BIN_AFTER_INST_PATH)/bin/$(GEN) --codeExchange=%u; exec bash\"" >> $@

## Xsession
$(XSESSION_PATH)/Xsession.d/91oidc-agent: $(CONFDIR)/Xsession/91oidc-agent
	@install -m 644 -D $< $@
	@sed -i -e 's!/usr/bin!$(BIN_AFTER_INST_PATH)/bin!g' $@

# Uninstall

.PHONY: purge
ifndef MAC_OS
purge: uninstall uninstall_conf uninstall_priv
else
purge: uninstall uninstall_conf
endif

.PHONY: uninstall
ifndef MAC_OS
uninstall: uninstall_man uninstall_bin uninstall_bashcompletion uninstall_scheme_handler
else
uninstall: uninstall_man uninstall_bin uninstall_scheme_handler
endif

.PHONY: uninstall_bin
uninstall_bin:
	@$(rm) $(BIN_PATH)/bin/$(AGENT)
	@$(rm) $(BIN_PATH)/bin/$(GEN)
	@$(rm) $(BIN_PATH)/bin/$(ADD)
	@$(rm) $(BIN_PATH)/bin/$(CLIENT)
	@echo "Uninstalled binaries"

.PHONY: uninstall_man
uninstall_man:
	@$(rm) $(MAN_PATH)/man1/$(AGENT).1
	@$(rm) $(MAN_PATH)/man1/$(GEN).1
	@$(rm) $(MAN_PATH)/man1/$(ADD).1
	@$(rm) $(MAN_PATH)/man1/$(CLIENT).1
	@echo "Uninstalled man pages!"

.PHONY: uninstall_conf
uninstall_conf:
	@$(rm) $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG)
	@$(rm) $(CONFIG_PATH)/oidc-agent/$(PUBCLIENTSCONFIG)
	@echo "Uninstalled config"

.PHONY: uninstall_priv
uninstall_priv:
	@$(rm) -r $(CONFIG_PATH)/oidc-agent/privileges/
	@echo "Uninstalled privileges config files"

.PHONY: uninstall_bashcompletion
uninstall_bashcompletion:
	@$(rm) $(BASH_COMPLETION_PATH)/$(CLIENT)
	@$(rm) $(BASH_COMPLETION_PATH)/$(GEN)
	@$(rm) $(BASH_COMPLETION_PATH)/$(ADD)
	@$(rm) $(BASH_COMPLETION_PATH)/$(AGENT)
	@echo "Uninstalled bash completion"

.PHONY: uninstall_lib
uninstall_lib:
	@$(rm) $(LIB_PATH)/$(SHARED_LIB_NAME_FULL)
	@$(rm) $(LIB_PATH)/$(SHARED_LIB_NAME_SO)
	@echo "Uninstalled liboidc-agent"

.PHONY: uninstall_libdev
uninstall_libdev: uninstall_lib
	@$(rm) $(LIB_PATH)/$(SHARED_LIB_NAME_SHORT)
	@$(rm) -r $(INCLUDE_PATH)/oidc-agent/
	@echo "Uninstalled liboidc-agent-dev"

.PHONY: uninstall_scheme_handler
uninstall_scheme_handler:
ifndef MAC_OS
	@$(rm) $(DESKTOP_APPLICATION_PATH)/oidc-gen.desktop
else
	@$(rm) -r oidc-gen.app/
endif
	@echo "Uninstalled scheme handler"

# Man pages

.PHONY: create_man
create_man: $(MANDIR)/$(AGENT).1 $(MANDIR)/$(GEN).1 $(MANDIR)/$(ADD).1 $(MANDIR)/$(CLIENT).1
	@echo "Created man pages"

$(MANDIR)/$(AGENT).1: $(MANDIR) $(BINDIR)/$(AGENT) $(SRCDIR)/h2m/$(AGENT).h2m
	@help2man $(BINDIR)/$(AGENT) -o $(MANDIR)/$(AGENT).1 --name="OIDC token agent" -s 1 -N -i $(SRCDIR)/h2m/$(AGENT).h2m

$(MANDIR)/$(GEN).1: $(MANDIR) $(BINDIR)/$(GEN) $(SRCDIR)/h2m/$(GEN).h2m
	@help2man $(BINDIR)/$(GEN) -o $(MANDIR)/$(GEN).1 --name="generates account configurations for oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(GEN).h2m

$(MANDIR)/$(ADD).1: $(MANDIR) $(BINDIR)/$(ADD) $(SRCDIR)/h2m/$(ADD).h2m
	@help2man $(BINDIR)/$(ADD) -o $(MANDIR)/$(ADD).1 --name="adds account configurations to oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(ADD).h2m

$(MANDIR)/$(CLIENT).1: $(MANDIR) $(BINDIR)/$(CLIENT) $(SRCDIR)/h2m/$(CLIENT).h2m $(LIB_PATH)/$(SHARED_LIB_NAME_SO) $(LIB_PATH)/$(SHARED_LIB_NAME_FULL)
	@export LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH && help2man $(BINDIR)/$(CLIENT) -o $(MANDIR)/$(CLIENT).1 --name="gets OIDC access token from oidc-agent" -s 1 -N -i $(SRCDIR)/h2m/$(CLIENT).h2m

# Library

$(APILIB)/liboidc-agent.a: $(APILIB) $(API_OBJECTS)
	@ar -crs $@ $(API_OBJECTS)

$(APILIB)/$(SHARED_LIB_NAME_FULL): create_picobj_dir_structure $(APILIB) $(PIC_OBJECTS)
ifdef MAC_OS
	@gcc -dynamiclib -fpic -Wl, -o $@ $(PIC_OBJECTS) -lc
else
	@gcc -shared -fpic -Wl,-soname,$(SONAME) -o $@ $(PIC_OBJECTS) -lc
endif

.PHONY: shared_lib
shared_lib: $(APILIB)/$(SHARED_LIB_NAME_FULL)
	@echo "Created shared library"



# Helpers

$(LIB_PATH):
	@install -d $@

ifneq ($(LIB_PATH), $(LIBDEV_PATH))
$(LIBDEV_PATH):
	@install -d $@
endif

$(INCLUDE_PATH)/oidc-agent:
	@install -d $@

$(BIN_PATH)/bin:
	@install -d $@

$(CONFIG_PATH)/oidc-agent:
	@install -d $@

$(BASH_COMPLETION_PATH):
	@install -d $@

$(MAN_PATH)/man1:
	@install -d $@

$(BINDIR):
	@mkdir -p $(BINDIR)

$(MANDIR):
	@mkdir -p $(MANDIR)

$(APILIB):
	@mkdir -p $(APILIB)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(PICOBJDIR):
	@mkdir -p $(PICOBJDIR)

$(TESTBINDIR):
	@mkdir -p $@

.PHONY: create_obj_dir_structure
create_obj_dir_structure: $(OBJDIR)
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;
	@cd $(LIBDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;

.PHONY: create_picobj_dir_structure
create_picobj_dir_structure: $(PICOBJDIR)
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(PICOBJDIR)/{} \;
	@cd $(LIBDIR) && find . -type d -exec mkdir -p -- ../$(PICOBJDIR)/{} \;

# Cleaners

.PHONY: clean
clean: cleanobj cleanapi cleanpackage cleantest distclean

.PHONY: cleanobj
cleanobj:
	@$(rm) -r $(OBJDIR)
	@$(rm) -r $(PICOBJDIR)

.PHONY: cleanpackage
cleanpackage:
	@$(rm) -r debian/.debhelper
	@$(rm) -r rpm/rpmbuild
	@$(rm) -r debian/files
	@$(rm) -r debian/liboidc-dev*
	@$(rm) -r debian/liboidc-agent3
	@$(rm) -r debian/liboidc-agent3.debhelper.log
	@$(rm) -r debian/liboidc-agent3.substvars
	@$(rm) -r debian/liboidc-agent-dev
	@$(rm) -r debian/liboidc-agent-dev.debhelper.log
	@$(rm) -r debian/liboidc-agent-dev.substvars
	@$(rm) -r debian/oidc-agent
	@$(rm) -r debian/oidc-agent.debhelper.log
	@$(rm) -r debian/oidc-agent.substvars

.PHONY: cleantest
cleantest:
	@$(rm) -r $(TESTBINDIR)

.PHONY: distclean
distclean: cleanobj
	@$(rm) -r $(BINDIR)
	@$(rm) -r $(MANDIR)

.PHONY: cleanapi
cleanapi:
	@$(rm) -r $(APILIB)

.PHONY: remove
remove: cleanobj cleanapi cleanpackage cleantest distclean

# Packaging

.PHONY: deb
deb: create_obj_dir_structure VERSION
	perl -0777 -pi -e 's/(\().*?(\))/`echo -n "("; echo -n $(VERSION); echo -n ")"`/e' debian/changelog
	debuild -b -uc -us
	@echo "Success: DEBs are in parent directory"

.PHONY: srctar
srctar:
	@#@(cd ..; tar cf $(BASENAME)/$(SRC_TAR) $(BASENAME)/src $(BASENAME)/Makefile)
	@tar cf $(SRC_TAR) src lib Makefile config LICENSE README.md VERSION --transform='s_^_$(PKG_NAME)-$(VERSION)/_'

.PHONY: rpm
rpm: srctar
	curl  http://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm > epel-release-latest-7.noarch.rpm
	rpm -U epel-release-latest-7.noarch.rpm || echo ""
	yum-builddep -y rpm/oidc-agent.spec
	@mkdir -p rpm/rpmbuild/SOURCES
	@#@cp -af src Makefile  rpm/rpmbuild/SOURCES
	@mv oidc-agent.tar rpm/rpmbuild/SOURCES/
	rpmbuild --define "_topdir $(BASEDIR)/rpm/rpmbuild" -bb  rpm/oidc-agent.spec
	@#rpmbuild --define "_topdir $(BASEDIR)/rpm/rpmbuild" -bb  rpm/liboidc-agent3.spec
	@mv rpm/rpmbuild/RPMS/*/*rpm ..
	@echo "Success: RPMs are in parent directory"

# Release

# .PHONY: gitbook
# gitbook: $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT)
# 	@perl -0777 -pi -e 's/(\$$ $(GEN) --help)(.|\n|\r)*?(```\n)/`echo "\$$ $(GEN) --help"; $(BINDIR)\/$(GEN) --help; echo "\\\`\\\`\\\`" `/e' gitbook/oidc-gen.md
# 	@perl -0777 -pi -e 's/(\$$ $(ADD) --help)(.|\n|\r)*?(```\n)/`echo "\$$ $(ADD) --help"; $(BINDIR)\/$(ADD) --help; echo "\\\`\\\`\\\`" `/e' gitbook/oidc-add.md
# 	@perl -0777 -pi -e 's/(\$$ $(AGENT) --help)(.|\n|\r)*?(```\n)/`echo "\$$ $(AGENT) --help"; $(BINDIR)\/$(AGENT) --help; echo "\\\`\\\`\\\`" `/e' gitbook/oidc-agent.md
# 	@perl -0777 -pi -e 's/(\$$ $(CLIENT) --help)(.|\n|\r)*?(```\n)/`echo "\$$ $(CLIENT) --help"; $(BINDIR)\/$(CLIENT) --help; echo "\\\`\\\`\\\`" `/e' gitbook/oidc-token.md
# 	@echo "Updated gitbook docu with help output"

# .PHONY: release
# release: deb gitbook

$(TESTBINDIR)/test: $(TESTBINDIR) $(TESTSRCDIR)/main.c $(TEST_SOURCES) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
	@$(CC) $(TEST_CFLAGS) $(TESTSRCDIR)/main.c $(TEST_SOURCES) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o) -o $@ $(TEST_LFLAGS)

.PHONY: test
test: $(TESTBINDIR)/test
	@$<


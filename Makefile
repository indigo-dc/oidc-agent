UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	MAC_OS = 1
endif
ifeq (, $(shell which dpkg-buildflags 2>/dev/null))
         NODPKG = 1
endif

# Where to store rpm source tarball
RPM_OUTDIR =rpm/rpmbuild/SOURCES

# Executable names
AGENT         = oidc-agent
GEN           = oidc-gen
ADD           = oidc-add
CLIENT        = oidc-token
KEYCHAIN      = oidc-keychain
AGENT_SERVICE = oidc-agent-service
PROMPT        = oidc-prompt

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
SRC_TAR   = oidc-agent-$(VERSION).tar.gz
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
SERVICECONFIG = oidc-agent-service.options

TESTSRCDIR = test/src
TESTBINDIR = test/bin

# USE_CJSON_SO ?= $(shell /sbin/ldconfig -N -v $(sed 's/:/ /g' <<< $LD_LIBRARY_PATH) 2>/dev/null | grep -i libcjson >/dev/null && echo 1 || echo 0)
USE_CJSON_SO ?= 0
USE_LIST_SO ?= $(shell /sbin/ldconfig -N -v $(sed 's/:/ /g' <<< $LD_LIBRARY_PATH) 2>/dev/null | grep -i liblist >/dev/null && echo 1 || echo 0)

ifeq ($(USE_CJSON_SO),1)
	DEFINE_USE_CJSON_SO = -DUSE_CJSON_SO
endif
ifeq ($(USE_LIST_SO),1)
	DEFINE_USE_LIST_SO = -DUSE_LIST_SO
endif

ifndef MAC_OS
	DIALOGTOOL ?= yad
else
	DIALOGTOOL ?= pashua
endif


LSODIUM = -lsodium
LARGP   = -largp
LMICROHTTPD = -lmicrohttpd
LCURL = -lcurl
LSECRET = -lsecret-1
LGLIB = -lglib-2.0
LLIST = -llist
LCJSON = -lcjson
LQR = -lqrencode
LAGENT = -l:$(SHARED_LIB_NAME_FULL)
ifdef MAC_OS
	LAGENT = -loidc-agent.$(LIBVERSION)
endif

# Compiler options
CC       := $(CC)
# compiling flags here
CFLAGS   := $(CFLAGS) -g -std=c99 -I$(SRCDIR) -I$(LIBDIR)  -Wall -Wextra -fno-common
ifndef MAC_OS
ifndef NODPKG
	CFLAGS   +=$(shell dpkg-buildflags --get CPPFLAGS)
	CFLAGS   +=$(shell dpkg-buildflags --get CFLAGS)
endif
	CFLAGS += $(shell pkg-config --cflags libsecret-1)
endif
TEST_CFLAGS = $(CFLAGS) -I.

# Linker options
LINKER   := $(CC)
ifdef MAC_OS
LFLAGS   = $(LSODIUM) $(LARGP)
else
LFLAGS   := $(LDFLAGS) $(LSODIUM) -fno-common -Wl,-z,now
ifndef NODPKG
LFLAGS +=$(shell dpkg-buildflags --get LDFLAGS)
endif
endif
ifeq ($(USE_CJSON_SO),1)
	LFLAGS += $(LCJSON)
endif
ifeq ($(USE_LIST_SO),1)
	LFLAGS += $(LLIST)
endif
AGENT_LFLAGS = $(LCURL) $(LMICROHTTPD) $(LQR) $(LFLAGS)
ifndef MAC_OS
	AGENT_LFLAGS += $(LSECRET) $(LGLIB)
endif
GEN_LFLAGS = $(LFLAGS) $(LMICROHTTPD) $(LQR)
ADD_LFLAGS = $(LFLAGS)
ifdef MAC_OS
CLIENT_LFLAGS = -L$(APILIB) $(LARGP) $(LAGENT) $(LSODIUM)
else
CLIENT_LFLAGS := $(LDFLAGS) -L$(APILIB) $(LAGENT) $(LSODIUM)
ifndef NODPKG
	CLIENT_LFLAGS += $(shell dpkg-buildflags --get LDFLAGS)
endif
endif
LIB_LFLAGS := $(LDFLAGS) -lc $(LSODIUM)
ifndef MAC_OS
ifndef NODPKG
	LIB_LFLAGS += $(shell dpkg-buildflags --get LDFLAGS)
endif
endif
ifeq ($(USE_CJSON_SO),1)
	CLIENT_LFLAGS += $(LCJSON)
	LIB_LFLAGS += $(LCJSON)
endif
ifeq ($(USE_LIST_SO),1)
	CLIENT_LFLAGS += $(LLIST)
	LIB_LFLAGS += $(LLIST)
endif

TEST_LFLAGS = $(LFLAGS) $(shell pkg-config --cflags --libs check)

# Install paths
ifndef MAC_OS
PREFIX                    ?=
BIN_PATH                  ?=$(PREFIX)/usr# /bin is appended later
BIN_AFTER_INST_PATH       ?=$(BIN_PATH)# needed for debian package and desktop file exec
PROMPT_BIN_PATH           ?=$(PREFIX)/usr# /bin is appended later
LIB_PATH                  ?=$(PREFIX)/usr/lib/x86_64-linux-gnu
LIBDEV_PATH               ?=$(PREFIX)/usr/lib/x86_64-linux-gnu
INCLUDE_PATH              ?=$(PREFIX)/usr/include/x86_64-linux-gnu
MAN_PATH                  ?=$(PREFIX)/usr/share/man
PROMPT_MAN_PATH           ?=$(PREFIX)/usr/share/man
CONFIG_PATH               ?=$(PREFIX)/etc
CONFIG_AFTER_INST_PATH    ?=$(CONFIG_PATH)
BASH_COMPLETION_PATH      ?=$(PREFIX)/usr/share/bash-completion/completions
DESKTOP_APPLICATION_PATH  ?=$(PREFIX)/usr/share/applications
XSESSION_PATH             ?=$(PREFIX)/etc/X11
else
PREFIX                    ?=/usr/local
BIN_PATH                  ?=$(PREFIX)# /bin is appended later
BIN_AFTER_INST_PATH       ?=$(BIN_PATH)# needed for debian package and desktop file exec
PROMPT_BIN_PATH           ?=$(PREFIX)# /bin is appended later
LIB_PATH                  ?=$(PREFIX)/lib
LIBDEV_PATH               ?=$(PREFIX)/lib
INCLUDE_PATH              ?=$(PREFIX)/include
MAN_PATH                  ?=$(PREFIX)/share/man
PROMPT_MAN_PATH           ?=$(PREFIX)/share/man
CONFIG_PATH               ?=$(PREFIX)/etc
CONFIG_AFTER_INST_PATH    ?=$(CONFIG_PATH)
endif

# Define sources
SRC_SOURCES := $(sort $(shell find $(SRCDIR) -name "*.c"))
ifneq ($(USE_CJSON_SO),1)
	LIB_SOURCES += $(LIBDIR)/cJSON/cJSON.c
endif
ifneq ($(USE_LIST_SO),1)
	LIB_SOURCES += $(LIBDIR)/list/list.c $(LIBDIR)/list/list_iterator.c $(LIBDIR)/list/list_node.c
endif
SOURCES  := $(SRC_SOURCES) $(LIB_SOURCES)

GENERAL_SOURCES := $(sort $(shell find $(SRCDIR)/utils -name "*.c") $(shell find $(SRCDIR)/account -name "*.c") $(shell find $(SRCDIR)/ipc -name "*.c") $(shell find $(SRCDIR)/defines -name "*.c"))
AGENT_SOURCES_TMP := $(sort $(shell find $(SRCDIR)/$(AGENT) -name "*.c"))
ifdef MAC_OS
	AGENT_SOURCES= $(filter-out $(SRCDIR)/$(AGENT)/oidcp/passwords/keyring.c, $(AGENT_SOURCES_TMP))
else
	AGENT_SOURCES = $(AGENT_SOURCES_TMP)
endif
GEN_SOURCES := $(sort $(shell find $(SRCDIR)/$(GEN) -name "*.c"))
ADD_SOURCES := $(sort $(shell find $(SRCDIR)/$(ADD) -name "*.c"))
CLIENT_SOURCES := $(sort $(filter-out $(SRCDIR)/$(CLIENT)/api.c $(SRCDIR)/$(CLIENT)/parse.c, $(shell find $(SRCDIR)/$(CLIENT) -name "*.c")))
KEYCHAIN_SOURCES := $(SRCDIR)/$(KEYCHAIN)/$(KEYCHAIN)
TEST_SOURCES :=  $(sort $(filter-out $(TESTSRCDIR)/main.c, $(shell find $(TESTSRCDIR) -name "*.c")))
PROMPT_SRCDIR := $(SRCDIR)/$(PROMPT)
AGENTSERVICE_SRCDIR := $(SRCDIR)/$(AGENT_SERVICE)

# Define objects
ALL_OBJECTS  := $(SRC_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
AGENT_OBJECTS  := $(AGENT_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o) $(OBJDIR)/$(GEN)/qr.o
GEN_OBJECTS  := $(GEN_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(OBJDIR)/oidc-agent/httpserver/termHttpserver.o $(OBJDIR)/oidc-agent/httpserver/running_server.o $(OBJDIR)/oidc-agent/oidc/device_code.o $(OBJDIR)/$(CLIENT)/parse.o $(OBJDIR)/$(CLIENT)/api.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
ADD_OBJECTS  := $(ADD_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
API_OBJECTS := $(OBJDIR)/$(CLIENT)/api.o $(OBJDIR)/$(CLIENT)/parse.o $(OBJDIR)/ipc/ipc.o $(OBJDIR)/ipc/cryptCommunicator.o $(OBJDIR)/ipc/cryptIpc.o $(OBJDIR)/utils/crypt/crypt.o $(OBJDIR)/utils/crypt/ipcCryptUtils.o $(OBJDIR)/utils/json.o $(OBJDIR)/utils/oidc_error.o $(OBJDIR)/utils/memory.o $(OBJDIR)/utils/string/stringUtils.o $(OBJDIR)/utils/colors.o $(OBJDIR)/utils/printer.o $(OBJDIR)/utils/ipUtils.o $(OBJDIR)/utils/listUtils.o $(OBJDIR)/utils/logger.o $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
ifdef MAC_OS
	API_OBJECTS += $(OBJDIR)/utils/file_io/oidc_file_io.o $(OBJDIR)/utils/file_io/file_io.o
endif
PIC_OBJECTS := $(API_OBJECTS:$(OBJDIR)/%=$(PICOBJDIR)/%)
CLIENT_OBJECTS := $(CLIENT_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(API_OBJECTS) $(OBJDIR)/utils/disableTracing.o
ifndef MAC_OS
	CLIENT_OBJECTS += $(OBJDIR)/utils/file_io/oidc_file_io.o $(OBJDIR)/utils/file_io/file_io.o
endif

rm       = rm -f

# RULES

.PHONY: all
all: build man


-include docker/docker.mk


# Compiling

.PHONY: build
build: create_obj_dir_structure $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT) $(BINDIR)/$(AGENT_SERVICE) $(BINDIR)/$(KEYCHAIN) $(BINDIR)/$(PROMPT)

## pull in dependency info for *existing* .o files
-include $(ALL_OBJECTS:.o=.d)

## Compile and generate depencency info
$(OBJDIR)/$(CLIENT)/$(CLIENT).o : $(APILIB)/$(SHARED_LIB_NAME_FULL)
$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERSION=\"$(VERSION)\" -DCONFIG_PATH=\"$(CONFIG_AFTER_INST_PATH)\" $(DEFINE_USE_CJSON_SO) $(DEFINE_USE_LIST_SO)
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
	@$(CC) $(CFLAGS) -fpic -fvisibility=hidden -c $< -o $@ -DVERSION=\"$(VERSION)\" -DCONFIG_PATH=\"$(CONFIG_AFTER_INST_PATH)\"
	@echo "Compiled "$<" with pic successfully!"

$(PICOBJDIR)/%.o : $(LIBDIR)/%.c
	@$(CC) $(CFLAGS) -fpic -fvisibility=hidden -c $< -o $@
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

$(BINDIR)/$(KEYCHAIN): $(KEYCHAIN_SOURCES)
	@cat $(KEYCHAIN_SOURCES) >$@ && chmod 755 $@
	@echo "Building "$@" complete!"

$(BINDIR)/$(PROMPT): $(PROMPT_SRCDIR)/$(PROMPT)
	@sed -n '/OIDC_INCLUDE/!p;//q' $<  >$@
	@cat $(PROMPT_SRCDIR)/$(PROMPT)_$(DIALOGTOOL) >>$@
	@sed '1,/OIDC_INCLUDE/d' $< >>$@
	@chmod 755 $@
	@echo "Building "$@" complete!"

$(BINDIR)/$(AGENT_SERVICE): $(AGENTSERVICE_SRCDIR)/$(AGENT_SERVICE) $(AGENTSERVICE_SRCDIR)/options
	@sed -n '/OIDC_INCLUDE/!p;//q' $< | sed 's!/usr/bin/oidc-agent!$(BIN_AFTER_INST_PATH)/bin/$(AGENT)!'  >$@
	@sed 's!/etc/oidc-agent!$(CONFIG_AFTER_INST_PATH)/oidc-agent!' $(AGENTSERVICE_SRCDIR)/options >>$@
	@sed '1,/OIDC_INCLUDE/d' $< >>$@
	@chmod 755 $@
	@echo "Building "$@" complete!"

# Phony Installer

.PHONY: install
ifndef MAC_OS
install: install_bin install_man install_conf install_bash install_scheme_handler install_xsession_script
else
install: install_bin install_man install_conf install_scheme_handler
endif
	@echo "Installation complete!"

.PHONY: install_bin
install_bin: $(BIN_PATH)/bin/$(AGENT) $(BIN_PATH)/bin/$(GEN) $(BIN_PATH)/bin/$(ADD) $(BIN_PATH)/bin/$(CLIENT) $(BIN_PATH)/bin/$(KEYCHAIN) $(BIN_PATH)/bin/$(AGENT_SERVICE) $(PROMPT_BIN_PATH)/bin/$(PROMPT)
	@echo "Installed binaries"

.PHONY: install_conf
install_conf: $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG) $(CONFIG_PATH)/oidc-agent/$(PUBCLIENTSCONFIG) $(CONFIG_PATH)/oidc-agent/$(SERVICECONFIG)
	@echo "Installed config files"

.PHONY: install_bash
install_bash: $(BASH_COMPLETION_PATH)/$(AGENT) $(BASH_COMPLETION_PATH)/$(GEN) $(BASH_COMPLETION_PATH)/$(ADD) $(BASH_COMPLETION_PATH)/$(CLIENT) $(BASH_COMPLETION_PATH)/$(AGENT_SERVICE) $(BASH_COMPLETION_PATH)/$(KEYCHAIN)
	@echo "Installed bash completion"

.PHONY: install_man
install_man: $(MAN_PATH)/man1/$(AGENT).1 $(MAN_PATH)/man1/$(GEN).1 $(MAN_PATH)/man1/$(ADD).1 $(MAN_PATH)/man1/$(CLIENT).1 $(MAN_PATH)/man1/$(AGENT_SERVICE).1 $(MAN_PATH)/man1/$(KEYCHAIN).1 $(PROMPT_MAN_PATH)/man1/$(PROMPT).1
	@echo "Installed man pages!"

.PHONY: install_lib
install_lib: $(LIB_PATH)/$(SHARED_LIB_NAME_FULL) $(LIB_PATH)/$(SHARED_LIB_NAME_SO)
	@echo "Installed library"

.PHONY: install_lib-dev
install_lib-dev: $(LIB_PATH)/$(SHARED_LIB_NAME_FULL) $(LIB_PATH)/$(SHARED_LIB_NAME_SO) $(LIBDEV_PATH)/$(SHARED_LIB_NAME_SHORT) $(LIBDEV_PATH)/liboidc-agent.a $(INCLUDE_PATH)/oidc-agent/api.h $(INCLUDE_PATH)/oidc-agent/ipc_values.h $(INCLUDE_PATH)/oidc-agent/oidc_error.h $(INCLUDE_PATH)/oidc-agent/export_symbols.h
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

$(BIN_PATH)/bin/$(KEYCHAIN): $(BINDIR)/$(KEYCHAIN) $(BIN_PATH)/bin
	@install $< $@

$(BIN_PATH)/bin/$(AGENT_SERVICE): $(BINDIR)/$(AGENT_SERVICE) $(BIN_PATH)/bin
	@install $< $@

$(PROMPT_BIN_PATH)/bin/$(PROMPT): $(BINDIR)/$(PROMPT) $(PROMPT_BIN_PATH)/bin
	@install $< $@

## Config
$(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG): $(CONFDIR)/$(PROVIDERCONFIG) $(CONFIG_PATH)/oidc-agent
	@install -m 644 $< $@

$(CONFIG_PATH)/oidc-agent/$(PUBCLIENTSCONFIG): $(CONFDIR)/$(PUBCLIENTSCONFIG) $(CONFIG_PATH)/oidc-agent
	@install -m 644 $< $@

$(CONFIG_PATH)/oidc-agent/$(SERVICECONFIG): $(CONFDIR)/$(SERVICECONFIG) $(CONFIG_PATH)/oidc-agent
	@install -m 644 $< $@

## Bash completion
$(BASH_COMPLETION_PATH)/$(AGENT): $(CONFDIR)/bash-completion/oidc-agent $(BASH_COMPLETION_PATH)
	@install -m 644 $< $@

$(BASH_COMPLETION_PATH)/$(GEN): $(BASH_COMPLETION_PATH)
	@ln -s $(AGENT) $@

$(BASH_COMPLETION_PATH)/$(ADD): $(BASH_COMPLETION_PATH)
	@ln -s $(AGENT) $@

$(BASH_COMPLETION_PATH)/$(CLIENT): $(BASH_COMPLETION_PATH)
	@ln -s $(AGENT) $@

$(BASH_COMPLETION_PATH)/$(KEYCHAIN): $(BASH_COMPLETION_PATH)
	@ln -s $(AGENT) $@

$(BASH_COMPLETION_PATH)/$(AGENT_SERVICE): $(CONFDIR)/bash-completion/oidc-agent-service $(BASH_COMPLETION_PATH)
	@install -m 644 $< $@

## Man pages
$(MAN_PATH)/man1/$(AGENT).1: $(MANDIR)/$(AGENT).1 $(MAN_PATH)/man1
	@install $< $@
$(MAN_PATH)/man1/$(GEN).1: $(MANDIR)/$(GEN).1 $(MAN_PATH)/man1
	@install $< $@
$(MAN_PATH)/man1/$(ADD).1: $(MANDIR)/$(ADD).1 $(MAN_PATH)/man1
	@install $< $@
$(MAN_PATH)/man1/$(CLIENT).1: $(MANDIR)/$(CLIENT).1 $(MAN_PATH)/man1
	@install $< $@
$(MAN_PATH)/man1/$(AGENT_SERVICE).1: $(MANDIR)/$(AGENT_SERVICE).1 $(MAN_PATH)/man1
	@install $< $@
$(MAN_PATH)/man1/$(KEYCHAIN).1: $(MANDIR)/$(KEYCHAIN).1 $(MAN_PATH)/man1
	@install $< $@
$(PROMPT_MAN_PATH)/man1/$(PROMPT).1: $(MANDIR)/$(PROMPT).1 $(PROMPT_MAN_PATH)/man1
	@install $< $@


## Lib
$(LIB_PATH)/$(SHARED_LIB_NAME_FULL): $(APILIB)/$(SHARED_LIB_NAME_FULL) $(LIB_PATH)
	@install $< $@

$(LIB_PATH)/$(SHARED_LIB_NAME_SO): $(LIB_PATH)
	@ln -sf $(SHARED_LIB_NAME_FULL) $@

$(LIBDEV_PATH)/$(SHARED_LIB_NAME_SHORT): $(LIBDEV_PATH)
	@ln -sf $(SHARED_LIB_NAME_SO) $@

$(INCLUDE_PATH)/oidc-agent/api.h: $(SRCDIR)/$(CLIENT)/api.h $(INCLUDE_PATH)/oidc-agent
	@install $< $@

$(INCLUDE_PATH)/oidc-agent/ipc_values.h: $(SRCDIR)/defines/ipc_values.h $(INCLUDE_PATH)/oidc-agent
	@install $< $@

$(INCLUDE_PATH)/oidc-agent/oidc_error.h: $(SRCDIR)/utils/oidc_error.h $(INCLUDE_PATH)/oidc-agent
	@install $< $@

$(LIBDEV_PATH)/liboidc-agent.a: $(APILIB)/liboidc-agent.a $(LIBDEV_PATH)
	@install $< $@

$(INCLUDE_PATH)/oidc-agent/export_symbols.h: $(SRCDIR)/$(CLIENT)/export_symbols.h $(INCLUDE_PATH)/oidc-agent
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
purge: uninstall uninstall_conf
else
purge: uninstall uninstall_conf
endif

.PHONY: uninstall
ifndef MAC_OS
uninstall: uninstall_man uninstall_bin uninstall_bashcompletion uninstall_scheme_handler uninstall_xsession_script
else
uninstall: uninstall_man uninstall_bin uninstall_scheme_handler
endif

.PHONY: uninstall_bin
uninstall_bin:
	@$(rm) $(BIN_PATH)/bin/$(AGENT)
	@$(rm) $(BIN_PATH)/bin/$(GEN)
	@$(rm) $(BIN_PATH)/bin/$(ADD)
	@$(rm) $(BIN_PATH)/bin/$(CLIENT)
	@$(rm) $(BIN_PATH)/bin/$(KEYCHAIN)
	@$(rm) $(BIN_PATH)/bin/$(AGENT_SERVICE)
	@$(rm) $(PROMPT_BIN_PATH)/bin/$(PROMPT)
	@echo "Uninstalled binaries"

.PHONY: uninstall_man
uninstall_man:
	@$(rm) $(MAN_PATH)/man1/$(AGENT).1
	@$(rm) $(MAN_PATH)/man1/$(GEN).1
	@$(rm) $(MAN_PATH)/man1/$(ADD).1
	@$(rm) $(MAN_PATH)/man1/$(CLIENT).1
	@$(rm) $(MAN_PATH)/man1/$(AGENT_SERVICE).1
	@$(rm) $(MAN_PATH)/man1/$(KEYCHAIN).1
	@$(rm) $(PROMPT_MAN_PATH)/man1/$(PROMPT).1
	@echo "Uninstalled man pages!"

.PHONY: uninstall_conf
uninstall_conf:
	@$(rm) $(CONFIG_PATH)/oidc-agent/$(PROVIDERCONFIG)
	@$(rm) $(CONFIG_PATH)/oidc-agent/$(PUBCLIENTSCONFIG)
	@$(rm) $(CONFIG_PATH)/oidc-agent/$(SERVICECONFIG)
	@echo "Uninstalled config"

.PHONY: uninstall_bashcompletion
uninstall_bashcompletion:
	@$(rm) $(BASH_COMPLETION_PATH)/$(CLIENT)
	@$(rm) $(BASH_COMPLETION_PATH)/$(GEN)
	@$(rm) $(BASH_COMPLETION_PATH)/$(ADD)
	@$(rm) $(BASH_COMPLETION_PATH)/$(AGENT)
	@$(rm) $(BASH_COMPLETION_PATH)/$(AGENT_SERVICE)
	@$(rm) $(BASH_COMPLETION_PATH)/$(KEYCHAIN)
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

.PHONY: uninstall_xsession_script
uninstall_xsession_script:
	@$(rm) $(XSESSION_PATH)/Xsession.d/91oidc-agent
	@echo "Uninstalled xsession_script"

# Man pages

.PHONY: create_man
create_man: $(MANDIR)/$(AGENT).1 $(MANDIR)/$(GEN).1 $(MANDIR)/$(ADD).1  $(MANDIR)/$(CLIENT).1 $(MANDIR)/$(AGENT_SERVICE).1 $(MANDIR)/$(KEYCHAIN).1 $(MANDIR)/$(PROMPT).1
	@echo "Created man pages"

$(MANDIR)/$(AGENT).1: $(MANDIR) $(BINDIR)/$(AGENT) $(SRCDIR)/h2m/$(AGENT).h2m
	@help2man $(BINDIR)/$(AGENT) -o $(MANDIR)/$(AGENT).1 -s 1 -N -i $(SRCDIR)/h2m/$(AGENT).h2m

$(MANDIR)/$(GEN).1: $(MANDIR) $(BINDIR)/$(GEN) $(SRCDIR)/h2m/$(GEN).h2m
	@help2man $(BINDIR)/$(GEN) -o $(MANDIR)/$(GEN).1 -s 1 -N -i $(SRCDIR)/h2m/$(GEN).h2m

$(MANDIR)/$(ADD).1: $(MANDIR) $(BINDIR)/$(ADD) $(SRCDIR)/h2m/$(ADD).h2m
	@help2man $(BINDIR)/$(ADD) -o $(MANDIR)/$(ADD).1 -s 1 -N -i $(SRCDIR)/h2m/$(ADD).h2m

$(MANDIR)/$(CLIENT).1: $(MANDIR) $(BINDIR)/$(CLIENT) $(SRCDIR)/h2m/$(CLIENT).h2m $(LIB_PATH)/$(SHARED_LIB_NAME_SO) $(LIB_PATH)/$(SHARED_LIB_NAME_FULL)
	@export LD_LIBRARY_PATH=$(LIB_PATH):$$LD_LIBRARY_PATH && help2man $(BINDIR)/$(CLIENT) -o $(MANDIR)/$(CLIENT).1 -s 1 -N -i $(SRCDIR)/h2m/$(CLIENT).h2m

$(MANDIR)/$(AGENT_SERVICE).1: $(MANDIR) $(BINDIR)/$(AGENT_SERVICE) $(SRCDIR)/h2m/$(AGENT_SERVICE).h2m
	@help2man $(BINDIR)/$(AGENT_SERVICE) -o $(MANDIR)/$(AGENT_SERVICE).1 -s 1 -N -i $(SRCDIR)/h2m/$(AGENT_SERVICE).h2m --no-discard-stderr

$(MANDIR)/$(KEYCHAIN).1: $(MANDIR) $(BINDIR)/$(KEYCHAIN) $(SRCDIR)/h2m/$(KEYCHAIN).h2m
	@help2man $(BINDIR)/$(KEYCHAIN) -o $(MANDIR)/$(KEYCHAIN).1 -s 1 -N -i $(SRCDIR)/h2m/$(KEYCHAIN).h2m --no-discard-stderr

$(MANDIR)/$(PROMPT).1: $(MANDIR) $(BINDIR)/$(PROMPT) $(SRCDIR)/h2m/$(PROMPT).h2m
	@help2man $(BINDIR)/$(PROMPT) -o $(MANDIR)/$(PROMPT).1 -s 1 -N -i $(SRCDIR)/h2m/$(PROMPT).h2m --no-discard-stderr

# Library

$(APILIB)/liboidc-agent.a: $(APILIB) $(API_OBJECTS)
	@ar -crs $@ $(API_OBJECTS)

$(APILIB)/$(SHARED_LIB_NAME_FULL): create_picobj_dir_structure $(APILIB) $(PIC_OBJECTS)
ifdef MAC_OS
	@$(LINKER) -dynamiclib -fpic -Wl, -o $@ $(PIC_OBJECTS) $(LIB_LFLAGS)
else
	@$(LINKER) -shared -fpic -Wl,-z,defs,-soname,$(SONAME) -o $@ $(PIC_OBJECTS) $(LIB_LFLAGS)
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

ifneq ($(BIN_PATH), $(PROMPT_BIN_PATH))
$(PROMPT_BIN_PATH)/bin:
	@install -d $@
endif

$(CONFIG_PATH)/oidc-agent:
	@install -d $@

$(BASH_COMPLETION_PATH):
	@install -d $@

$(MAN_PATH)/man1:
	@install -d $@

ifneq ($(MAN_PATH), $(PROMPT_MAN_PATH))
$(PROMPT_MAN_PATH)/man1:
	@install -d $@
endif

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
.NOTPARALLEL: create_obj_dir_structure
create_obj_dir_structure: $(OBJDIR)
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;
	@cd $(LIBDIR) && find . -type d -exec mkdir -p -- ../$(OBJDIR)/{} \;

.PHONY: create_picobj_dir_structure
.NOTPARALLEL: create_picobj_dir_structure
create_picobj_dir_structure: $(PICOBJDIR)
	@cd $(SRCDIR) && find . -type d -exec mkdir -p -- ../$(PICOBJDIR)/{} \;
	@cd $(LIBDIR) && find . -type d -exec mkdir -p -- ../$(PICOBJDIR)/{} \;

# Cleaners

.PHONY: clean
clean: cleanobj cleanapi cleanpackage cleantest

.PHONY: cleanobj
cleanobj:
	@$(rm) -r $(OBJDIR)
	@$(rm) -r $(PICOBJDIR)

.PHONY: cleantest
cleantest:
	@$(rm) -r $(TESTBINDIR)

.PHONY: distclean
distclean: cleanobj clean
	@$(rm) -r $(BINDIR)
	@$(rm) -r $(MANDIR)

.PHONY: cleanapi
cleanapi:
	@$(rm) -r $(APILIB)

.PHONY: remove
remove: cleanobj cleanapi cleanpackage cleantest distclean

# Packaging

###################### RPM ###############################################

.PHONY: centos7_patch
centos7_patch:
	@mv src/utils/file_io/fileUtils.c src/utils/file_io/fileUtils.c.bck
	@cat src/utils/file_io/fileUtils.c.bck \
		| sed s/"define _DEFAULT_SOURCE 1"/"define _BSD_SOURCE"/ \
		> src/utils/file_io/fileUtils.c

.PHONY: rpmsource $(RPM_OUTDIR)/$(SRC_TAR)
rpmsource: $(RPM_OUTDIR)/$(SRC_TAR)
	test -e $(RPM_OUTDIR) || mkdir -p $(RPM_OUTDIR)
	@(cd ..; \
		tar czf $(SRC_TAR) \
			--exclude-vcs \
			--exclude=debian \
			--exclude=windows \
			--exclude=docker \
			--exclude=gitbook \
			--exclude=.pc \
			--exclude $(PGK_NAME)/config \
			--transform='s_${PKG_NAME}_${PKG_NAME}-$(VERSION)_' \
			$(PKG_NAME) \
		)
	mv ../$(SRC_TAR) $(RPM_OUTDIR)
	# Fix RPM source in spec file (it points to github for build systems
	# that build from only using the spec file
	@(cat rpm/oidc-agent.spec \
        | grep -v ^Source \
        > rpm/oidc-agent.spec.bckp)

	@(  grep -q "\#DO_NOT_REPLACE_THIS_LINE" rpm/oidc-agent.spec && {\
		VERSION=$(shell head debian/changelog -n 1|cut -d \( -f 2|cut -d \) -f 1|cut -d \- -f 1);\
		RELEASE=$(shell head debian/changelog -n 1|cut -d \( -f 2|cut -d \) -f 1|cut -d \- -f 2);\
		sed "s/\#DO_NOT_REPLACE_THIS_LINE/Source0: oidc-agent-${VERSION}.tar.gz/" -i rpm/oidc-agent.spec.bckp;\
		rm -f rpm/oidc-agent.spec;\
		mv rpm/oidc-agent.spec.bckp rpm/oidc-agent.spec;\
		}\
		|| true\
		)

.PHONY: rpms
rpms: srpm rpm 

.PHONY: rpm
rpm: rpmsource
	rpmbuild --define "_topdir ${PWD}/rpm/rpmbuild" -bb  rpm/${PKG_NAME}.spec

.PHONY: srpm
srpm: rpmsource
	rpmbuild --define "_topdir ${PWD}/rpm/rpmbuild" -bs  rpm/${PKG_NAME}.spec


$(TESTBINDIR)/test: $(TESTBINDIR) $(TESTSRCDIR)/main.c $(TEST_SOURCES) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o)
	@$(CC) $(TEST_CFLAGS) $(TESTSRCDIR)/main.c $(TEST_SOURCES) $(GENERAL_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(LIB_SOURCES:$(LIBDIR)/%.c=$(OBJDIR)/%.o) -o $@ $(TEST_LFLAGS)

.PHONY: test
test: $(TESTBINDIR)/test
	@$<

# .PHONY: testdocu
# testdocu: $(BINDIR)/$(AGENT) $(BINDIR)/$(GEN) $(BINDIR)/$(ADD) $(BINDIR)/$(CLIENT) gitbook/$(GEN).md gitbook/$(AGENT).md gitbook/$(ADD).md gitbook/$(CLIENT).md
# 	@$(BINDIR)/$(AGENT) -h | grep "^[[:space:]]*-" | grep -v "debug" | grep -v "verbose" | grep -v "usage" | grep -v "help" | grep -v "version" | sed 's/.*--/--/' | sed 's/\s.*$$//' | sed 's/=.*//' | sed 's/\[.*//' | xargs -I {} sh -c 'grep -c -- ^###.*{} gitbook/$(AGENT).md>/dev/null || echo "In gitbook/$(AGENT).md: {} not documented"'
# 	@$(BINDIR)/$(GEN) -h | grep "^[[:space:]]*-" | grep -v "debug" | grep -v "verbose" | grep -v "usage" | grep -v "help" | grep -v "version" | sed 's/.*--/--/' | sed 's/\s.*$$//' | sed 's/=.*//' | sed 's/\[.*//' | xargs -I {} sh -c 'grep -c -- ^###.*{} gitbook/$(GEN).md>/dev/null || echo "In gitbook/$(GEN).md: {} not documented"'
# 	@$(BINDIR)/$(ADD) -h | grep "^[[:space:]]*-" | grep -v "debug" | grep -v "verbose" | grep -v "usage" | grep -v "help" | grep -v "version" | sed 's/.*--/--/' | sed 's/\s.*$$//' | sed 's/=.*//' | sed 's/\[.*//' | xargs -I {} sh -c 'grep -c -- ^###.*{} gitbook/$(ADD).md>/dev/null || echo "In gitbook/$(ADD).md: {} not documented"'
# 	@$(BINDIR)/$(CLIENT) -h | grep "^[[:space:]]*-" | grep -v "debug" | grep -v "verbose" | grep -v "usage" | grep -v "help" | grep -v "version" | sed 's/.*--/--/' | sed 's/\s.*$$//' | sed 's/=.*//' | sed 's/\[.*//' | xargs -I {} sh -c 'grep -c -- ^###.*{} gitbook/$(CLIENT).md>/dev/null || echo "In gitbook/$(CLIENT).md: {} not documented"'

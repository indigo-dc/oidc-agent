SRC_TAR=oidc-agent-4.1.1.tar.gz
PKG_NAME  = oidc-agent
outdir ?= /tmp
srpm:
	@echo "cwd: $(shell pwd)"
	@echo "ls: $(shell ls -la|sed s/$$/\\\\n/g)"
	@echo "ls..: $(shell ls .. -la|sed s/$$/\\\\n/g)"

	#mkdir -p $(outdir)
	@echo "cd ../..; tar czf $(SRC_TAR) --exclude-vcs --exclude=.pc --exclude $(PGK_NAME)/config --transform='s_${PKG_NAME}_${PKG_NAME}-$(VERSION)_' $(PKG_NAME) "
	@(cd ../..; \
		tar czf $(SRC_TAR) \
			--exclude-vcs \
			--exclude=.pc \
			--exclude $(PGK_NAME)/config \
			--transform='s_${PKG_NAME}_${PKG_NAME}-$(VERSION)_' \
			$(PKG_NAME) \
		)
	mv ../../$(SRC_TAR) $(outdir)

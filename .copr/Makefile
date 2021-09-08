srpm:
	@echo "cwd: $(shell pwd)"
	@echo "ls: $(shell ls -la|sed s/$$/\\\\n/g)"
	@echo "OUTDIR: $(outdir)"
	(cd ..; pwd; make rpmsource outdir=$(outdir))

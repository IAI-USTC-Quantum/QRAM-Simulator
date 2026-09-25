# Minimal makefile for Sphinx documentation (bilingual: zh + en trees).
# Hand-trimmed after sphinx-apidoc generation:
#   make html     -> build both language trees into build/html/{zh,en}
#   make zh / en  -> build a single language tree
#   make livehtml-> live-reload build of both trees
#   make clean    -> remove build outputs
#
# The C++ API chapter needs the Doxygen XML first: run `doxygen Doxyfile`
# in the repository root before building (docs.yml CI does this for you).

SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build

.PHONY: help Makefile zh en livehtml clean

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)/zh" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

html:
	$(SPHINXBUILD) -M html "$(SOURCEDIR)/zh" "$(BUILDDIR)/html/zh" $(SPHINXOPTS) $(O)
	$(SPHINXBUILD) -M html "$(SOURCEDIR)/en" "$(BUILDDIR)/html/en" $(SPHINXOPTS) $(O)

zh:
	$(SPHINXBUILD) -M html "$(SOURCEDIR)/zh" "$(BUILDDIR)/html/zh" $(SPHINXOPTS) $(O)

en:
	$(SPHINXBUILD) -M html "$(SOURCEDIR)/en" "$(BUILDDIR)/html/en" $(SPHINXOPTS) $(O)

livehtml:
	sphinx-autobuild "$(SOURCEDIR)/zh" "$(BUILDDIR)/html/zh" $(SPHINXOPTS)
	sphinx-autobuild "$(SOURCEDIR)/en" "$(BUILDDIR)/html/en" $(SPHINXOPTS)

clean:
	rm -rf $(BUILDDIR)

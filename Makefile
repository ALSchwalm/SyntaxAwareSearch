LLVM_LDFLAGS := `llvm-config --ldflags --libs --system-libs`

CLANG_LIBS := \
	-Wl,--start-group \
	-lclangAST \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangEdit \
	-lclangASTMatchers \
	-lclangRewrite \
	-lclangRewriteFrontend \
	-lclangStaticAnalyzerFrontend \
	-lclangStaticAnalyzerCheckers \
	-lclangStaticAnalyzerCore \
	-lclangSerialization \
	-lclangToolingCore \
	-lclangTooling \
	-Wl,--end-group

BOOST_LIBS := -lboost_program_options -lboost_system -lboost_filesystem

all:
	clang++ -fpic sas.cpp parser.cpp search.cpp -g -o sas -std=c++14 \
	$(CLANG_LIBS) $(BOOST_LIBS) \
	$(LLVM_LDFLAGS)

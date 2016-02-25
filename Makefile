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
	clang++ -fpic src/sas.cpp src/parser.cpp src/search.cpp -g -o bin/sas -std=c++14 \
	-Iinclude \
	$(CLANG_LIBS) $(BOOST_LIBS) \
	$(LLVM_LDFLAGS)

test:
	clang++ -fpic tests/tests.cpp src/parser.cpp src/search.cpp -g -o bin/tests -std=c++14 \
	-Iinclude \
	$(CLANG_LIBS) $(BOOST_LIBS) \
	$(LLVM_LDFLAGS)

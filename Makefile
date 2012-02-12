all: compiler

OBJS = language.o \
        Message.o \
        TuringType.o \
        TypeManager.o \
        Scope.o \
        BasicScopes.o \
        ScopeManager.o \
	   ast.o  \
	   codegen.o \
       Turing.g.d_parser.o \
        StdLib.o \
        Executor.o \
       main.o

LLVM_MODULES = core jit native interpreter

CPPFLAGS = `/usr/local/bin/llvm-config --cppflags $(LLVM_MODULES)`
LDFLAGS = -L/usr/local/lib -ldparse `/usr/local/bin/llvm-config --ldflags $(LLVM_MODULES)`
LIBS = `/usr/local/bin/llvm-config --libs $(LLVM_MODULES)`

clean:
	$(RM) -rf Turing.g.d_parser.cpp $(OBJS)

Turing.g.d_parser.cpp: Turing.g
	/usr/local/bin/make_dparser $^ -v -Xcpp

%.o: %.cpp tokens.def
	clang++ -g -c $(CPPFLAGS) -o $@ $<


compiler: $(OBJS)
	clang++ -g -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

test: compiler
	./compiler testcompile.txt
debug: compiler
	gdb compiler
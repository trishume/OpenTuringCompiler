all: compiler

OBJS = language.o \
        Message.o \
        TypeManager.o \
	   ast.o  \
	   codegen.o \
       Turing.g.d_parser.o \
       main.o

LLVM_MODULES = core jit native

CPPFLAGS = `/usr/local/bin/llvm-config --cppflags $(LLVM_MODULES)`
LDFLAGS = -L/usr/local/lib -ldparse `/usr/local/bin/llvm-config --ldflags $(LLVM_MODULES)`
LIBS = `/usr/local/bin/llvm-config --libs $(LLVM_MODULES)`

clean:
	$(RM) -rf Turing.g.d_parser.cpp $(OBJS)

Turing.g.d_parser.cpp: Turing.g
	make_dparser $^ -v -Xcpp

%.o: %.cpp tokens.def
	g++ -g -c $(CPPFLAGS) -o $@ $<


compiler: $(OBJS)
	g++ -g -o $@ $(LDFLAGS) $(OBJS) $(LIBS)

test: compiler
	./compiler testcompile.txt
debug: compiler
	gdb compiler
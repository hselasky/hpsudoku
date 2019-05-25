PROG_CXX=hpsudoku
LDADD=
MAN=
SRCS= \
	hpsudoku.cpp \
	hpsudoku_elem.cpp \
	hpsudoku_head.cpp
BINDIR?=/usr/local/bin

.include <bsd.prog.mk>

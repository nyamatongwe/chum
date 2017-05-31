INCLUDEDIRS=-I../scintilla/include

chum: chum.cc ../scintilla/bin/scintilla.a
	g++ $^ -o $@ -D GTK=1 $(INCLUDEDIRS) `pkg-config gtkmm-3.0 gmodule-no-export-2.0 --cflags --libs`

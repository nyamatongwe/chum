// Chum, a small example of using Scintilla from a gtkmm 3 application.

#include <stdio.h>

#include <gtkmm.h>

#include <Scintilla.h>
#include <SciLexer.h>
#include <ScintillaWidget.h>

class ScintillaWindow {
	ScintillaObject *sci;
public:
	Gtk::Widget *editor;
	ScintillaWindow();
	~ScintillaWindow();
	int Call(unsigned int msg, uptr_t wParam=0, sptr_t lParam=0);
};

ScintillaWindow::ScintillaWindow() {
	editor = Glib::wrap(scintilla_new());
	sci = SCINTILLA(editor->gobj());
	scintilla_set_id(sci, 0);
}

ScintillaWindow::~ScintillaWindow() {
	delete editor;
}

int ScintillaWindow::Call(unsigned int msg, uptr_t wParam, sptr_t lParam) {
	return scintilla_send_message(sci, msg, wParam, lParam);
}

class ChumPort : public Gtk::Window {

public:
	ChumPort();
	virtual ~ChumPort() = default;

protected:
	//Signal handlers:
	void on_action_file_open();
	void on_action_file_find_selection();
	void on_action_file_wrap();
	void on_action_file_autocompletion();
	void on_action_file_calltip();
	void on_action_file_quit();

	void on_encoding_choices(int parameter);

	//Member widgets:
	Gtk::Box box;
	ScintillaWindow sw;

private:
	Glib::RefPtr<Gio::SimpleActionGroup> refActionGroup;
	Glib::RefPtr<Gio::SimpleAction> refChoiceEncoding;
	Glib::RefPtr<Gio::SimpleAction> refToggleWrap;

	std::string GetRange(int start, int end);
	int Call(unsigned int msg, uptr_t wParam=0, sptr_t lParam=0);
};

static const char *menu = R"(
<interface>
  <menu id='menubar'>
    <submenu>
    <attribute name='label' translatable='yes'>_File</attribute>
    <section>
      <item>
        <attribute name='label' translatable='yes'>_Open</attribute>
        <attribute name='action'>chum.open</attribute>
        <attribute name='accel'>&lt;Primary&gt;o</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name='label' translatable='yes'>_Find Selection</attribute>
        <attribute name='action'>chum.findselection</attribute>
        <attribute name='accel'>&lt;Primary&gt;f</attribute>
      </item>
      <item>
        <attribute name='label' translatable='yes'>_Wrap</attribute>
        <attribute name='action'>chum.wrap</attribute>
        <attribute name='accel'>&lt;Primary&gt;w</attribute>
      </item>
      <item>
        <attribute name='label' translatable='yes'>_Autocompletion</attribute>
        <attribute name='action'>chum.autocompletion</attribute>
        <attribute name='accel'>&lt;Primary&gt;a</attribute>
      </item>
      <item>
        <attribute name='label' translatable='yes'>_Calltip</attribute>
        <attribute name='action'>chum.calltip</attribute>
        <attribute name='accel'>&lt;Primary&gt;c</attribute>
      </item>
    </section>
    <section>
      <item>
        <attribute name='label' translatable='yes'>_Quit</attribute>
        <attribute name='action'>chum.quit</attribute>
        <attribute name='accel'>&lt;Primary&gt;q</attribute>
      </item>
    </section>
    </submenu>
    <submenu>
    <attribute name='label' translatable='yes'>_Encoding</attribute>
    <item>
      <attribute name='label' translatable='yes'>_UTF-8</attribute>
      <attribute name='action'>chum.choice</attribute>
      <attribute name='target' type='i'>0</attribute>
    </item>
    <item>
      <attribute name='label' translatable='yes'>_Latin 1</attribute>
      <attribute name='action'>chum.choice</attribute>
      <attribute name='target' type='i'>1</attribute>
    </item>
    <item>
      <attribute name='label' translatable='yes'>_ShiftJIS</attribute>
      <attribute name='action'>chum.choice</attribute>
      <attribute name='target' type='i'>2</attribute>
    </item>
    </submenu>
  </menu>
</interface>)";

ChumPort::ChumPort() : box(Gtk::ORIENTATION_VERTICAL) {
	add(box);
	box.show();

	refActionGroup = Gio::SimpleActionGroup::create();

	refActionGroup->add_action("open", sigc::mem_fun(*this, &ChumPort::on_action_file_open));
	refActionGroup->add_action("findselection", sigc::mem_fun(*this, &ChumPort::on_action_file_find_selection));
	refToggleWrap = refActionGroup->add_action_bool(
		"wrap", sigc::mem_fun(*this, &ChumPort::on_action_file_wrap), false);
	refActionGroup->add_action("autocompletion", sigc::mem_fun(*this, &ChumPort::on_action_file_autocompletion));
	refActionGroup->add_action("calltip", sigc::mem_fun(*this, &ChumPort::on_action_file_calltip));
	refActionGroup->add_action("quit", sigc::mem_fun(*this, &ChumPort::on_action_file_quit));

	refChoiceEncoding = refActionGroup->add_action_radio_integer("choice",
		sigc::mem_fun(*this, &ChumPort::on_encoding_choices), 0);
	
	insert_action_group("chum", refActionGroup);
	
	Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create();


	refBuilder->add_from_string(menu);

	Glib::RefPtr<Glib::Object> object = refBuilder->get_object("menubar");
	Glib::RefPtr<Gio::Menu> gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
	Gtk::MenuBar* pMenuBar = Gtk::manage(new Gtk::MenuBar(gmenu));
	pMenuBar->show();
	box.pack_start(*pMenuBar, Gtk::PACK_SHRINK);

	box.pack_start(*sw.editor);
	sw.editor->set_size_request(400, 300);
	sw.editor->show();

	const char *keywords =
		"and and_eq asm auto bitand bitor bool break "
		"case catch char class compl const const_cast continue "
		"default delete do double dynamic_cast else enum explicit export extern false float for "
		"friend goto if inline int long mutable namespace new not not_eq "
		"operator or or_eq private protected public "
		"register reinterpret_cast return short signed sizeof static static_cast struct switch "
		"template this throw true try typedef typeid typename union unsigned using "
		"virtual void volatile wchar_t while xor xor_eq";

	Call(SCI_SETCODEPAGE, SC_CP_UTF8);
	Call(SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
	Call(SCI_STYLECLEARALL, 0, 0);
	Call(SCI_SETMARGINWIDTHN, 0, 35);
	Call(SCI_SETSCROLLWIDTH, 200, 0);
	Call(SCI_SETSCROLLWIDTHTRACKING, 1, 0);
	Call(SCI_SETMULTIPLESELECTION, 1);
	Call(SCI_SETVIRTUALSPACEOPTIONS,
	     SCVS_RECTANGULARSELECTION | SCVS_USERACCESSIBLE);
	Call(SCI_SETADDITIONALSELECTIONTYPING, 1);
	Call(SCI_SETLEXER, SCLEX_CPP, 0);
	Call(SCI_SETKEYWORDS, 0, (sptr_t)keywords);
	Call(SCI_STYLESETFORE, SCE_C_COMMENT, 0x008000);
	Call(SCI_STYLESETFORE, SCE_C_COMMENTLINE, 0x008000);
	Call(SCI_STYLESETFORE, SCE_C_COMMENTDOC, 0x008040);
	Call(SCI_STYLESETITALIC, SCE_C_COMMENTDOC, 1);
	Call(SCI_STYLESETFORE, SCE_C_NUMBER, 0x808000);
	Call(SCI_STYLESETFORE, SCE_C_WORD, 0x800000);
	Call(SCI_STYLESETBOLD, SCE_C_WORD, 1);
	Call(SCI_STYLESETFORE, SCE_C_STRING, 0x800080);
	Call(SCI_STYLESETFORE, SCE_C_PREPROCESSOR, 0x008080);
	Call(SCI_STYLESETBOLD, SCE_C_OPERATOR, 1);
	Call(SCI_INSERTTEXT, 0, (sptr_t)
		"int main(int argc, char *argv[]) {\n"
		"    // Put the chum out\n"
		"    auto app = Gtk::Application::create(argc, argv, \"org.scintilla.chum\");\n"
		"    ChumPort chumPort;\n"
		"    return app->run(chumPort);\n"
		"}\n"
	);
	
}

void ChumPort::on_action_file_open() {
	Gtk::FileChooserDialog dialog("Please choose a file",
		Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(*this);

	//Add response buttons the the dialog:
	dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
	dialog.add_button("_Open", Gtk::RESPONSE_OK);
	
	const int result = dialog.run();
	if (result == Gtk::RESPONSE_OK) {
		Call(SCI_CLEARALL);
		const std::string filename = dialog.get_filename();
		const std::string contents = Glib::file_get_contents(filename);
		Call(SCI_ADDTEXT, contents.length(), reinterpret_cast<sptr_t>(contents.data()));
	}
}

void ChumPort::on_action_file_find_selection() {
	const int selStart = Call(SCI_GETSELECTIONSTART);
	const int selEnd = Call(SCI_GETSELECTIONEND);
	const int lenDoc = Call(SCI_GETLENGTH);

	const int indicatorHightlightCurrentWord = INDIC_CONTAINER;
	Call(SCI_INDICSETSTYLE, indicatorHightlightCurrentWord, INDIC_ROUNDBOX);
	Call(SCI_INDICSETFORE, indicatorHightlightCurrentWord, 0xff0000);
	Call(SCI_SETINDICATORCURRENT, indicatorHightlightCurrentWord);
	Call(SCI_INDICATORCLEARRANGE, 0, lenDoc);
	Call(SCI_INDICATORFILLRANGE, selStart, selEnd - selStart);

	const std::string wordToFind = GetRange(selStart, selEnd);
	Call(SCI_SETSEARCHFLAGS, 0);
	Call(SCI_SETTARGETSTART, selEnd);
	Call(SCI_SETTARGETEND, lenDoc);
	int indexOf = Call(SCI_SEARCHINTARGET,
			   wordToFind.length()-1, (sptr_t)wordToFind.data());
	if (indexOf < 0) {	// Wrap around
		Call(SCI_SETTARGETSTART, 0);
		Call(SCI_SETTARGETEND, selEnd);
		indexOf = Call(SCI_SEARCHINTARGET,
			       wordToFind.length()-1, (sptr_t)wordToFind.data());
	}
	if (indexOf >= 0) {
		Call(SCI_SETSELECTIONSTART, indexOf);
		Call(SCI_SETSELECTIONEND, indexOf+(selEnd - selStart));
	}
}

void ChumPort::on_action_file_wrap() {
	bool active = false;
	refToggleWrap->get_state(active);
	active = !active;
	refToggleWrap->change_state(active);
	Call(SCI_SETWRAPMODE, active ? SC_WRAP_WORD : SC_WRAP_NONE);
}

/* XPM */
static const char * arrow_xpm[] = {
"12 12 3 1",
" 	c None",
".	c #000000",
"+	c #808080",
"            ",
"     .+     ",
"      .+    ",
"      +.+   ",
" ........+  ",
" .........+ ",
" .........+ ",
" ........+  ",
"      +.+   ",
"      .+    ",
"     .+     ",
"            "};

/* XPM */
static const char * box_xpm[] = {
"12 12 2 1",
" 	c None",
".	c #000000",
"   .........",
"  .   .   ..",
" .   .   . .",
".........  .",
".   .   .  .",
".   .   . ..",
".   .   .. .",
".........  .",
".   .   .  .",
".   .   . . ",
".   .   ..  ",
".........   "};

void ChumPort::on_action_file_autocompletion() {
	const char *words = "Babylon-5?1 Battlestar-Galactica Millenium-Falcon?2 Moya?2 Serenity Voyager";
	Call(SCI_AUTOCSETIGNORECASE, 1);
	Call(SCI_REGISTERIMAGE, 1, (sptr_t)arrow_xpm);
	Call(SCI_REGISTERIMAGE, 2, (sptr_t)box_xpm);
	Call(SCI_AUTOCSHOW, 0, (sptr_t)words);
}

void ChumPort::on_action_file_calltip() {
	const char *calltip = "spaceship(int design, double capacity)";
	const int selStart = Call(SCI_GETSELECTIONSTART);
	Call(SCI_CALLTIPSHOW, selStart, (sptr_t)calltip);
}

void ChumPort::on_action_file_quit() {
	hide();
}

void ChumPort::on_encoding_choices(int parameter) {
	refChoiceEncoding->change_state(parameter);
	switch (parameter) {
		case 0:
			Call(SCI_SETCODEPAGE, SC_CP_UTF8);
			break;
		case 1:
			Call(SCI_SETCODEPAGE, 0);
			break;
		case 2:
			Call(SCI_SETCODEPAGE, 932);
			break;
	}
	const int charSet = (parameter==2) ? SC_CHARSET_SHIFTJIS : 0;
	for (int i=0; i<127;i++)
		Call(SCI_STYLESETCHARACTERSET, i, charSet);
}

std::string ChumPort::GetRange(int start, int end) {
	std::string bytes(end-start+1, 0);
	Sci_TextRange tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = &bytes[0];
	Call(SCI_GETTEXTRANGE, 0, (sptr_t)&tr);
	return bytes;
}

int ChumPort::Call(unsigned int msg, uptr_t wParam, sptr_t lParam) {
	return sw.Call(msg, wParam, lParam);
}

int main(int argc, char *argv[]) {
	auto app = Gtk::Application::create(argc, argv, "org.scintilla.chum");
	app->set_accel_for_action("chum.open", "<Primary>o");
	app->set_accel_for_action("chum.findselection", "<Primary>f");
	app->set_accel_for_action("chum.wrap", "<Primary>w");
	app->set_accel_for_action("chum.autocompletion", "<Primary>a");
	app->set_accel_for_action("chum.calltip", "<Primary>c");
	app->set_accel_for_action("chum.quit", "<Primary>q");
	ChumPort chumPort;
	return app->run(chumPort);
}

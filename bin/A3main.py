'''
    A3main.py
    frontend python code - old school terminal style.
    calls C functions in src folder and connects to MySQL database
'''
#!/usr/bin/env python3 - I can still do python 3
#REFER asciimatics github
from asciimatics.widgets import Frame, ListBox, Layout, Divider, Text, Button, TextBox, Widget, Label
from asciimatics.scene import Scene
from asciimatics.screen import Screen
from asciimatics.exceptions import ResizeScreenError, NextScene, StopApplication
import sys, os, datetime, ctypes
import mysql.connector
from mysql.connector import Error

#paths shared library initialization
BASE_DIR = os.path.dirname(os.path.realpath(__file__))
CARDS_DIR = os.path.join(BASE_DIR, "cards")
parserLib = ctypes.CDLL(os.path.join(BASE_DIR, "libvcparser.so"))

#ctypes c->python
CardPtr = ctypes.c_void_p
parserLib.createCard.argtypes = [ctypes.c_char_p, ctypes.POINTER(CardPtr)]; parserLib.createCard.restype = ctypes.c_int
parserLib.validateCard.argtypes = [CardPtr]; parserLib.validateCard.restype = ctypes.c_int
parserLib.writeCard.argtypes = [ctypes.c_char_p, CardPtr]; parserLib.writeCard.restype = ctypes.c_int
parserLib.cardToString.argtypes = [CardPtr]; parserLib.cardToString.restype = ctypes.c_char_p
parserLib.errorToString.argtypes = [ctypes.c_int]; parserLib.errorToString.restype = ctypes.c_char_p

#helper functions to call from vclibparser.so (C) and
def py_create_card(path): 
    card=CardPtr(None); err=parserLib.createCard(path.encode('utf-8'), ctypes.byref(card)); return (err, card)

def py_validate(card): 
    return parserLib.validateCard(card)

def py_write(path, card): 
    return parserLib.writeCard(path.encode('utf-8'), card)

def py_card_to_str(card): 
    if not card: 
        return ""
    raw = parserLib.cardToString(card)
    return raw.decode('utf-8', errors='replace') if raw else ""

def py_error_to_str(err): 
    raw = parserLib.errorToString(err)
    return raw.decode('utf-8', errors='replace') if raw else ""

#writes vCard file using only FN, BDAY, and ANNIVERSARY (really basic)
def py_rewrite_vcf(path, full_name, bday="", anniv=""):
    with open(path, "w", encoding="utf-8") as f:
        f.write("BEGIN:VCARD\r\nVERSION:4.0\r\nFN:" + full_name + "\r\n")
        if bday: f.write("BDAY:" + bday + "\r\n")
        if anniv: f.write("ANNIVERSARY:" + anniv + "\r\n")
        f.write("END:VCARD\r\n")

#parse the vCard string and returns FN, birthday and anniversary values
def py_parse_card(card_str):
    full_name = ""; bday = ""; anniv = ""
    for line in card_str.splitlines():
        line=line.strip(); uline=line.upper()
        if uline.startswith("FN:"):
            full_name = line[3:].strip()
        elif uline.startswith("BDAY:") or uline.startswith("BIRTHDAY:"):
            parts = line.split(":", 1); 
            if len(parts) > 1: 
                bday = parts[1].strip()
        elif uline.startswith("ANNIVERSARY:"):
            parts = line.split(":", 1); 
            if len(parts) > 1: 
                anniv = parts[1].strip()
    return full_name, bday, anniv

#count optional properties in the card string.
def py_count_optional(card_str):
    count = 0
    for line in card_str.splitlines():
        line=line.strip(); uline=line.upper()
        if uline.startswith("BEGIN:VCARD") or uline.startswith("VERSION:") or uline.startswith("FN:") or uline.startswith("BDAY:") or uline.startswith("BIRTHDAY:") or uline.startswith("ANNIVERSARY:") or uline.startswith("END:VCARD"):
            continue
        if line: count += 1
    return count

#convert (YYYYMMDD) into "YYYY-MM-DD 00:00:00" format; otherwise returns the string.
def convert_date(date_str):
    ds = date_str.strip()
    if not ds: 
        return None
    if len(ds)==8 and ds.isdigit():
        return f"{ds[0:4]}-{ds[4:6]}-{ds[6:8]} 00:00:00"
    return ds

#vcard
class VCardModel:
    def __init__(self):
        self.db = None; self.connected = False; self.cards = []; self.next_id = 1; self.current_id = None

    def connect(self, user, pwd, dbname):
        self.db = mysql.connector.connect(host="dursley.socs.uoguelph.ca", user=user, password=pwd, database=dbname)
        self.connected = True; self._create_tables()

    def _create_tables(self):
        if not self.connected or not self.db: 
            return
        cur = self.db.cursor()
        cur.execute("CREATE TABLE IF NOT EXISTS FILE(file_id INT AUTO_INCREMENT PRIMARY KEY, file_name VARCHAR(60) NOT NULL, last_modified DATETIME, creation_time DATETIME NOT NULL);")
        cur.execute("CREATE TABLE IF NOT EXISTS CONTACT(contact_id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(256) NOT NULL, birthday DATETIME, anniversary DATETIME, file_id INT NOT NULL, FOREIGN KEY(file_id) REFERENCES FILE(file_id) ON DELETE CASCADE);")
        self.db.commit()

    def load_cards(self):
        self.cards.clear()
        if not os.path.isdir(CARDS_DIR): 
            return
        for card_file in os.listdir(CARDS_DIR):
            if not card_file.lower().endswith(".vcf"): 
                continue
            path = os.path.join(CARDS_DIR, card_file)
            err, card_ptr = py_create_card(path)
            if err == 0 and card_ptr.value and py_validate(card_ptr)==0:
                py_write(path, card_ptr)
                card_str = py_card_to_str(card_ptr)
                full_name, bday, anniv = py_parse_card(card_str)
                opt_count = py_count_optional(card_str)
                card_dict = {"id": self.next_id, "file": card_file, "name": full_name, "bday": bday, "anniv": anniv, "opt": opt_count}
                self.cards.append(card_dict); self.next_id += 1
                if self.connected: self._db_upsert(card_dict, path)

    def _db_upsert(self, card_dict, path):
        cur = self.db.cursor()
        file_name = card_dict["file"]; name = card_dict["name"]
        bday_raw = card_dict["bday"]; anniv_raw = card_dict["anniv"]
        mtime = os.path.getmtime(path)
        mod_time = datetime.datetime.fromtimestamp(mtime).strftime('%Y-%m-%d %H:%M:%S')
        now_str = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        cur.execute("SELECT file_id FROM FILE WHERE file_name=%s", (file_name,))
        row = cur.fetchone()

        if row:
            file_id = row[0]
            cur.execute("UPDATE FILE SET last_modified=%s WHERE file_id=%s", (mod_time, file_id))

        else:
            cur.execute("INSERT INTO FILE(file_name, last_modified, creation_time) VALUES(%s, %s, %s)", (file_name, mod_time, now_str))
            file_id = cur.lastrowid
        bday_conv = convert_date(bday_raw); anniv_conv = convert_date(anniv_raw)
        cur.execute("SELECT contact_id FROM CONTACT WHERE file_id=%s", (file_id,))
        row = cur.fetchone()

        if row:
            contact_id = row[0]
            cur.execute("UPDATE CONTACT SET name=%s, birthday=%s, anniversary=%s WHERE contact_id=%s", (name, bday_conv, anniv_conv, contact_id))

        else:
            cur.execute("INSERT INTO CONTACT(name, birthday, anniversary, file_id) VALUES(%s, %s, %s, %s)", (name, bday_conv, anniv_conv, file_id))
        self.db.commit()

    def get_summary(self): 
        return [(card["file"], card["id"]) for card in self.cards]

    def get_card(self, cid):
        for card in self.cards:
            if card["id"]==cid: 
                return card
        return {"file": "", "name": "", "bday": "", "anniv": "", "opt": 0}


    def update_card(self, details):
        #update file name and contact name.
        new_name = details["name"].strip(); new_file = details["file"].strip()
        path = os.path.join(CARDS_DIR, new_file)
        if not new_file.lower().endswith(".vcf"): 
            return
        if self.current_id is None:
            if os.path.exists(path): 
                return
            py_rewrite_vcf(path, new_name)
            err, card_ptr = py_create_card(path)

            if err!=0 or not card_ptr.value or py_validate(card_ptr)!=0 or py_write(path, card_ptr)!=0: 
                return
            card_str = py_card_to_str(card_ptr)
            full_name, bday, anniv = py_parse_card(card_str)
            opt_count = py_count_optional(card_str)
            new_card = {"id": self.next_id, "file": new_file, "name": full_name, "bday": bday, "anniv": anniv, "opt": opt_count}
            self.cards.append(new_card); self.next_id += 1
            if self.connected: 
                self._db_upsert(new_card, path)
        else:
            existing = self.get_card(self.current_id)
            old_path = os.path.join(CARDS_DIR, existing["file"])


            #file name and FN change; keep birthday and anniversary as is.
            py_rewrite_vcf(old_path, new_name, existing["bday"], existing["anniv"])
            err, card_ptr = py_create_card(old_path)
            if err!=0 or not card_ptr.value or py_validate(card_ptr)!=0 or py_write(old_path, card_ptr)!=0: 
                return
            card_str = py_card_to_str(card_ptr)
            full_name, bday, anniv = py_parse_card(card_str)
            existing["name"] = full_name; existing["bday"] = bday; existing["anniv"] = anniv; existing["opt"] = py_count_optional(card_str)
            if self.connected: 
                self._db_upsert(existing, old_path)

#login view to enter DB credentials (phantom db???)
class LoginView(Frame):
    def __init__(self, scr, model):
        super(LoginView, self).__init__(scr, scr.height*2//3, scr.width*2//3, hover_focus=True, can_scroll=False, title="Login to DB")
        self.model = model; layout = Layout([100], fill_frame=True); self.add_layout(layout)
        layout.add_widget(Label("Enter DB credentials for dursley.socs.uoguelph.ca", align="^"))
        layout.add_widget(Text(label="Username:", name="username"))
        layout.add_widget(Text(label="Password:", name="password", hide_char="*"))
        layout.add_widget(Text(label="DB Name:", name="dbname"))
        layout.add_widget(Divider())
        layout2 = Layout([1,1,1]); self.add_layout(layout2)
        layout2.add_widget(Button("OK", self.ok), 0); layout2.add_widget(Button("Cancel", self.cancel), 2)
        self.fix()
    def ok(self):
        self.save(); user = self.data["username"].strip(); pwd = self.data["password"].strip(); dbname = self.data["dbname"].strip()
        if not user or not pwd or not dbname: 
            return
        try:
            self.model.connect(user, pwd, dbname)
            raise NextScene("Main")
        except Error:
            return
    def cancel(self): raise StopApplication("User cancelled login")

#main view: displays a list of valid vCard files.
class VCardListView(Frame):
    def __init__(self, scr, model):
        super(VCardListView, self).__init__(scr, scr.height*2//3, scr.width*2//3, on_load=self.reload_list, hover_focus=True, can_scroll=False, title="vCard List")
        self.model = model; self.list_box = ListBox(Widget.FILL_FRAME, [], name="card_list", add_scroll_bar=True, on_change=self.toggle_edit, on_select=self.edit)
        self.edit_btn = Button("Edit", self.edit); self.create_btn = Button("Create", self.create)
        self.db_btn = Button("DB queries", self.db_view); self.exit_btn = Button("Exit", self.exit_app)
        layout = Layout([100], fill_frame=True); self.add_layout(layout); layout.add_widget(self.list_box); layout.add_widget(Divider())
        layout2 = Layout([1,1,1,1]); self.add_layout(layout2)
        layout2.add_widget(self.create_btn, 0); layout2.add_widget(self.edit_btn, 1); layout2.add_widget(self.db_btn, 2); layout2.add_widget(self.exit_btn, 3)
        self.fix()
    def reload_list(self, new_val=None):
        self.model.load_cards(); self.list_box.options = self.model.get_summary(); self.list_box.value = new_val; self.toggle_edit()
    def toggle_edit(self): 
        self.edit_btn.disabled = (self.list_box.value is None)
    def create(self): 
        self.model.current_id = None; raise NextScene("DetailsView")
    def edit(self):
        self.save(); self.model.current_id = self.data["card_list"]
        if self.model.current_id is not None: raise NextScene("DetailsView")
    def db_view(self): 
        raise NextScene("DBQueriesView")
    def exit_app(self): 
        raise StopApplication("Exit pressed")

#Details view: only file name and contact name are editable.
#oh and allso displays birthday, anniversary (read-only) and the count of optional properties.
class VCardDetailsView(Frame):
    def __init__(self, scr, model):
        super(VCardDetailsView, self).__init__(scr, scr.height*2//3, scr.width*2//3, hover_focus=True, can_scroll=False, title="vCard Details")
        self.model = model; layout = Layout([100], fill_frame=True); self.add_layout(layout)
        self.file_field = Text("File name (.vcf):", name="file")
        self.name_field = Text("Contact Name (FN):", name="name")
        self.bday_field = Text("Birthday:", name="bday", disabled=True)
        self.anniv_field = Text("Anniversary:", name="anniv", disabled=True)
        self.opt_field = Text("Other Properties:", name="opt", disabled=True)
        layout.add_widget(self.file_field); layout.add_widget(self.name_field); layout.add_widget(self.bday_field)
        layout.add_widget(self.anniv_field); layout.add_widget(self.opt_field); layout.add_widget(Divider())
        layout2 = Layout([1,1]); self.add_layout(layout2)
        layout2.add_widget(Button("OK", self.ok), 0); layout2.add_widget(Button("Cancel", self.cancel), 1)
        self.fix()
    def reset(self):
        super(VCardDetailsView, self).reset()
        card = self.model.get_card(self.model.current_id)
        self.data = {"file": card.get("file", ""), "name": card.get("name", ""), "bday": card.get("bday", ""), "anniv": card.get("anniv", ""), "opt": str(card.get("opt", 0))}
    def ok(self):
        self.save(); self.model.update_card(self.data); raise NextScene("Main")
    def cancel(self): 
        raise NextScene("Main")

#DB Queries view.
class DBQueriesView(Frame):
    def __init__(self, scr, model):
        super(DBQueriesView, self).__init__(scr, scr.height*2//3, scr.width*2//3, hover_focus=True, can_scroll=False, title="DB Queries")
        self.model = model; layout = Layout([100], fill_frame=True); self.add_layout(layout)
        self.results_box = TextBox(Widget.FILL_FRAME, name="results", label="Results:", as_string=True, line_wrap=True)
        layout.add_widget(self.results_box); layout.add_widget(Divider())
        layout2 = Layout([1,1,1]); self.add_layout(layout2)
        layout2.add_widget(Button("Display all contacts", self.display_all), 0)
        layout2.add_widget(Button("Find contacts born in June", self.find_june), 1)
        layout2.add_widget(Button("Close", self.close_view), 2)
        self.fix()
    def display_all(self):
        if not self.model.connected or not self.model.db: 
            return
        cur = self.model.db.cursor(dictionary=True)
        sql = """SELECT C.contact_id, C.name, C.birthday, C.anniversary, F.file_name, F.last_modified, F.creation_time 
                 FROM CONTACT C JOIN FILE F ON C.file_id = F.file_id ORDER BY C.name;"""
        cur.execute(sql); rows = cur.fetchall()
        if rows:
            out = []
            for r in rows:
                out.append(f"ID={r['contact_id']} Name={r['name']}\nBday={r['birthday']} Anniv={r['anniversary']}\nFile={r['file_name']} LastMod={r['last_modified']}\n")
            self.results_box.value = "\n".join(out)
        self.show_db_stats()
    def find_june(self):
        if not self.model.connected: 
            return
        cur = self.model.db.cursor(dictionary=True)
        sql = """SELECT C.name, C.birthday, TIMESTAMPDIFF(YEAR, C.birthday, F.last_modified) AS age 
                 FROM CONTACT C JOIN FILE F ON C.file_id = F.file_id WHERE MONTH(C.birthday)=6 ORDER BY age;"""
        cur.execute(sql); rows = cur.fetchall()
        if rows:
            out = []
            for r in rows:
                out.append(f"Name={r['name']} Bday={r['birthday']} Age={r['age']}")
            self.results_box.value = "\n".join(out)
        self.show_db_stats()
    def show_db_stats(self):
        cur = self.model.db.cursor(); cur.execute("SELECT COUNT(*) FROM FILE"); n_files = cur.fetchone()[0]
        cur.execute("SELECT COUNT(*) FROM CONTACT"); n_contacts = cur.fetchone()[0]
        self.results_box.value += f"\n\nDatabase has {n_files} files and {n_contacts} contacts."
    def close_view(self): 
        raise NextScene("Main")

#et up scenes and runs the UI.
def demo(scr, scene):
    scenes = [Scene([LoginView(scr, model)], -1, name="Login"),
              Scene([VCardListView(scr, model)], -1, name="Main"),
              Scene([VCardDetailsView(scr, model)], -1, name="DetailsView"),
              Scene([DBQueriesView(scr, model)], -1, name="DBQueriesView")]
    scr.play(scenes, stop_on_resize=True, start_scene=scene, allow_int=True)

model = VCardModel()
last_scene = None
while True:
    try:
        Screen.wrapper(demo, catch_interrupt=True, arguments=[last_scene])
        sys.exit(0)
    except ResizeScreenError as e:
        last_scene = e.scene

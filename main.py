import time
import threading
import interception
import XInput
import ctypes
import sys
import json
import os
import customtkinter as ctk
from tkinter import messagebox
from pynput import keyboard

# --- CONFIGURAÇÕES DE ARQUIVO ---
CONFIG_FILE = "config_tibia.json"

class Config:
    RUNNING = False
    MOUSE_SENSITIVITY = 5
    DEADZONE_STICK = 0.4
    MOUSE_STICK = "RIGHT"
    # Estados para o loop de movimento (Heartbeat)
    MOVE_AXIS = {"w": 0, "s": 0, "a": 0, "d": 0}
    
    MAP = {
        'A': ['', 'F2'], 'B': ['', 'F3'], 'X': ['', 'F1'], 'Y': ['', 'F4'],
        'LB': ['', 'F6'], 'RB': ['', 'F5'], 'LT': ['SHIFT', ''], 'RT': ['CTRL', ''],
        'BACK': ['', 'MOUSE_RIGHT'], 'START': ['', 'ESCAPE'], 
        'UP': ['', 'F7'], 'DOWN': ['', 'F8'], 'LEFT': ['', 'F9'], 'RIGHT': ['', 'F10'],
        'L3': ['ALT', 'Q'], 'R3': ['', 'MOUSE_LEFT']
    }

    @staticmethod
    def save():
        try:
            data = {
                "mouse_sensitivity": Config.MOUSE_SENSITIVITY,
                "deadzone_stick": Config.DEADZONE_STICK,
                "mouse_stick": Config.MOUSE_STICK,
                "mapping": Config.MAP
            }
            with open(CONFIG_FILE, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=4)
            print(f"[LOG] Configurações salvas: Sensibilidade={Config.MOUSE_SENSITIVITY}, Mouse={Config.MOUSE_STICK}")
        except Exception as e:
            print(f"[ERRO] Falha ao salvar arquivo: {e}")

    @staticmethod
    def load():
        if os.path.exists(CONFIG_FILE):
            try:
                with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    Config.MOUSE_SENSITIVITY = data.get("mouse_sensitivity", 5)
                    Config.DEADZONE_STICK = data.get("deadzone_stick", 0.4)
                    Config.MOUSE_STICK = data.get("mouse_stick", "RIGHT")
                    Config.MAP.update(data.get("mapping", {}))
                print("[LOG] Perfil carregado com sucesso do disco.")
            except:
                print("[AVISO] Erro ao carregar arquivo. Usando valores padrão.")

class ConsoleRedirector:
    def __init__(self, textbox):
        self.textbox = textbox
    def write(self, text):
        self.textbox.configure(state="normal")
        self.textbox.insert("end", text)
        self.textbox.see("end")
        self.textbox.configure(state="disabled")
    def flush(self):
        pass

# --- LOOP DE MOVIMENTO (HEARTBEAT) ---
def movement_loop():
    while True:
        if Config.RUNNING:
            for key, active in Config.MOVE_AXIS.items():
                if active:
                    interception.key_down(key)
            time.sleep(0.03)
        else:
            time.sleep(0.5)

# --- MOTOR DE EXECUÇÃO ---
def execute_action(btn_name, action_list):
    if not Config.RUNNING: return
    mod, key = action_list[0].lower().strip(), action_list[1].lower().strip()
    if not mod and not key: return 

    print(f"[DEBUG] Ação disparada: {btn_name} -> {f'({mod}) + ' if mod else ''}{key}")
    
    if mod in ['alt', 'ctrl', 'shift']:
        interception.key_down(mod)
        time.sleep(0.01)

    if key == 'mouse_left':
        interception.mouse_down("left"); time.sleep(0.01); interception.mouse_up("left")
    elif key == 'mouse_right':
        interception.mouse_down("right"); time.sleep(0.01); interception.mouse_up("right")
    elif key == 'mouse_middle':
        interception.mouse_down("middle"); time.sleep(0.01); interception.mouse_up("middle")
    elif key != "":
        interception.key_down(key); time.sleep(0.02); interception.key_up(key)

    if mod in ['alt', 'ctrl', 'shift']:
        time.sleep(0.01); interception.key_up(mod)

# --- CAPTURA DE EVENTOS DO CONTROLE ---
class GamepadHandler(XInput.EventHandler):
    def process_button_event(self, event):
        if not Config.RUNNING: return
        btn_map_ids = {
            XInput.BUTTON_A: 'A', XInput.BUTTON_B: 'B', XInput.BUTTON_X: 'X', XInput.BUTTON_Y: 'Y',
            XInput.BUTTON_LEFT_SHOULDER: 'LB', XInput.BUTTON_RIGHT_SHOULDER: 'RB',
            XInput.BUTTON_BACK: 'BACK', XInput.BUTTON_START: 'START',
            XInput.BUTTON_DPAD_UP: 'UP', XInput.BUTTON_DPAD_DOWN: 'DOWN',
            XInput.BUTTON_DPAD_LEFT: 'LEFT', XInput.BUTTON_DPAD_RIGHT: 'RIGHT',
            XInput.BUTTON_LEFT_THUMB: 'L3', XInput.BUTTON_RIGHT_THUMB: 'R3'
        }
        if event.type == XInput.EVENT_BUTTON_PRESSED:
            btn_name = btn_map_ids.get(event.button_id)
            if btn_name in Config.MAP: execute_action(btn_name, Config.MAP[btn_name])

    def process_trigger_event(self, event):
        if not Config.RUNNING or event.value < 0.5: return
        btn_name = 'LT' if event.trigger == XInput.LEFT else 'RT'
        print(f"[LOG] Gatilho {btn_name} pressionado: Pressão={round(event.value, 2)}")
        if btn_name in Config.MAP: execute_action(btn_name, Config.MAP[btn_name])

    def process_stick_event(self, event):
        if not Config.RUNNING: return
        
        # Log de pressão para os analógicos
        if abs(event.x) > 0.2 or abs(event.y) > 0.2:
            lado = "ESQ" if event.stick == XInput.LEFT else "DIR"
            print(f"[STICK] {lado} -> X: {round(event.x, 2)} | Y: {round(event.y, 2)}")

        is_mouse = (Config.MOUSE_STICK == "LEFT" and event.stick == XInput.LEFT) or \
                   (Config.MOUSE_STICK == "RIGHT" and event.stick == XInput.RIGHT)
        
        if is_mouse:
            global mouse_stick_pos
            mouse_stick_pos = [event.x, event.y]
        else:
            Config.MOVE_AXIS["w"] = 1 if event.y >= Config.DEADZONE_STICK else 0
            Config.MOVE_AXIS["s"] = 1 if event.y <= -Config.DEADZONE_STICK else 0
            Config.MOVE_AXIS["a"] = 1 if event.x <= -Config.DEADZONE_STICK else 0
            Config.MOVE_AXIS["d"] = 1 if event.x >= Config.DEADZONE_STICK else 0
            
            if not Config.MOVE_AXIS["w"]: interception.key_up("w")
            if not Config.MOVE_AXIS["s"]: interception.key_up("s")
            if not Config.MOVE_AXIS["a"]: interception.key_up("a")
            if not Config.MOVE_AXIS["d"]: interception.key_up("d")

    def process_connection_event(self, event):
        status = "CONECTADO" if event.type == XInput.EVENT_CONNECTED else "DESCONECTADO"
        print(f"[LOG] Evento de Hardware: Controle {status}")

    def process_battery_event(self, event): pass

# --- INTERFACE GRÁFICA ---
class TibiaGamepadGUI(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("TibiaGamepad v8.1")
        self.geometry("680x750")
        
        self.capturing = False
        self.current_entry = None
        self.keyboard_listener = None
        self.entry_widgets = {}

        self.tabview = ctk.CTkTabview(self)
        self.tabview.pack(padx=20, pady=20, fill="both", expand=True)
        self.tab_general = self.tabview.add("Geral")
        self.tab_buttons = self.tabview.add("Mapeamento")
        self.tab_logs = self.tabview.add("Logs")
        
        self.setup_general()
        self.setup_buttons()
        self.setup_logs()

    def setup_general(self):
        ctk.CTkLabel(self.tab_general, text="Painel de Controle", font=("Arial", 22, "bold")).pack(pady=15)
        self.status_btn = ctk.CTkButton(self.tab_general, text="ATIVAR EMULADOR", fg_color="green", height=55, font=("Arial", 16, "bold"), command=self.toggle_status)
        self.status_btn.pack(pady=10, fill="x", padx=40)
        
        ctk.CTkLabel(self.tab_general, text="Analógico do Mouse:").pack(pady=(15, 5))
        self.stick_choice = ctk.CTkSegmentedButton(self.tab_general, values=["LEFT", "RIGHT"], command=lambda v: setattr(Config, 'MOUSE_STICK', v))
        self.stick_choice.set(Config.MOUSE_STICK); self.stick_choice.pack(pady=5)
        
        ctk.CTkLabel(self.tab_general, text="Velocidade do Mouse:").pack(pady=(15, 5))
        self.s_mouse = ctk.CTkSlider(self.tab_general, from_=1, to=20, command=lambda v: setattr(Config, 'MOUSE_SENSITIVITY', int(v)))
        self.s_mouse.set(Config.MOUSE_SENSITIVITY); self.s_mouse.pack(pady=5)
        
        ctk.CTkButton(self.tab_general, text="SALVAR PERFIL", fg_color="#1f538d", height=35, command=Config.save).pack(pady=35)

    def setup_buttons(self):
        scroll_frame = ctk.CTkScrollableFrame(self.tab_buttons, label_text="Tabela de Mapeamento")
        scroll_frame.pack(fill="both", expand=True, padx=10, pady=10)
        
        header = ctk.CTkFrame(scroll_frame, fg_color="transparent")
        header.pack(fill="x", pady=(0, 10))
        ctk.CTkLabel(header, text="BOTÃO", width=80, font=("Arial", 11, "bold"), text_color="gray").pack(side="left")
        ctk.CTkLabel(header, text="MOD (SHIFT/CTRL/ALT)", width=140, font=("Arial", 11, "bold"), text_color="gray").pack(side="left", padx=5)
        ctk.CTkLabel(header, text="AÇÃO (TECLA/MOUSE)", font=("Arial", 11, "bold"), text_color="gray").pack(side="left", expand=True)

        botoes = ['A', 'B', 'X', 'Y', 'LB', 'RB', 'LT', 'RT', 'BACK', 'START', 'UP', 'DOWN', 'LEFT', 'RIGHT', 'L3', 'R3']
        for btn in botoes:
            frame = ctk.CTkFrame(scroll_frame)
            frame.pack(fill="x", pady=2)
            ctk.CTkLabel(frame, text=btn, width=80, font=("Arial", 12, "bold")).pack(side="left")
            
            me = ctk.CTkEntry(frame, width=130, state="readonly", justify="center")
            me.insert(0, Config.MAP[btn][0].upper()); me.pack(side="left", padx=2)
            me.bind("<Button-1>", lambda e, b=btn, i=0, ent=me: self.handle_entry_click(b, i, ent, e))
            
            ke = ctk.CTkEntry(frame, width=160, state="readonly", justify="center")
            ke.insert(0, Config.MAP[btn][1].upper()); ke.pack(side="left", fill="x", expand=True, padx=2)
            ke.bind("<Button-1>", lambda e, b=btn, i=1, ent=ke: self.handle_entry_click(b, i, ent, e))
            ke.bind("<Button-2>", lambda e, b=btn, i=1, ent=ke: self.handle_entry_click(b, i, ent, e))
            ke.bind("<Button-3>", lambda e, b=btn, i=1, ent=ke: self.handle_entry_click(b, i, ent, e))
            
            ctk.CTkButton(frame, text="X", width=25, height=25, fg_color="#444", hover_color="red", command=lambda b=btn: self.clear_row(b)).pack(side="right", padx=5)
            self.entry_widgets[btn] = (me, ke)

    def clear_row(self, btn):
        me, ke = self.entry_widgets[btn]
        Config.MAP[btn] = ['', '']
        for ent in [me, ke]:
            ent.configure(state="normal"); ent.delete(0, 'end'); ent.configure(state="readonly", fg_color=["#F9F9FA", "#343638"])
        print(f"[LOG] Mapeamento do botão {btn} foi resetado.")

    def handle_entry_click(self, btn, index, entry_widget, event):
        if self.capturing and self.current_entry and self.current_entry[2] == entry_widget:
            if index == 1:
                mouse_map = {1: "MOUSE_LEFT", 2: "MOUSE_MIDDLE", 3: "MOUSE_RIGHT"}
                btn_mouse = mouse_map.get(event.num)
                if btn_mouse:
                    self.finish_capture(btn, index, entry_widget, btn_mouse)
                    return "break"
        self.start_capture(btn, index, entry_widget)

    def setup_logs(self):
        self.log_textbox = ctk.CTkTextbox(self.tab_logs, state="disabled", font=("Consolas", 12))
        self.log_textbox.pack(fill="both", expand=True, padx=10, pady=(10, 5))
        ctk.CTkButton(self.tab_logs, text="Limpar Console", command=self.clear_logs, height=25).pack(pady=5)
        sys.stdout = ConsoleRedirector(self.log_textbox)

    def clear_logs(self):
        self.log_textbox.configure(state="normal"); self.log_textbox.delete("1.0", "end"); self.log_textbox.configure(state="disabled")

    def stop_listener(self):
        if self.keyboard_listener: self.keyboard_listener.stop(); self.keyboard_listener = None

    def start_capture(self, btn, index, entry_widget):
        if self.capturing and self.current_entry:
            _, _, old_ent = self.current_entry
            old_ent.configure(fg_color=["#F9F9FA", "#343638"])
            self.stop_listener()
        self.capturing, self.current_entry = True, (btn, index, entry_widget)
        entry_widget.configure(state="readonly", fg_color="#8B0000") 
        self.keyboard_listener = keyboard.Listener(on_press=self.on_key_press)
        self.keyboard_listener.start()

    def on_key_press(self, key):
        btn, index, entry_widget = self.current_entry
        if key == keyboard.Key.backspace:
            self.after(0, lambda: self.finish_capture(btn, index, entry_widget, ""))
            return False
        try: key_name = key.char if key.char else str(key)
        except AttributeError: key_name = str(key).replace("Key.", "")
        if index == 0:
            if 'ctrl' in key_name: key_name = 'ctrl'
            elif 'alt' in key_name: key_name = 'alt'
            elif 'shift' in key_name: key_name = 'shift'
            else: key_name = Config.MAP[btn][index]
        self.after(0, lambda: self.finish_capture(btn, index, entry_widget, key_name))
        return False

    def finish_capture(self, btn, index, entry_widget, key_name):
        entry_widget.configure(state="normal"); entry_widget.delete(0, 'end'); entry_widget.insert(0, key_name.upper())
        entry_widget.configure(state="readonly", fg_color=["#F9F9FA", "#343638"])
        Config.MAP[btn][index] = key_name
        self.capturing = False; self.stop_listener()
        
        mod_atual = Config.MAP[btn][0].upper() or "Nenhum"
        key_atual = Config.MAP[btn][1].upper() or "Vazia"
        print(f"[EDIT] {btn} atualizado. Combo: ({mod_atual}) + {key_atual}")

    def toggle_status(self):
        if not Config.RUNNING:
            conn = XInput.get_connected()
            if not any(conn): messagebox.showerror("Erro de Hardware", "Controle não detectado!"); return
            XInput.GamepadThread(GamepadHandler(conn.index(True)))
            print("[LOG] Emulador ATIVADO. Analógicos e botões respondendo.")
        else:
            for k in ["w", "s", "a", "d"]: Config.MOVE_AXIS[k] = 0; interception.key_up(k)
            print("[LOG] Emulador DESATIVADO.")
        Config.RUNNING = not Config.RUNNING
        self.status_btn.configure(text="PARAR" if Config.RUNNING else "ATIVAR", fg_color="red" if Config.RUNNING else "green")

# --- INICIALIZAÇÃO ---
mouse_stick_pos = [0.0, 0.0]
if __name__ == "__main__":
    if not ctypes.windll.shell32.IsUserAnAdmin(): sys.exit()
    Config.load()
    def mouse_runner():
        while True:
            if Config.RUNNING:
                mx, my = mouse_stick_pos
                if abs(mx) > 0.15 or abs(my) > 0.15:
                    interception.move_relative(int(mx*Config.MOUSE_SENSITIVITY), int(-my*Config.MOUSE_SENSITIVITY))
            time.sleep(0.001)
    threading.Thread(target=mouse_runner, daemon=True).start()
    threading.Thread(target=movement_loop, daemon=True).start()
    TibiaGamepadGUI().mainloop()
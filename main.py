import time
import threading
import interception
import XInput
import ctypes

# --- CONFIGURAÇÕES DE SENSIBILIDADE ---
# Aumente para 5 ou 10 se ainda estiver lento. 
# Como agora usamos uma thread de 1ms, valores menores já são bem rápidos.
MOUSE_SENSITIVITY = 2  
DEADZONE = 0.15

# --- MAPEAMENTO DE TECLAS ---
KEY_F1, KEY_F2, KEY_F3, KEY_F4 = "f1", "f2", "f3", "f4"
KEY_F5, KEY_F6 = "f5", "f6"
KEY_W, KEY_A, KEY_S, KEY_D = "w", "a", "s", "d"
KEY_ALT = "alt"
KEY_Q = "q"
KEY_RIGHT, KEY_LCONTROL = "right", "ctrl"
MOUSE_RIGHT, MOUSE_LEFT = "right", "left"

# Estado Global do Analógico e Macros
right_stick_pos = [0.0, 0.0] # [x, y]
turbo_flags = {
    'dpad_up': False, 'dpad_down': False, 
    'dpad_left': False, 'dpad_right': False
}

# --- FUNÇÕES DE INJEÇÃO ---

def is_admin():
    try: return ctypes.windll.shell32.IsUserAnAdmin()
    except: return False

def press_combo(modifier, key, delay=0.02):
    """
    Executa uma combinação de duas teclas (ex: Alt+Q, Ctrl+F1).
    :param modifier: A tecla modificadora (ex: "alt", "ctrl", "shift")
    :param key: A tecla principal (ex: "q", "f1")
    :param delay: Tempo que as teclas ficam pressionadas
    """
    interception.key_down(modifier)
    time.sleep(0.001)
    interception.key_down(key)
    time.sleep(delay)
    interception.key_up(key)
    time.sleep(0.001)
    interception.key_up(modifier)

def press_key(key_str):
    interception.key_down(key_str)
    time.sleep(0.01) 
    interception.key_up(key_str)

def hold_key(key_str):
    interception.key_down(key_str)

def release_key(key_str):
    interception.key_up(key_str)

def click_mouse(button="left"):
    interception.mouse_down(button)
    time.sleep(0.01) 
    interception.mouse_up(button)

# --- THREAD DE MOVIMENTAÇÃO DO MOUSE (LISO) ---

def mouse_movement_loop():
    """
    Esta thread roda a 1000Hz. Ela olha para onde o analógico está
    apontando e move o mouse continuamente
    """
    while True:
        rx, ry = right_stick_pos
        
        # Verifica se o analógico saiu da zona morta
        if abs(rx) > DEADZONE or abs(ry) > DEADZONE:
            # Cálculo de velocidade
            dx = int(rx * MOUSE_SENSITIVITY)
            dy = int(-ry * MOUSE_SENSITIVITY) # Inverte Y
            
            interception.move_relative(dx, dy)
        
        # Espera 1ms para a próxima atualização (1000Hz)
        time.sleep(0.001)

# --- MACROS TURBO ---

def turbo_worker(flag_name, key_main):
    while turbo_flags[flag_name]:
        if flag_name == 'dpad_right':
            hold_key(KEY_LCONTROL)
            time.sleep(0.01)
            press_key(KEY_RIGHT)
            time.sleep(0.01)
            release_key(KEY_LCONTROL)
        else:
            hold_key(KEY_LCONTROL)
            hold_key(key_main)
            time.sleep(0.01)
            release_key(key_main)
            release_key(KEY_LCONTROL)
        time.sleep(0.01)

# --- HANDLER DO CONTROLE ---

class GamepadHandler(XInput.EventHandler):
    def process_button_event(self, event):
        if event.type == XInput.EVENT_BUTTON_PRESSED:
            if event.button_id == XInput.BUTTON_A: press_key(KEY_F2)
            elif event.button_id == XInput.BUTTON_B: press_key(KEY_F3)
            elif event.button_id == XInput.BUTTON_X: press_key(KEY_F1)
            elif event.button_id == XInput.BUTTON_Y: press_key(KEY_F4)
            elif event.button_id == XInput.BUTTON_LEFT_SHOULDER: press_key(KEY_F6)
            elif event.button_id == XInput.BUTTON_RIGHT_SHOULDER: press_key(KEY_F5)
            elif event.button_id == XInput.BUTTON_BACK: click_mouse(MOUSE_RIGHT)
            elif event.button_id == XInput.BUTTON_LEFT_THUMB: press_combo(KEY_ALT, KEY_Q)
            elif event.button_id == XInput.BUTTON_RIGHT_THUMB: click_mouse(MOUSE_LEFT)
            
            elif event.button_id == XInput.BUTTON_DPAD_UP:
                turbo_flags['dpad_up'] = True
                threading.Thread(target=turbo_worker, args=('dpad_up', KEY_W), daemon=True).start()
            elif event.button_id == XInput.BUTTON_DPAD_DOWN:
                turbo_flags['dpad_down'] = True
                threading.Thread(target=turbo_worker, args=('dpad_down', KEY_S), daemon=True).start()
            elif event.button_id == XInput.BUTTON_DPAD_LEFT:
                turbo_flags['dpad_left'] = True
                threading.Thread(target=turbo_worker, args=('dpad_left', KEY_A), daemon=True).start()
            elif event.button_id == XInput.BUTTON_DPAD_RIGHT:
                turbo_flags['dpad_right'] = True
                threading.Thread(target=turbo_worker, args=('dpad_right', KEY_RIGHT), daemon=True).start()

        elif event.type == XInput.EVENT_BUTTON_RELEASED:
            btn = event.button_id
            if btn == XInput.BUTTON_DPAD_UP: turbo_flags['dpad_up'] = False
            elif btn == XInput.BUTTON_DPAD_DOWN: turbo_flags['dpad_down'] = False
            elif btn == XInput.BUTTON_DPAD_LEFT: turbo_flags['dpad_left'] = False
            elif btn == XInput.BUTTON_DPAD_RIGHT: turbo_flags['dpad_right'] = False

    def process_stick_event(self, event):
        if event.stick == XInput.LEFT:
            if event.y > 0.4: hold_key(KEY_W)
            else: release_key(KEY_W)
            if event.y < -0.4: hold_key(KEY_S)
            else: release_key(KEY_S)
            if event.x < -0.4: hold_key(KEY_A)
            else: release_key(KEY_A)
            if event.x > 0.4: hold_key(KEY_D)
            else: release_key(KEY_D)

        elif event.stick == XInput.RIGHT:
            # Apenas atualiza o estado global. 
            # A thread 'mouse_movement_loop' fará o resto.
            right_stick_pos[0] = event.x
            right_stick_pos[1] = event.y

    def process_connection_event(self, event): pass
    def process_trigger_event(self, event): pass

# --- MAIN ---

def main():
    if not is_admin():
        print("ERRO: Execute o terminal como ADMINISTRADOR.")
        return
    print("TibiaGamepad Ativo!")
    
    connected = XInput.get_connected()
    if not any(connected):
        print("Erro: Controle não encontrado.")
        return
    
    # Inicia a thread de movimento do mouse antes do controle
    threading.Thread(target=mouse_movement_loop, daemon=True).start()
    
    handler = GamepadHandler(connected.index(True))
    XInput.GamepadThread(handler)
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nDesligando...")
    finally:
        for f in turbo_flags: turbo_flags[f] = False
        for k in [KEY_W, KEY_A, KEY_S, KEY_D, KEY_LCONTROL]: release_key(k)

if __name__ == '__main__':
    main()
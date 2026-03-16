# 🎮 TibiaGamepad v1.1

O **TibiaGamepad** é um emulador de entrada de baixo nível projetado especificamente para permitir que jogadores de Tibia utilizem controles de Xbox (ou compatíveis com XInput) para movimentação e execução de hotkeys. 

Diferente de emuladores comuns, ele utiliza o driver **Interception**, o que garante que as entradas sejam reconhecidas pelo jogo mesmo em modo BattleEye, simulando um teclado/mouse físico real.

---

## ✨ Funcionalidades Principais

* **Movimentação Heartbeat**: Lógica de repetição forçada (30ms) que garante que o personagem continue andando suavemente enquanto o analógico estiver pressionado.
* **Mapeamento Híbrido**: Suporte total a teclas de teclado, combinações com modificadores (Ctrl, Alt, Shift) e botões do mouse (Esquerdo, Direito e Central).
* **Captura Inteligente**: Clique no campo para capturar a tecla ou o botão do mouse instantaneamente.
* **Console de Logs Integrado**: Visualize em tempo real a pressão dos analógicos, gatilhos e o status das hotkeys disparadas.
* **Suporte a D-Pad**: As setas direcionais do controle podem ser mapeadas para hotkeys extras (anéis, amuletos, etc).
* **Persistência**: Salva automaticamente suas configurações em um arquivo config_tibia.json.

---

## 🛠️ Requisitos de Instalação

### 1. Driver Interception (Obrigatório)
Para que o código funcione, você deve instalar o driver que permite a emulação de hardware:
1. Baixe o Interception Driver no GitHub oficial.
2. Abra o terminal como Administrador na pasta "command line installer".
3. Execute: install-interception.exe /install.
4. REINICIE o seu computador.

### 2. Dependências do Python
Instale as bibliotecas necessárias via pip:
- pip install interception
- pip install XInput
- pip install customtkinter
- pip install pynput

---

## 🚀 Como Usar

1. **Executar como Administrador**: O script precisa de privilégios de administrador para interagir com o driver de baixo nível.
2. **Mapeamento**:
    * Clique na aba Mapeamento.
    * Clique uma vez no campo desejado (ele ficará vermelho).
    * Pressione uma tecla no teclado ou clique com o botão do mouse no campo para salvar.
    * Use Backspace para limpar um campo individual ou o botão X para limpar a linha.
3. **Ativação**:
    * Na aba Geral, clique em ATIVAR EMULADOR.
    * O status mudará para vermelho e o log confirma a ativação.

---

## 📖 Estrutura da Interface

| Aba | Descrição |
| :--- | :--- |
| Geral | Ativação do emulador, escolha do analógico do mouse e ajuste de sensibilidade. |
| Mapeamento | Configuração de todos os botões físicos para Hotkeys ou Cliques de Mouse. |
| Logs | Depuração em tempo real de movimentos, pressão de botões e erros. |

---

## 💻 Compilando para Executável (.exe)

Se você deseja gerar um arquivo único para rodar sem precisar do Python instalado, instale o PyInstaller (pip install pyinstaller) e execute o comando abaixo no terminal:

pyinstaller --noconsole --onefile --uac-admin --name "TibiaGamepad_v1.1" main.py

*O executável será gerado na pasta dist.*

---

## ⚠️ Aviso Legal
Este software é uma ferramenta de acessibilidade e conveniência. O uso de emuladores pode estar sujeito às regras dos termos de serviço do jogo. Use com responsabilidade.
# 🎮 TibiaGamepad

O **TibiaGamepad** é um emulador de entrada de baixo nível projetado especificamente para permitir que jogadores de Tibia utilizem controles de Xbox (ou compatíveis com XInput) para movimentação e execução de hotkeys. 

Diferente de emuladores comuns, ele utiliza o driver **Interception**, o que garante que as entradas sejam reconhecidas pelo jogo mesmo em modo BattleEye, simulando um teclado/mouse físico real.

---

## ✨ Funcionalidades Principais

* **Movimentação Heartbeat (Restaurada)**: Lógica de repetição forçada (30ms) que garante que o personagem continue andando suavemente e sem interrupções enquanto o analógico estiver pressionado.
* **Mapeamento Híbrido**: Suporte total a teclas de teclado, combinações com modificadores (Ctrl, Alt, Shift) e botões do mouse (Esquerdo, Direito e Central).
* **Captura Inteligente**: Clique no campo para capturar a tecla ou o botão do mouse instantaneamente. Use `Backspace` ou o botão `X` para limpar.
* **Auto-Admin**: O programa agora solicita automaticamente os privilégios de Administrador ao ser aberto, requisito obrigatório para o driver funcionar.
* **UI Auto-Sync**: O perfil salvo (`config_tibia.json`) é carregado e exibido automaticamente na interface assim que o programa é iniciado.
* **Console de Logs Otimizado**: Visualize em tempo real a pressão dos analógicos (com trava de 0.5s para não travar o emulador), gatilhos e o status das hotkeys disparadas.

---

## 🛠️ Requisitos de Instalação

### 1. Driver Interception (Obrigatório)
Para que o código funcione, você deve instalar o driver que permite a emulação de hardware de baixo nível:
1. Baixe o Interception Driver no GitHub oficial.
2. Abra o terminal como Administrador na pasta "command line installer".
3. Execute: `install-interception.exe /install`.
4. **REINICIE o seu computador.**

### 2. Dependências do Python
Instale as bibliotecas necessárias via pip:
- `pip install interception`
- `pip install XInput`
- `pip install customtkinter`
- `pip install pynput`

---

## 🚀 Como Usar

1. **Executar**: Inicie o `main.py` (ou o executável gerado). Ele pedirá permissão de Administrador automaticamente.
2. **Mapeamento**:
    * Clique na aba **Mapeamento**.
    * Clique uma vez no campo desejado (ele ficará vermelho indicando modo de escuta).
    * Pressione uma tecla no teclado ou clique com o botão do mouse (Esquerdo/Direito/Meio) no campo para salvar.
3. **Ativação**:
    * Na aba **Geral**, clique em **ATIVAR EMULADOR**.
    * O status mudará para vermelho e o log confirmará a ativação. O analógico e os botões já estarão funcionando no jogo.

---

## 📖 Estrutura da Interface

| Aba | Descrição |
| :--- | :--- |
| **Geral** | Ativação do emulador, escolha do analógico de movimento do mouse e ajuste de sensibilidade. |
| **Mapeamento** | Configuração de todos os botões físicos para Hotkeys ou Cliques de Mouse. |
| **Logs** | Depuração em tempo real de movimentos, pressão de botões, salvamentos e erros. |

---

## 💻 Compilando para Executável (.exe)

Se você deseja gerar um arquivo único para rodar sem precisar do Python aberto no terminal, instale o PyInstaller (`pip install pyinstaller`) e execute o comando abaixo:

```bash
pyinstaller --noconsole --onefile --uac-admin --name "TibiaGamepad_v1.1.7" main.py
```
---

## ⚠️ Aviso Legal
Este software é uma ferramenta de acessibilidade e conveniência. O uso de emuladores e macros pode estar sujeito às regras dos termos de serviço do jogo. Use com responsabilidade.

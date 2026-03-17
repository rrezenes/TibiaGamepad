# 🎮 TibiaGamepad (C++ Native)

O **TibiaGamepad** é um emulador de entrada de baixo nível projetado especificamente para permitir que jogadores de Tibia utilizem controles de Xbox (ou compatíveis com XInput) para movimentação e execução de hotkeys.

Feito em **C++ Nativo (Qt6)** para maior performance, estabilidade e menor pegada de memória.

Diferente de emuladores comuns, ele utiliza o driver **Interception**, o que garante que as entradas sejam reconhecidas pelo jogo, simulando um teclado/mouse físico real em nível de kernel.

---

## ✨ Funcionalidades Principais

*   **Movimentação Heartbeat**: Lógica de repetição forçada (30ms) que garante que o personagem continue andando suavemente e sem interrupções enquanto o analógico estiver pressionado.
*   **Mapeamento Híbrido**: Suporte total a teclas de teclado, combinações com modificadores (Ctrl, Alt, Shift) e botões do mouse (Esquerdo, Direito e Central).
*   **Captura Inteligente**: Clique no campo para capturar a tecla ou o botão do mouse instantaneamente.
*   **Passthrough de Baixo Nível**: Implementação de loop de processamento do Interception que evita o travamento do teclado/mouse enquanto o emulador está ativo.
*   **Reinicialização de Segurança**: Botão dedicado para resetar instâncias do driver em caso de anomalia.
*   **Auto-Admin**: Solicita automaticamente privilégios de Administrador, requisito obrigatório para o driver Interception.

---

## 🛠️ Requisitos e Compilação

### 1. Driver Interception (Obrigatório)
1. Instale o driver Interception via instalador oficial.
2. Execute: `install-interception.exe /install` como Administrador.
3. **REINICIE o computador.**

### 2. Compilação (Windows + Qt6)
Para compilar o projeto, use o PowerShell na pasta raiz:

```powershell
# 1. Configurar
cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.10.2\msvc2022_64"

# 2. Compilar
cmake --build build --config Release
```

O executável e todas as DLLs necessárias (Qt e Interception) serão gerados automaticamente em `build/Release/`.

---

## 🚀 Como Usar

1.  **Executar**: Inicie o `TibiaGamepad.exe` como Administrador.
2.  **Mapeamento**:
    *   Na aba **Mapeamento**, clique no campo que deseja configurar (ele ficará vermelho).
    *   Pressione a tecla ou botão do mouse desejado.
3.  **Ativação**:
    *   Na aba **Geral**, clique em **ATIVAR EMULADOR**.
    *   Use o analógico configurado para movimentar o mouse ou o personagem.

---

## 📖 Estrutura do Projeto

*   `src/`: Código fonte (`.cpp`).
*   `include/`: Cabeçalhos (`.h`).
*   `lib/`: Bibliotecas estáticas do Interceptor.
*   `interception_ext/`: SDK do Interception.
*   `config_tibia.json`: Arquivo de persistência de configurações.

---

## ⚠️ Aviso Legal
Este software é uma ferramenta de acessibilidade e conveniência. O uso de emuladores e macros pode estar sujeito às regras dos termos de serviço do jogo. Use com responsabilidade.

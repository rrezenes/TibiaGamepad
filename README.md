TibiaGamepad 🎮
===============

O **TibiaGamepad** é um emulador de comando de alto desempenho desenvolvido especificamente para o Tibia. Utiliza o driver **Interception** para emular entradas de teclado e mouse a nível de hardware (HID), permitindo uma experiência de jogo fluida, segura e altamente responsiva através de um comando de Xbox ou compatível.

🚀 Principais Funcionalidades
-----------------------------

*   **Thread de Movimento 1000Hz**: Uma thread dedicada que processa a entrada do analógico direito a cada 1ms, eliminando "saltos" e garantindo uma precisão de câmara absoluta.
    
*   **Emulação Low-Level**: Ao utilizar o driver Interception, os comandos são injetados diretamente na stack de entrada do Windows, sendo vistos pelo sistema como um hardware real.
    
*   **Macros Turbo Inteligentes**: O D-Pad está configurado com macros turbo (Repetição rápida de Direção + CTRL) para facilitar a movimentação em situações de combate.
    
*   **Métodos Genéricos de Combos**: Sistema robusto para execução de combinações de teclas (ex: Alt + Q) com tempos de pressão (delays) ajustáveis para garantir o registo no jogo.
    
*   **Segurança**: Inclui verificação automática de privilégios de Administrador, necessária para a comunicação com o driver.
    

🛠️ Pré-requisitos
------------------

### 1\. Driver Interception (Obrigatório)

Este projeto não funciona sem o driver Interception instalado no sistema.

1.  Transfira o driver do repositório oficial: [Interception Releases](https://www.google.com/search?q=https://github.com/oblitum/Interception/releases/latest).
    
2.  Extraia o conteúdo.
    
3.  Abra o **Prompt de Comando (CMD)** como **Administrador**.
    
4.  Navegue até à pasta command line installer dentro dos ficheiros extraídos.
    
5.  install-interception.exe /install
    
6.  **Reinicie o seu computador** para que o driver seja carregado.
    

### 2\. Ambiente Python

*   Python 3.10 ou superior.
    

📦 Instalação e Configuração
----------------------------

1.  git clone \[https://github.com/seu-usuario/TibiaGamepad.git\](https://github.com/seu-usuario/TibiaGamepad.git)cd TibiaGamepad
    
2.  pip install -r requirements.txt
    

🎮 Como Utilizar
----------------

1.  Ligue o seu controle no PC.
    
2.  Certifique-se de que o terminal (VS Code, CMD ou PowerShell) está aberto como **Administrador**.
    
3.  python main.py
    
4.  O terminal deverá exibir a mensagem TibiaGamepad Ativo!.
    

⌨️ Mapeamento de Controlos
--------------------------

| **Comando no Comando** | **Ação no Tibia** | **Detalhes Técnicos** |

| **Analógico Esquerdo** | Movimentação | Mapeado para W, A, S, D |

| **Analógico Direito** | Movimentação do mouse | Processamento relativo a 1000Hz |

| **L3 (Pressionar Esq.)** | Atalho Especial | Executa Alt + Q (via press\_combo) |

| **R3 (Pressionar Dir.)** | Clique Esquerdo | Injeção via Interception |

| **Botão Back (Select)** | Clique Direito | Injeção via Interception |

| **D-Pad (Direcionais)** | Macros Turbo | Direção + L-CTRL em loop |

| **Botões A, B, X, Y** | Hotkeys F1-F4 | Pressionar rápido |

| **Bumpers (L1/R1)** | Hotkeys F5-F6 | Pressionar rápido |

🔧 Personalização
-----------------

Para alterar a sensibilidade do mouse ou os mapeamentos, edite as constantes no topo do ficheiro main.py:

`# Sensibilidade do analógico direito  MOUSE_SENSITIVITY = 5`
`# Zona morta para evitar drift do analógico  DEADZONE = 0.15` 

⚠️ Resolução de Problemas
-------------------------

*   **O mouse não se move**: Verifique se instalou o driver Interception e se reiniciou o PC.
    
*   **Erro de Administrador**: O script requer acesso direto ao hardware; clique com o botão direito no seu terminal e escolha "Executar como Administrador".
    
*   **Comando não detetado**: Verifique se o comando é reconhecido pelo Windows como um dispositivo XInput (Xbox 360/One/Series).
    

📄 Licença
----------

Este projeto está licenciado sob a licença MIT - consulte o ficheiro [LICENSE](https://www.google.com/search?q=LICENSE) para mais detalhes.

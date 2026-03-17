---
description: Como compilar o TibiaGamepad (C++)
---

Para compilar o projeto, siga estas etapas no PowerShell dentro da pasta do projeto:

1.  **Configurar o CMake** (apontando para o seu diretório do Qt):
    ```powershell
    cmake -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.10.2\msvc2022_64"
    ```
    *(Nota: Se você estiver usando o Visual Studio 2019, troque para `-G "Visual Studio 16 2019"`, mas certifique-se de que a pasta no `CMAKE_PREFIX_PATH` corresponde à sua versão do MSVC).*

2.  **Compilar o executável**:
    ```powershell
    cmake --build build --config Release
    ```

3.  **Executar**:
    O executável será gerado em `build\Release\TibiaGamepad.exe`. **O CMake agora copia automaticamente todas as DLLs necessárias (Qt e Interceptor) para esta pasta**, então você pode rodar o programa diretamente de lá.

> [!IMPORTANT]
> Lembre-se de que o executável precisa de privilégios de **Administrador** para o driver Interception carregar corretamente.

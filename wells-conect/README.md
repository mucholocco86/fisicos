# WELLS CONECT — base experimental Qt 6 + scrcpy 5.0.1

Esta é a primeira base executável do novo aplicativo WELLS CONECT, isolada na branch `dev/wells-conect-qt6-scrcpy501`. O histórico aprovado do QtScrcpy permanece intacto.

## Implementado nesta base
- C++ 17, Qt 6 Widgets com identidade escura e vermelha.
- Janela principal com lista de dispositivos ADB, USB e conexão TCP/IP.
- Botões proporcionais ao texto (sem preenchimento vermelho desnecessário).
- Engrenagem abre janela de Configurações separada e não modal.
- Controle de bitrate, resolução, FPS, topo de janela, FPS visível e desligar tela.
- Console ADB na janela de configurações.
- Inicia scrcpy 5.0.1 **cliente e servidor oficiais do mesmo pacote** como processo independente. Compatível com áudio quando o scrcpy e o Android suportam.
- O espelhamento nesta base inicial é exibido na **janela nativa do scrcpy 5.0.1**, não ainda no painel com barra lateral criado pelo usuário.

## Ainda não implementado
- Janela de espelhamento idêntica ao modelo enviado (vídeo, barra com 12 botões, gamepad, teclado PT-BR, mute, captura e navegação integrados). O modelo segue sendo a especificação para a próxima implementação.
- Todas as opções originais do QtScrcpy nas abas de configurações (as abas ainda não implementadas são identificadas na interface).
- Botões sndcpy de legado (deliberadamente desativados, não simulados).
- Ajustes de áudio independentes da sessão.
- Validação física em Android 8/15 ou medição real de CPU e RAM.

**Não substituir a versão funcional anterior por esta alfa.**

### Arquitetura
QtScrcpy 4.2.1 permanece no repositório para consulta de código e funcionalidades. O novo `wells-conect/` usa Qt 6; o scrcpy 5.0.1 oficial é incluído sem trocar arbitrariamente apenas o scrcpy-server antigo. Essa estratégia reduz inicialmente o risco de incompatibilidade entre versões de protocolo. A futura integração do painel de espelhamento exigirá controlar a janela de vídeo e mapear seus comandos, sem reintroduzir ScrcpyKeyMapper.

### Fontes
- QtScrcpy: https://github.com/barry-ran/QtScrcpy
- scrcpy 5.0.1: https://github.com/Genymobile/scrcpy/releases/tag/v5.0.1

Preservar os avisos e licenças das dependências ao distribuir o pacote.

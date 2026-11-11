# WELLS CONECT — contrato de áudio nativo (scrcpy 5.0.1)

## Evidência do protótipo
O protótipo Python lê a checkbox antes de iniciar o processo scrcpy.exe e acrescenta `--no-audio` quando desmarcada. O próprio scrcpy.exe recebe, decodifica e reproduz áudio. O cliente QtScrcpy do Wells não é esse executável.

## Estado verificado no código
- `QtScrcpyCore/src/device/server/server.cpp`: argumento fixo `audio=false`.
- `Server::Server()`: recebe socket de vídeo (`VideoSocket`) e socket de controle; não possui tratamento de socket de áudio.
- `QtScrcpy/audio/audiooutput.cpp`: áudio Wells/sndcpy iniciado separadamente pelo usuário, depois da conexão.
- A checkbox `wellsNativeAudioCheck` foi incluída na interface mas permanece desabilitada até existir receptor nativo completo.

## Requisitos acordados
1. Desmarcada (padrão): sessão scrcpy sem áudio; sndcpy continua disponível para acionamento manual.
2. Marcada: próxima sessão de espelhamento solicita áudio nativo; a conexão ADB prévia não é impedimento.
3. Mudança durante uma sessão não altera retroativamente o protocolo já estabelecido.
4. Intertravamento do botão de áudio Wells depende do modo **efetivamente iniciado** na sessão, não apenas da checkbox.
5. Aparelho incompatível com áudio não deve perder a possibilidade de espelhamento sem áudio; testar tratamento de falha/recuperação.
6. Preservar versão estável e não alterar APK original sem validação de assinatura e instalação.

## Implementação ainda necessária — NÃO CONCLUÍDA
- Investigar a ordem e cabeçalhos dos sockets do protocolo 5.0.1 e ajustar tanto túnel reverse quanto forward.
- Criar receptor, demultiplexação e decodificação de codec de áudio compatível com scrcpy 5.0.1; reproduzir via Qt6Multimedia.
- Conectar preferência persistente ao parâmetro enviado na inicialização do servidor, com modo efetivo por sessão.
- Proteger o botão sndcpy contra uso simultâneo; não tocar no funcionamento interno do áudio Wells desnecessariamente.
- Compilar e testar em Windows com dispositivos compatíveis e incompatíveis.

**Regra de segurança:** não trocar `audio=false` por `audio=true` isoladamente. O cliente atual confundiria o novo socket de áudio com o de controle e poderia interromper o espelhamento.

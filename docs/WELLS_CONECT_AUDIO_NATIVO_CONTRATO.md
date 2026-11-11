# WELLS CONECT — estudo arquivado sobre áudio nativo

**Decisão do usuário (11/11/2026): NÃO IMPLEMENTAR áudio nativo no Wells Conect nesta fase.**

A análise anterior demonstrou que o QtScrcpy integrado não possui o receptor do terceiro socket de áudio do scrcpy 5.0.1. Alterar somente `audio=false` seria inseguro. O protótipo Python utiliza o executável scrcpy completo, cuja arquitetura é diferente.

## Escopo vigente
1. Corrigir o áudio **Wells/sndcpy** que conecta mas não reproduz no Windows.
2. Personalizar **somente o ícone** do `sndcpy.apk`, preservando a função, pacote e permissões; avaliar necessidade de assinatura e reinstalação.
3. Não alterar a inicialização do vídeo ou o protocolo QtScrcpy.
4. Não adicionar checkbox de áudio nativo: a opção experimental foi revertida.
5. Preservar a versão estável até validação física.

Este documento fica somente como registro histórico da investigação, **não como especificação de implementação pendente**.

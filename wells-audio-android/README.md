# Wells Audio — integração experimental

Branch: `feat/wells-audio-android-prototype`. **Não é a versão estável.**

## Implementado no código-fonte
- Interface Android escura com opções 128, 96 (padrão) e 32 kbps.
- Pedido explícito de autorização do Android para captura de reprodução.
- Serviço Android em primeiro plano usando `AudioPlaybackCapture` e `MediaProjection` (Android 10+).
- Transmissão experimental **PCM 48 kHz estéreo 16-bit, sem compressão**, via socket local `wells_audio`.
- `wells_audio.bat` instala/abre `com.wells.audio` e encaminha TCP 28200 via ADB.
- Receptor Qt6 do Wells Conect seleciona Wells Audio quando `wells-audio.apk` e `wells_audio.bat` estão presentes; sem eles mantém sndcpy.
- O receptor tenta reconectar enquanto o usuário concede permissão de captura.
- Workflow `wells-audio-package.yml` tenta construir um pacote Windows com APK e binário integrado.

## Limitações e riscos
- **32, 96 e 128 kbps ainda são apenas preferências de interface.** Não há codificação AAC/Opus. O PCM utiliza 1.536 kbps. O usuário deve ser informado disso.
- A permissão de captura precisa ser concedida no Android; o Windows não pode concedê-la automaticamente.
- Apps que proíbem captura de áudio não serão capturados.
- A comunicação ainda precisa de testes reais de USB e Wi-Fi, estabilidade, latência e reinício.
- O fluxo atual espera a conexão Android por ADB e não dispensa ADB no Wi-Fi.
- O novo APK não é substituto binário do `com.rom1v.sndcpy`.
- O ícone oficial é copiado da imagem `res/wells_connect_icon.png`.
- **Não substituir o pacote estável antes da validação.**

## Teste
1. Baixar o artefato `WELLS-CONECT-WELLS-AUDIO-EXPERIMENTAL` da execução bem-sucedida de `wells-audio-package.yml`.
2. Extrair em diretório separado da versão estável.
3. Conectar Android 10+ com depuração ADB e iniciar Wells Conect.
4. Ativar áudio. O Windows abre Wells Audio no celular; selecionar **Transmitir áudio** e autorizar a captura do Android.
5. Comparar reprodução de mídia permitida pelo Android por USB e Wi-Fi.
6. Se houver falha, preservar logs e não substituir o sistema estável.

## Próxima etapa
Implementar Opus/AAC com bitrates efetivos 32/96/128 kbps e decodificação correspondente no Windows. Não prometer essa funcionalidade antes de medir os bitrates reais.

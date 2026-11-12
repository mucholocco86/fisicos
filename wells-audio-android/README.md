# Wells Audio — protótipo Android

Branch experimental `feat/wells-audio-android-prototype`. Não modifica a versão estável do Wells Conect.

## Estado
- Tela Android escura com três opções: 128, 96 (padrão), 32 kbps.
- Alternância de transmissão e persistência local das preferências.
- **Ainda não transmite áudio.** A chave é uma preferência de interface, não ativa uma captura.
- **Ainda não é um APK testado.** Necessita compilar com Android SDK/Gradle e testar em aparelho real.
- Ícone oficial do Wells Conect ainda precisa ser convertido para recursos Android; não reutilizar o ícone antigo do QtScrcpy.

## Próximas etapas obrigatórias
1. Integrar os recursos de ícone oficial do Wells Conect.
2. Implementar captura Android via MediaProjection + AudioPlaybackCapture, com consentimento e serviço em primeiro plano conforme versão Android.
3. Escolher e validar codec e bitrate efetivo (Opus ou AAC), considerando suporte Android.
4. Criar protocolo de transporte e receptor Windows, buffers e telemetria.
5. Comparar USB e Wi-Fi com o sndcpy existente, incluindo latência e áudio contínuo.
6. Só habilitar a opção de transmissão após o receptor e a captura funcionarem; manter sndcpy como fallback.

Nota: Android pode impedir captura de áudio de apps específicos; PCM 48 kHz estéreo 16-bit corresponde a 1536 kbps antes de compressão.

# QtScrcpy — versão personalizada (projeto principal)

Este repositório foi **reiniciado a pedido do proprietário** em 7 de novembro de 2026. A versão anterior, WELLS Conect, encontra-se apenas em `archive/wells-conect-2026-11-07` e não faz parte da aplicação atual.

## Fontes originais importadas integralmente

| Fonte | Pasta | Versão/commit |
| --- | --- | --- |
| [QtScrcpy](https://github.com/barry-ran/QtScrcpy) | `upstream/QtScrcpy/` | v4.2.1 / `f90e2259ea6745c30fc242f501a0210771288f8a` |
| [QtScrcpyCore](https://github.com/barry-ran/QtScrcpyCore) | `upstream/QtScrcpy/QtScrcpy/QtScrcpyCore/` | `9e388b8aa48e4e1c2cfdef408ef00c4c6b45c921` (fixado pelo QtScrcpy 4.2.1) |
| [ScrcpyKeyMapper](https://github.com/w4po/ScrcpyKeyMapper) | `upstream/ScrcpyKeyMapper/` | v1.0.1 / `022870e2caf905046c8e7502d4053bb5f171c1e4` |
| [scrcpy](https://github.com/Genymobile/scrcpy) | `upstream/scrcpy/` | v5.0.1 / `a60891aea193d92e7e5c3942700eca63f9d19a5f` |

As pastas `upstream/` contêm o código-fonte original, sem substituições de módulos. Cada projeto mantém seus avisos e termos de licenciamento. Para reproduzir a importação, executar o workflow `vendor-upstreams.yml` manualmente. Não utilizar versões móveis de tags sem revisão.

**Importante:** scrcpy 5.0.1 e QtScrcpy 4.2.1 são bases distintas. Copiar o servidor mais recente não garante compatibilidade; ela precisa ser implementada e testada antes de distribuir o programa como funcional.

## Requisitos aprovados

- Preservar toda a funcionalidade original do QtScrcpy e do ScrcpyKeyMapper; não reaproveitar os módulos experimentais do WELLS Conect.
- Interface C++/Qt com tema preto/vermelho, traduzida para português brasileiro e com explicações ao posicionar o mouse.
- Inicialização pronta para uso: identificar aparelhos automaticamente; escolher aparelho e conectar; abrir outros aparelhos em sessões independentes.
- Configurações iniciais voltadas ao desempenho, deixando acessíveis todos os comandos avançados.
- Editor visual de mapeamento **dentro do aplicativo** em C++/Qt, sem navegador, com captura automática do aparelho, teclado/mouse visual, perfis por jogo, armazenamento e aplicação compatíveis com QtScrcpy.
- Painel de espelhamento com funções originais e botões adicionais de mapeamento e gamepad; gamepad conectado/desconectado no mesmo canal de controle, sem criar outra janela scrcpy.
- Revisar as mudanças de protocolo em scrcpy 5.0.1, áudio, vídeo, entrada, rotação, mapeamento e múltiplas sessões.
- Entrega Windows portátil, com todas as DLLs/dependências, validação da compilação e testes físicos antes de classificar o pacote como pronto.

## Estado

**Fontes de referência em importação. Aplicação personalizada e homologação de uso físico ainda não concluídas.** Não anunciar qualquer artefato inicial como versão final.

Veja `docs/PLANO-TECNICO.md`.

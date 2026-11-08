# Registro de engenharia, testes e regressões — QtScrcpy integrado

**Atualizado em:** 2026-11-07  
**Repositório:** `mucholocco86/fisicos`  
**Branch de trabalho:** `dev/qtscrcpy-scrcpykeymapper-teste`  
**Natureza:** registro factual de desenvolvimento, **não** homologação.  

## 1. Finalidade e regras inegociáveis

Este sistema é a **unificação de blocos comprovadamente funcionais**, e não a criação de outro QtScrcpy, outro ScrcpyKeyMapper ou outro scrcpy. Preservar o código de referência dos fornecedores e concentrar as adaptações nas pontes entre os componentes e na apresentação. Evitar reescritas ou trocas de framework por preferência individual.

* Base para o pacote em teste: **QtScrcpy 4.2.1 + Qt5 5.15.2**, ScrcpyKeyMapper 1.0.1 e o **scrcpy-server 4.1** verificado pelo workflow. Não confundir o repositório de pesquisa de scrcpy 5.0.1 com o servidor efetivamente empacotado.
* O original do usuário foi medido no Windows: QtScrcpy 4.2.1 **76,90 MB** descompactado (46 arquivos, 9 pastas); ScrcpyKeyMapper **7,96 MB** descompactado (32 arquivos, 10 pastas).
* Computador de referência: monitor **1280 × 720**, Windows com barra de tarefas; hardware de poucos recursos. **920 × 620 é a âncora inicial global** para todas as janelas, incluindo QtScrcpy, espelhamento e editor; pode-se ampliar/diminuir manualmente. Respeitar `QScreen::availableGeometry()`, não cortar barras nem distorcer a proporção de celulares/tablets.
* UI: português brasileiro integral, tema preto/vermelho no aplicativo principal; janela de espelhamento deve reproduzir o **mockup enviado pelo usuário**, não apenas parecer semelhante.
* Editor: usar o ScrcpyKeyMapper e seu JSON original, UI com composição de referência do WASD+, imagem do aparelho já presente ao abrir, edição sobre o fundo, perfis nomeados na pasta **`keymap` nativa do QtScrcpy**, aplicação dos perfis no mesmo canal. Sem navegador externo ou internet.
* Espelhamento NÃO pode parar/desaparecer ao fechar o editor. Gamepad via controle físico e teclado físico remoto precisam funcionar, não apenas ter botões.
* Todos os pacotes devem ser compilados e entregues via **GitHub Actions**, nunca gerar ZIP local e tratá-lo como entrega.
* **Não declarar uma funcionalidade funcional/aprovada apenas por passar no compilador.**

## Decisão confirmada — scrcpy 5.0.1 apenas após homologação do sistema atual

**Decisão do proprietário (07/11/2026):** a versão **scrcpy 5.0.1 é uma possibilidade de atualização futura**, a ser examinada **somente depois** de o sistema integrado atual funcionar e ser aprovado em testes físicos. Ela **não faz parte do escopo de alteração presente**, não deve ser copiada sobre o servidor existente e não deve provocar mudança de protocolo agora.

**Base efetivamente escolhida para desenvolver e validar:** QtScrcpy 4.2.1 e seu QtScrcpyCore correspondente, Qt5 conforme a distribuição original de referência, ScrcpyKeyMapper 1.0.1, e **scrcpy-server 4.1 da base QtScrcpy**. Priorizar as funcionalidades originais já demonstradas pelos desenvolvedores e construir apenas as pontes de integração estritamente necessárias.

**Motivo:** a compatibilidade do scrcpy 5.0.1 com o QtScrcpy 4.2.1 ainda não foi comprovada pelo desenvolvedor desse QtScrcpy; não se deve presumir que substituir o executável/servidor por versão mais nova preserve funcionamento. Só considerar essa migração depois de: (1) capturar commit e pacote de referência fisicamente aprovados; (2) comparar diferenças reais de protocolo e APIs; (3) implementar compatibilidade isoladamente em branch de pesquisa; (4) comprovar o comportamento em testes; (5) obter autorização do proprietário para promover a mudança.

**Proibição de regressão:** nenhuma correção da interface atual deve antecipar ou depender do scrcpy 5.0.1. A atualização futura não justifica substituir blocos originais do sistema em validação.

## 2. Referências visuais e critérios de aceitação

### 2.1 Janela de espelhamento

No mockup aprovado a **barra de título é nativa e está no mesmo frame que o vídeo**, com ícone à esquerda, texto `Conect - <serial>` e minimizar/maximizar/fechar no topo à direita. A imagem do telefone/tablet fica dentro da janela; **a barra lateral está dentro do mesmo retângulo**, à direita do vídeo, e acompanha mover, redimensionar, minimizar e fechar.

A sequência visual de botões (de cima para baixo) tem: grupo verde, expandir/tela cheia, teclado do mapeador, gamepad, telefone/teclado físico, separador, enquadramento, volume alto, volume baixo, mudo, separador, aplicativos recentes, menu, início, voltar. As separações são parte da especificação. **Não inserir botão de clipboard ou outro botão extra acima do grupo.** Os comandos originais devem ser reaproveitados, sem desenhar botões sem ação.

Critérios: screenshot do vídeo com janela e botões como conjunto indivisível; os botões e símbolos nas posições da imagem; vídeo visível após abrir/fechar editor; proporção correta em retrato, paisagem e tablet. O usuário testará as ações em aparelho real.

### 2.2 Editor do mapeador

Referência de disposição do WASD+: ações e cenas à esquerda; imagem do jogo/celular no centro com nós visíveis e arrastáveis; à direita propriedades dos comandos, listas e controles. Elementos do original podem ser reutilizados, mas **não basta abrir a página HTML original**: ela não equivale à referência visual. A imagem deve ser capturada ao clicar no botão *Mapear teclas*, transferida para o editor **antes de disponibilizar a edição** e exibida automaticamente no centro. Salvar e reabrir o perfil com o nome do jogo e execução efetiva no Android são obrigatórios.

**Pendente de desenvolvimento/comprovação:** reprodução fiel das ferramentas, cenas e composição WASD+, captura e imagem realmente renderizada, salvar/recarregar/executar todos os tipos de comandos e integração sem regressão do vídeo.

### 2.3 Janela principal e configurações

Âncora de 920 × 620 válida em toda abertura; controles internos não podem forçar a janela para praticamente a largura de um monitor 1280 × 720. Ao redimensionar, opções continuam acessíveis por organização/rolagem; configurações de alta performance devem ser aplicadas por padrão, sem exigir troca manual a cada inicialização. Tradução integral PT-BR, inclusive textos do editor e ferramentas avançadas.

## 3. Testes físicos e defeitos observados (reprodução)

| ID | Evidência observada pelo usuário | Impacto | Situação |
|---|---|---|---|
| F-01 | QtScrcpy abria quase na largura inteira da área de trabalho apesar de tamanho inicial desejado 920 × 620 | Fora de especificação | Alterações de código feitas; **não retestado fisicamente** |
| F-02 | Tela do telefone e barra vertical apareciam como **janelas separadas** em vez de um painel único | Fora de especificação, posicionamento frágil | Causa identificada: `MagneticWidget` como `Qt::Tool` e `AP_OUTSIDE_RIGHT`; alteração para modo incorporado; **não retestado** |
| F-03 | Barra tinha botão extra de clipboard acima do grupo e outros elementos visualmente divergentes | Não corresponde ao mockup | `updateCameraMode()` tornava o clipboard visível novamente; corrigido no código; **não retestado** |
| F-04 | Editor abria sem a captura do aparelho em segundo plano | Bloqueia o uso principal | Transferência JS modificada para dados base64 locais; **não retestado** |
| F-05 | Editor mostrava lista de tipos em vez de canvas central com disposição WASD+ | Bloqueia usabilidade | CSS original empilhava colunas abaixo de 1200px; CSS integrado passou a grade de três áreas; **fidelidade WASD+ ainda pendente** |
| F-06 | Em teste anterior, ao fechar editor, o vídeo desapareceu, restando área preta | Regressão grave | Janela do editor separada do pai e mecanismos de vídeo preservados; **causa e correção definitiva ainda não demonstradas em teste** |
| F-07 | Pacote anterior ocupava 319 MB descompactados contra 84,86 MB dos originais somados | Peso excessivo | Qt6 WebEngine foi principal componente (Qt6WebEngineCore ~154 MB); workflow voltou ao Qt5; artefato Qt5 ~88,72 MB **compactado**; tamanho descompactado/otimização final pendentes |
| F-08 | Botões gamepad/teclado presentes sem teste de operação comprovado no Android | Funcionalidade exigida | Implementações UHID/XInput no código, **testes físicos pendentes** |
| F-09 | Interface do editor não totalmente PT-BR na captura apresentada | Fora de especificação | Carregamento padrão PT-BR modificado; verificar traduções completas e efetivas |

Os quatro screenshots de inspeção foram **enviados na conversa** e não estão armazenados automaticamente neste repositório. Não alegar que foram versionados. Para futuras evidências, registrar capturas em uma issue/anexo ou inserir arquivo explicitamente no GitHub.

## 4. Linha de commits, builds e tamanho

| Referência | Resultado efetivamente conhecido |
|---|---|
| Run [42415084996](https://github.com/mucholocco86/fisicos/actions/runs/42415084996) / `b122560` | Build Qt6 anterior com problemas graves de UI e recursos locais |
| Run [42422421405](https://github.com/mucholocco86/fisicos/actions/runs/42422421405) / `543c2f9` | Qt6 compilou; ZIP único; pacote ~144,59 MB compactado; `Qt6WebEngineCore.dll` ~154,4 MB |
| Run [42425672778](https://github.com/mucholocco86/fisicos/actions/runs/42425672778) / `4dcc9d0` | **Qt5 compilou**, servidor 4.1 verificado, pacote **88.721.387 bytes compactado**; teste físico revelou F-01 a F-05 |
| Run [42431650207](https://github.com/mucholocco86/fisicos/actions/runs/42431650207) / `0436c79` | Compilação e verificações estáticas concluídas; **alterações subsequentes da branch não incluídas** |
| Run [42431930394](https://github.com/mucholocco86/fisicos/actions/runs/42431930394) / `6bcd06b` | Qt5 + alterações estruturais em UI/JS; build **concluído com sucesso**, artefato **88.723.201 bytes compactado**; **teste físico ainda pendente** |

A linha `6bcd06b` é **o estado compilado mais recente confirmado na elaboração deste registro**, **não é uma versão homologada**. Artefatos GitHub Actions podem expirar; commits continuam sendo a referência de código. O workflow importa upstreams e pode gerar commits adicionais durante o build; conferir SHA da execução para saber exatamente o que foi testado.

## 5. Locais de código relevantes e correções recentes

- `upstream/QtScrcpy/QtScrcpy/ui/dialog.cpp`: construtor, `QScrollArea`, `resize(920,620)`, `on_useSingleModeCheck_clicked()` (remover `adjustSize()`), `onDeviceConnected()` e bloqueio de geometria antiga acima do limite. **Revisar área rolável e acessibilidade**.
- `upstream/QtScrcpy/QtScrcpy/ui/dialog.ui`: dimensões iniciais 920 × 620.
- `upstream/QtScrcpy/QtScrcpy/ui/videoform.cpp`: `showToolForm()` passou de janela externa `MagneticWidget` para `useEmbeddedMode()`; `resizeEvent()` acompanha a barra, `updateShowSize()` preserva proporções e limita âncora; captura de `grabFramebuffer()` ao solicitar editor.
- `upstream/QtScrcpy/QtScrcpy/uibase/magneticwidget.{h,cpp}`: `useEmbeddedMode()` desliga magnetismo e `Qt::Tool` para o caso integrado.
- `upstream/QtScrcpy/QtScrcpy/ui/toolform.cpp`: ordem/medidas dos botões, UHID gamepad e teclado; `updateCameraMode()` não deve restabelecer clipboard ou screenshot excluídos.
- `upstream/QtScrcpy/QtScrcpy/ui/nativemapperwindow.cpp`: janela independente, arquivo HTML local offline, ponte Qt, captura passada diretamente ao JavaScript; perfis JSON na pasta do QtScrcpy.
- `upstream/ScrcpyKeyMapper/css/embedded-panel.css`: CSS da disposição em três áreas; **verificar resultado renderizado e controles realmente acessíveis**.
- `upstream/ScrcpyKeyMapper/js/native-bridge.js`: `mapperApplyScreenshotData()`, ponte Qt/WebChannel, salvamento de perfis. **Verificar sincronização**.
- `upstream/ScrcpyKeyMapper/js/managers/{ConfigManager,LanguageManager}.js`: JSON compatível com formato original, idioma PT-BR.
- `.github/workflows/build-public-test.yml`: Qt5, verificação servidor 4.1, dependências, checagens estáticas, ZIP único; **essas checagens não validam pixel a pixel nem comunicação com aparelho**.

## 6. Antipadrões identificados — não repetir

1. Trocar Qt5 por Qt6 e introduzir bibliotecas pesadas sem aprovação ou justificativa funcional.
2. Fazer apenas `resize(920,620)` e deixar `adjustSize()`, `minimumSizeHint()` ou widgets internos expandirem a janela.
3. Criar barra lateral `Qt::Tool` independente e presumir que o usuário aceitará uma barra *colada* fora da janela como painel integrado.
4. Mover DOM do ScrcpyKeyMapper sem verificar que as regras `@media` deixam canvas central visível em 920 × 620.
5. Considerar que uma captura existe só porque o código chama `grabFramebuffer()`; confirmar imagem válida e renderizada no editor.
6. Aceitar regressão no espelhamento em consequência da abertura/fechamento do editor.
7. Confundir arquivo ZIP do artefato com a pasta descompactada; comparar ambos em unidades coerentes.
8. Misturar notas sobre scrcpy 5.0.1 com o servidor 4.1 efetivamente usado pelo QtScrcpy 4.2.1.
9. Reescrever componentes originais quando uma ponte ou reorganização localizada resolve.
10. Chamar um pacote de funcional ou aprovado só porque o compilador e checagens estáticas passaram.

## 7. Como continuar sem perder o trabalho

1. Ler este registro, `docs/MATRIZ-VALIDACAO.md`, `README.md`, `docs/PLANO-TECNICO.md` e verificar commits, workflow e SHA da branch.
2. Comparar a falha com as referências já especificadas. Abrir issue/registro para cada regressão; registrar **dispositivo, resolução, passos para reproduzir, resultado esperado, resultado real, screenshot e commit testado**.
3. Antes de qualquer mudança, identificar a **ponte responsável** e os módulos originais que não devem ser alterados. Fazer correção localizada.
4. Compilar via GitHub Actions em branch de testes. Conferir warnings, componentes empacotados e tamanho compactado/descompactado.
5. Testar **no Windows 1280 × 720 com celular/tablet real**. Preencher matriz como aprovado somente quando o usuário confirmar; manter pendências explicitamente.
6. Preservar commit/tag de **última versão fisicamente aprovada** antes de mudanças maiores. Nunca nomear "funcional" uma versão sem esse teste.
7. Anotar neste arquivo o erro novo, hipótese/correção, commit, run, tamanho e resultado físico. Não sobrescrever evidências de falhas antigas.
8. Só promover alterações a referência funcional quando todas as regressões críticas relevantes estiverem resolvidas.

## 8. Critério de conclusão

É necessário comprovar visual e funcionalmente, em conjunto: 920 × 620 inicial e redimensionamento coerente; única janela integrada de espelhamento com botões do mockup; vídeo intacto ao fechar editor; screenshot automático visível no canvas; organização visual do editor equivalente à referência WASD+; criação/salvamento/reabertura/execução de perfis nativos; gamepad/teclado remoto; interface PT-BR; configurações padrão de performance; pacote portátil sem recursos desnecessários. **Até lá: status de desenvolvimento, mesmo com GitHub Actions verde.**

## Continuidade — correção preventiva da rotação (07/11/2026)

- **Problema detectado por inspeção de código:** `VideoForm::updateStyleSheet()` redefinia as margens da área de vídeo com `getMargins(vertical)` ao atualizar a skin, potencialmente apagando a reserva de **34 px** para a barra lateral agora incorporada. Rotação retrato/paisagem ou ajuste de tamanho poderiam sobrepor os controles ao vídeo.
- **Mudança localizada:** preservar `getMargins(vertical)` e adicionar novamente a reserva da barra quando ela estiver presente, habilitada e fora de tela cheia; arquivo `upstream/QtScrcpy/QtScrcpy/ui/videoform.cpp`, commit `b9d3ec3`.
- **Prevenção de regressão:** verificação estática adicionada em `.github/workflows/build-public-test.yml`, commit `0079ca2`.
- **Escopo mantido:** QtScrcpy 4.2.1 / Qt5 / ScrcpyKeyMapper 1.0.1 / servidor scrcpy 4.1. **scrcpy 5.0.1 não foi introduzido.**
- **Estado:** alteração de código realizada; **sem validação física de rotação, tablet ou funcionamento da barra**. Registrar o SHA exato da próxima execução e testar T02, T04, T05 e T11 antes de considerar funcional.

## Marco de preservação e correção isolada do painel principal — 08/11/2026

**Confirmação do proprietário:** o pacote de testes [42436630181](https://github.com/mucholocco86/fisicos/actions/runs/42436630181), commit `0079ca27b1f2b87c157d0d6958341ddd66c7e9d6`, apresentou resultado físico muito próximo do ideal no restante do sistema. **Não se deve modificar o espelhamento, barra lateral, controles, editor ou ponte Qt sem evidência específica de falha e autorização para o escopo correspondente.** A versão não está homologada integralmente, mas é a referência de restauração desta etapa.

- **Backup permanente do código testado:** branch `backup/qt5-teste-promissor-42436630181`, criada apontando diretamente para `0079ca27b1f2b87c157d0d6958341ddd66c7e9d6`. Não reescrever, apagar, fazer merge ou avançar essa branch automaticamente.
- **Requisito específico da tela inicial:** restaurar **layout, organização, comportamento e dimensões originais QtScrcpy 4.2.1**, preservando SOMENTE o idioma PT-BR, tema escuro com botões e controles vermelhos e as escolhas iniciais de espelhamento de alto desempenho.
- **Causa do defeito do print 08/11:** reparenting das colunas para `QScrollArea` provocou corte da coluna direita; resize 920 × 620 contrariou a janela de fábrica. Correção: remover bloco de reparenting e wrapper, restaurar `on_useSingleModeCheck_clicked()` com `adjustSize()`, restaurar geometria original `1293 × 502` da `dialog.ui`; preservar `onDeviceConnected()` e demais personalizações responsáveis pelo vídeo/mapeamento.
- **Desempenho inicial:** bitrate default 8 Mbps com resolução 1080p já definida pelo código e codec padrão automático; `config.ini` com `MaxFps=0` e `RenderExpiredFrames=0` permanece inalterado. **Não sobrescrever preferências de instalações já existentes**: somente o valor fallback do bitrate é alterado.
- **Escopo que NÃO pode ser tocado nesta correção:** `videoform.*`, `toolform.*`, `magneticwidget.*`, `nativemapperwindow.*`, `ScrcpyKeyMapper/**`, `QtScrcpyCore/**`, scrcpy-server e scripts de execução.
- **Pendência maior depois do vídeo demonstrativo do usuário:** problemas específicos do ScrcpyKeyMapper. Problemas menores relatados: um comando de um botão do espelhamento e centralização da imagem. **Não antecipar essas correções.**
- **Aceitação da rodada atual:** comparar somente painel principal com o QtScrcpy 4.2.1 de fábrica e conferir tradução, botões vermelhos, defaults de performance. Checar que vídeo, barra e editor continuam funcionando como no build-base, sem diferenças de código nesses componentes. CI verde não substitui esse teste físico.


## Ajustes finais isolados do espelhamento — 08/11/2026

**Base física aprovada para recuperação:** compilação 42488151270, commit `95f20887f57d9cfe066cdff6704194a4942f5a7b`, preservado na branch imutável de referência `backup/painel-restaurado-aprovado-42488151270`. A branch antiga `backup/qt5-teste-promissor-42436630181` também permanece disponível.

**Solicitação restrita a dois pontos da janela de espelhamento:**
1. Distância visual de aproximadamente 10 pixels entre o conteúdo espelhado e a barra lateral de 34 pixels. A reserva de layout passa a 44 pixels somente quando a barra aparece, nos caminhos normal e de rotação/skin; barra e comandos ficam inalterados. Código: `ui/videoform.cpp`.
2. Ao clicar no botão do teclado físico, a barra de espaço podia acionar o último botão focado em vez de ser encaminhada ao Android. Causa provável verificada no código: botões `QPushButton` recebem foco de teclado por padrão. Na barra incorporada, os botões continuam acionáveis pelo mouse, mas passam a `Qt::NoFocus`; o botão que alterna o teclado devolve foco explicitamente à janela `VideoForm`, com `Qt::StrongFocus`, cuja implementação original encaminha `keyPressEvent/keyReleaseEvent` ao dispositivo. Código: `ui/toolform.cpp` e `ui/videoform.cpp`. Isso é hipótese de causa com correção de código, **aguardando validação física** digitando frases com espaços repetidamente após clicar em diferentes botões.

**Exclusões obrigatórias nesta rodada:** nenhuma alteração em `ScrcpyKeyMapper/**`, `nativemapperwindow.*`, `QtScrcpyCore/**`, `dialog.*`, tema PT-BR, layout inicial aprovado e servidor Android v4.1. Próxima etapa só após aprovação: correções da ponte/edição do ScrcpyKeyMapper.

**Validação física requerida:** vídeo retrato/paisagem, centralização e 10px visuais em ambos, teclado físico com frases/espaços em aplicativo Android após clicar alternância e também nos botões laterais; confirmar que comandos de mouse dos botões continuam normais. CI verde não prova essas condições.


## Regressão confirmada por fotos: vídeo comprimido após margem lateral de 10px — 08/11/2026

**Evidência física do proprietário:** na compilação 42522675149, a imagem do celular ficou mais estreita, contrariando o objetivo do espaço lateral. O erro não estava no valor de margem de 44px em si, mas no cálculo `updateShowSize`: os 44px (34 barra + 10 separação) eram subtraídos da largura disponível **antes** do cálculo do tamanho do vídeo; logo, os 10px de espaço foram retirados do próprio vídeo.

**Correção estritamente localizada:** `upstream/QtScrcpy/QtScrcpy/ui/videoform.cpp`, commit `982c5c5`. Dimensionar a imagem com a reserva anterior de **34px** e adicionar os **10px exclusivamente à largura externa** da janela. Reservar 44px nas margens do layout (34 barra + 10 espaço), preservando o cálculo anterior da imagem em monitores com largura útil suficiente. Em telas pequenas, o tamanho total continua limitado à área disponível. Não modificar barra, teclas, editor, serialização ou núcleo de espelhamento.

**Verificação estática:** workflow atualizado para exigir o orçamento separado de largura do vídeo e recusar a antiga fórmula que descontava os 44px antes do redimensionamento. **Estado: reteste físico pendente**; comparar largura da imagem com a referência aprovada 42488151270 e confirmar separação visual sem apertar ou distorcer a imagem.


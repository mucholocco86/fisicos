# Matriz de validação e continuidade — QtScrcpy integrado

**Revisão:** 2026-11-07  
**Situação:** em desenvolvimento. O status `OK COMPILAÇÃO` nunca equivale a `APROVADO EM TESTE FÍSICO`.  
**Origem das evidências:** feedback e screenshots enviados pelo proprietário; fontes e workflow do repositório.

## Semântica de estados

- **NÃO TESTADO:** houve alteração ou implementação, mas ainda não foi exercitada fisicamente.
- **FALHOU:** o teste físico documentou o problema, mesmo que o código pareça implementado.
- **CORRIGIDO NO CÓDIGO / RETESTE PENDENTE:** não é aprovado; falta o usuário testar.
- **APROVADO FISICAMENTE:** resultado observado e confirmado pelo proprietário, com commit e captura associados.
- **BLOQUEADO:** requisito depende de outro problema impeditivo.

## Checklist de aceitação para a próxima rodada de testes

| ID | Condição e como testar | Resultado já observado | Estado atual |
|---|---|---|---|
| T01 | Abrir QtScrcpy em monitor 1280 × 720: verificar 920 × 620, barra de tarefas livre e controles acessíveis | Janela excessivamente larga em 42425672778 | CORRIGIDO NO CÓDIGO / RETESTE PENDENTE |
| T02 | Conectar celular em retrato: observar vídeo + barra no MESMO frame, incluindo barra superior nativa | Barra flutuante externa em 42425672778 | CORRIGIDO NO CÓDIGO / RETESTE PENDENTE |
| T03 | Comparar screenshot de 920 × 620 ou menor com mockup: botões exatos, ordem, posição, tamanho, fundo e agrupamentos | Botão extra e ícones fora do padrão | CORRIGIDO PARCIALMENTE / RETESTE PENDENTE |
| T04 | Movimentar, redimensionar, minimizar/restaurar e fechar janela espelhada: barra acompanha e não fica órfã | Barra separada anteriormente | NÃO TESTADO APÓS ALTERAÇÃO |
| T05 | Conectar tablet e girar celular paisagem/retrato: proporção correta sem ultrapassar área útil | Falha potencial identificada, sem teste tablet documentado | NÃO TESTADO |
| T06 | Clicar mapear: capturar quadro atual imediatamente, abrir editor e exibir captura como fundo central | Editor abriu sem imagem em 42425672778 | CORRIGIDO NO CÓDIGO / RETESTE PENDENTE |
| T07 | Editor em janela 920 × 620: colunas/canvas/propriedades presentes, controles equivalentes aos modelos WASD+ | Canvas fora da vista em 42425672778 | CORRIGIDO PARCIALMENTE / NÃO APROVADO |
| T08 | Arrastar/posicionar todos os tipos de mapeamento e conferir coordenadas na captura; redimensionar sem perda | Não demonstrado | NÃO TESTADO |
| T09 | Nomear jogo; salvar JSON em keymap nativo; reabrir e conferir posições, teclas e nomes | Não demonstrado | NÃO TESTADO |
| T10 | Carregar o perfil no QtScrcpy e executar os comandos reais no Android | Não demonstrado | NÃO TESTADO |
| T11 | Abrir e fechar editor várias vezes: vídeo e controle do celular permanecem operantes | Vídeo desapareceu em um teste anterior | FALHOU ANTERIORMENTE / RETESTE PENDENTE |
| T12 | Gamepad físico conectado: ativar, jogar, desativar e confirmar liberação do Android | Não demonstrado | NÃO TESTADO |
| T13 | Alternar teclado virtual Android/teclado físico PC, digitar, voltar ao teclado virtual | Não demonstrado | NÃO TESTADO |
| T14 | Conferir botões de volume e silenciar em aparelho real | Botão mudo não possui mecanismo confirmado | BLOQUEADO / NÃO APROVADO |
| T15 | Verificar PT-BR em menu principal, configurações, tooltips e editor | Inglês no editor do print anterior | FALHOU ANTERIORMENTE / RETESTE PENDENTE |
| T16 | Verificar opções de alta performance já selecionadas na primeira execução limpa, sem configuração manual | Não demonstrado | NÃO TESTADO |
| T17 | Baixar artifact Github Actions: único ZIP, instalação portátil, conferir DLLs e pasta descompactada | ZIP duplo eliminado; Qt5 artifact ~88,72 MB compactado | PARCIALMENTE VERIFICADO |
| T18 | Duas sessões simultâneas, sem colisão de perfis/controles | QtScrcpy original suporta múltiplos dispositivos; integração não testada | NÃO TESTADO |

## Formato obrigatório de registro após cada teste

```text
Data/hora:
Commit SHA exato do artifact:
Workflow run / artifact ID:
SO e resolução / escala de exibição / barra de tarefas:
Dispositivo / Android / orientação / conexão USB ou Wi-Fi:
ID do teste:
Passos reproduzíveis:
Resultado esperado:
Resultado observado:
Status: APROVADO | FALHOU | BLOQUEADO
Evidência: screenshot/log (link ou caminho versionado):
Hipótese de causa:
Arquivos efetivamente alterados:
Commit da correção:
Reteste (data, run, resultado):
```

## Fluxo de correção sem regressões

1. **Analisar antes de mudar:** investigar erro real e código original; proibir "recriação" de módulos apenas por conveniência.
2. **Uma ponte de cada vez:** preferir pequenas alterações isoladas, com commits e motivo. Não trocar base Qt5, servidor, parser JSON ou UI sem justificativa e autorização.
3. **Reprodutibilidade:** registrar versões e hashes usados no Actions; nunca depender de `latest` sem controle.
4. **Automação:** CI verifica compilação, distribuição e invariantes estáticas; testes de UI e Android requerem prova específica e/ou testes físicos.
5. **Não promover sem aprovação:** runs bem-sucedidas permanecem builds de diagnóstico enquanto T01–T18 não estiverem aprovados onde aplicáveis.
6. **Rollback seguro:** referência "funcional" é a última homologada pelo proprietário; antes da primeira homologação, identificar explicitamente builds apenas como experimentais.

## Pendências prioritárias por dependência

A. Integridade do espelhamento e painel único (T02–T05, T11).  
B. Captura e editor utilizável com fundo real (T06–T08).  
C. Persistência e execução dos perfis (T09–T10).  
D. Entrada Android (T12–T14) e PT-BR/performance (T15–T16).  
E. Dimensão da janela principal e empacotamento (T01, T17), continuamente.

**Importante:** esta lista NÃO autoriza inventar funcionalidades, reduzir escopo do usuário nem substituir componentes originais. Registra os compromissos para continuidade entre chats, commits e workflow.

# Plano técnico — QtScrcpy + ScrcpyKeyMapper + scrcpy 5.0.1

## Restrições

1. O programa é uma evolução do **QtScrcpy** original. O gerenciador e renderizador são os do upstream; não executar nem incorporar o antigo WELLS Conect.
2. Preservar as APIs reais de QtScrcpyCore e a estrutura de mapeamentos QtScrcpy. O ScrcpyKeyMapper fornece edição de JSON compatível; sua versão v1.0.1 é uma aplicação web, então sua interface requer **reimplementação nativa Qt**, mantendo intactos os formatos/semântica originais.
3. Não exibir navegador: nenhuma ação de mapeamento pode chamar Chrome/Edge.
4. Múltiplos celulares: preservar o suporte original e evitar limites artificiais de UI; cada sessão deve ter seu próprio controle e perfil.
5. Controle gamepad: se QtScrcpyCore não oferecer encaminhamento nativo, adaptar a implementação do scrcpy 5.0.1 usando canal de controle; não converter gamepad em toques, não abrir segunda instância por botão.
6. Modificar o mínimo necessário e comparar com o upstream. Testes físicos são obrigatórios para afirmar funcionamento.

## Sequência

- [x] Arquivar a base antiga sem removê-la irreversivelmente
- [x] Fixar versões oficiais e descobrir QtScrcpyCore necessário
- [ ] Importar arquivos integrais de todos os repositórios para `upstream/` em `main` via GitHub Actions
- [ ] Inspecionar as configurações default e adaptar a UI QtScrcpy para PT-BR, preto/vermelho, perfis de performance
- [ ] Integrar editor nativo que edita o mesmo JSON usado pelo QtScrcpy
- [ ] Capturar tela automaticamente para fundo do editor
- [ ] Adicionar encaminhamento nativo de gamepad com alternância ao vivo
- [ ] Revisar e aplicar compatibilidade de QtScrcpyCore ao servidor scrcpy 5.0.1
- [ ] Gerar instalável portátil Windows com dependências oficiais
- [ ] Validar dois celulares físicos simultâneos, perfis e ações, sem regressões

## Originais

- QtScrcpy: tag v4.2.1, `f90e2259ea6745c30fc242f501a0210771288f8a`
- QtScrcpyCore submódulo: `9e388b8aa48e4e1c2cfdef408ef00c4c6b45c921`
- ScrcpyKeyMapper: tag v1.0.1, `022870e2caf905046c8e7502d4053bb5f171c1e4`
- scrcpy: tag v5.0.1, `a60891aea193d92e7e5c3942700eca63f9d19a5f`

Não editar os diretórios `upstream/` para alterações do produto até estabelecer estratégia de patches reproduzíveis e revisão dos testes.

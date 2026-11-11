# ScrcpyKeyMapper — aplicativo independente (protótipo)

Projeto isolado do **WELLS CONECT**. A versão final do Wells Conect não é alterada.

## Origem limpa
- Autor: [w4po/ScrcpyKeyMapper](https://github.com/w4po/ScrcpyKeyMapper)
- Commit original fixado: `022870e2caf905046c8e7502d4053bb5f171c1e4`
- O código original é importado para `scrcpykeymapper-standalone/original/` pelo GitHub Actions, sem reutilizar protótipos antigos.
- Preservar `original/LICENSE` e os créditos. O editor original é servido localmente, sem abrir Chrome/Firefox.
- `editor/` é uma cópia de empacotamento com bibliotecas web locais; não altera `original/`.

## Interface
Janela nativa Windows C++/Qt6 com minimizar, maximizar, restaurar e fechar. Qt WebEngine apresenta o editor HTML/CSS/JS original. Servidor HTTP somente em `127.0.0.1`, porta dinâmica, para permitir módulos JavaScript ES sem problemas de CORS do esquema file://. Nenhuma conexão ou comunicação com Wells Conect.

## Compilação
GitHub Actions: `ScrcpyKeyMapper standalone Windows`. O workflow importa a base oficial fixada, instala dependências web locais e gera ZIP de teste. O primeiro build deve ser validado em Windows antes de qualquer declaração de estabilidade. Importar screenshot, criar/editar controles, salvar e recarregar JSON são critérios obrigatórios.

## Estado
Protótipo inicial; não substituir projetos finalizados.

## Personalização aprovada
- Ícone exclusivo em carbono e vermelho, incorporado ao executável Windows por recurso .rc (arte reduzida para uso como ícone).
- Seletor de Background abre na pasta Imagens do Windows, sem alterar o editor original.
- Exportação JSON permanece independente do Wells Conect.

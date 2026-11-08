/* Ponte Qt WebChannel: preserva o editor e os nós originais do ScrcpyKeyMapper. */
(function () {
    'use strict';
    // Apenas reposiciona os cartões já existentes; os mesmos nós DOM
    // continuam recebendo todos os eventos originais do ScrcpyKeyMapper.
    document.addEventListener('DOMContentLoaded', function () {
        const left = document.getElementById('mapperLeftSections');
        if (!left) return;
        for (const id of ['availableMappings', 'nodeListSection']) {
            const part = document.getElementById(id);
            const card = part && part.closest('section');
            if (card) left.appendChild(card);
        }
    });

    window.mapperReady = false;
    window.mapperInitialProfile = null;

    window.mapperLoadProfile = function (data) {
        if (!data || !window.configManager || !window.nodeManager) return;
        // Executa o importador oficial, inclusive os vários tipos de nós.
        const file = new File([JSON.stringify(data)], 'profile.json',
                              { type: 'application/json' });
        window.configManager.loadFromJson(file);
    };

    // A captura e entregue diretamente pelo Qt em base64, sem navegador e
    // sem depender de disponibilidade da ponte WebChannel.
    window.mapperApplyScreenshotData = function (imageData) {
        if (!imageData || !imageData.startsWith('data:image/png;base64,')) {
            console.error('Captura de tela vazia ou invalida.');
            return;
        }
        try {
            const raw = atob(imageData.substring(imageData.indexOf(',') + 1));
            const bytes = new Uint8Array(raw.length);
            for (let i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
            const file = new File([bytes], 'tela-atual-celular.png', { type: 'image/png' });
            const input = document.getElementById('uploadBackground');
            if (!input) throw new Error('Campo de imagem de fundo nao encontrado');
            const transfer = new DataTransfer();
            transfer.items.add(file);
            input.files = transfer.files;
            input.dispatchEvent(new Event('change', { bubbles: true }));
        } catch (error) {
            console.error('Nao foi possivel inserir a imagem no mapeador:', error);
        }
    };
    window.mapperApplyScreenshot = function () {
        if (!window.nativeMapper) return;
        window.nativeMapper.screenData(window.mapperApplyScreenshotData);
    };

    if (window.qt && window.qt.webChannelTransport && window.QWebChannel) {
        new QWebChannel(window.qt.webChannelTransport, function (channel) {
            window.nativeMapper = channel.objects.nativeMapper;
        });
    }
}());

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

    window.mapperApplyScreenshot = function () {
        if (!window.nativeMapper || !window.nodeManager) return;
        window.nativeMapper.screenData(function (imageData) {
            if (!imageData) return;
            fetch(imageData).then(response => response.blob()).then(blob => {
                const input = document.getElementById('uploadBackground');
                if (!input) return;
                const file = new File([blob], 'device-screen.png',
                                      { type: 'image/png' });
                const transfer = new DataTransfer();
                transfer.items.add(file);
                input.files = transfer.files;
                input.dispatchEvent(new Event('change', { bubbles: true }));
            }).catch(error => console.error('Falha ao capturar a imagem:', error));
        });
    };

    if (window.qt && window.qt.webChannelTransport && window.QWebChannel) {
        new QWebChannel(window.qt.webChannelTransport, function (channel) {
            window.nativeMapper = channel.objects.nativeMapper;
        });
    }
}());

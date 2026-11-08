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
        if (!window.nativeMapper || !window.nodeManager) {
            console.error('A ponte ou o canvas do mapeador nao esta pronta.');
            return;
        }
        window.nativeMapper.screenData(function (imageData) {
            if (!imageData || !imageData.startsWith('data:image/png;base64,')) {
                console.error('A captura da tela do Android esta vazia.');
                return;
            }
            try {
                // Usa o screenshot fornecido pelo QtScrcpy inteiramente offline.
                // Evita fetch(data:), que pode ser bloqueado pelo esquema file://.
                const raw = atob(imageData.substring(imageData.indexOf(',') + 1));
                const bytes = new Uint8Array(raw.length);
                for (let i = 0; i < raw.length; i++) bytes[i] = raw.charCodeAt(i);
                const file = new File([bytes], 'tela-atual-celular.png',
                    { type: 'image/png' });
                const input = document.getElementById('uploadBackground');
                if (!input) throw new Error('Campo de fundo nao encontrado');
                const transfer = new DataTransfer();
                transfer.items.add(file);
                input.files = transfer.files;
                input.dispatchEvent(new Event('change', { bubbles: true }));
            } catch (error) {
                console.error('Falha ao adicionar captura no canvas:', error);
            }
        });
    };

    if (window.qt && window.qt.webChannelTransport && window.QWebChannel) {
        new QWebChannel(window.qt.webChannelTransport, function (channel) {
            window.nativeMapper = channel.objects.nativeMapper;
        });
    }
}());

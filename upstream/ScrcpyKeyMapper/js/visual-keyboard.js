// Teclado visual opcional; dispara os eventos já aceitos pelo KeyInputManager.
(function () {
  'use strict';
  const rows = [
    ['Esc','F1','F2','F3','F4','F5','F6','F7','F8','F9','F10','F11','F12'],
    ['~','1','2','3','4','5','6','7','8','9','0','-','=','Backspace'],
    ['Tab','Q','W','E','R','T','Y','U','I','O','P','[',']','\\'],
    ['Caps','A','S','D','F','G','H','J','K','L',';',"'",'Enter'],
    ['Shift','Z','X','C','V','B','N','M',',','.','/'],
    ['Ctrl','Alt','Space','Left','Down','Up','Right'],
    ['LMB','MMB','RMB','Forward','Back']
  ];
  const special = {
    Esc:['Escape','Escape'], '~':['Backquote',String.fromCharCode(96)],
    Backspace:['Backspace','Backspace'], Tab:['Tab','Tab'],
    Caps:['CapsLock','CapsLock'], Enter:['Enter','Enter'],
    Shift:['ShiftLeft','Shift'], Ctrl:['ControlLeft','Control'],
    Alt:['AltLeft','Alt'], Space:['Space',' '],
    Left:['ArrowLeft','ArrowLeft'], Right:['ArrowRight','ArrowRight'],
    Up:['ArrowUp','ArrowUp'], Down:['ArrowDown','ArrowDown'],
    '-':['Minus','-'], '=':['Equal','='],
    '[':['BracketLeft','['], ']':['BracketRight',']'],
    '\\':['Backslash','\\'], ';':['Semicolon',';'],
    "'":['Quote',"'"], ',':['Comma',','], '.':['Period','.'],
    '/':['Slash','/']
  };
  let target = null;
  let modal = null;

  function chooseKey(label) {
    let field = target;
    if (!field || !field.isConnected) {
      field = document.getElementById('mappingKeyProperties')
           || document.getElementById('switchKeyGlobal');
    }
    if (!field) return;
    modal.hidden = true;
    field.focus();
    if (['LMB','MMB','RMB','Forward','Back'].includes(label)) {
      const number = {LMB:0,MMB:1,RMB:2,Forward:3,Back:4}[label];
      document.dispatchEvent(new MouseEvent('mousedown', {bubbles:true,button:number}));
    } else {
      const entry = special[label] || (/^F\d+$/.test(label) ? [label,label]
         : /^\d$/.test(label) ? ['Digit'+label,label]
         : ['Key'+label,label.toLowerCase()]);
      field.dispatchEvent(new KeyboardEvent('keyup', {
        bubbles:true,code:entry[0],key:entry[1]
      }));
    }
    field.blur();
  }

  document.addEventListener('DOMContentLoaded', function () {
    modal = document.createElement('div');
    modal.className = 'visual-keyboard-overlay';
    modal.hidden = true;
    modal.innerHTML = '<section class="visual-keyboard-dialog" role="dialog" aria-modal="true">'
      + '<header><strong>Selecione a tecla de mapeamento</strong>'
      + '<button type="button" class="visual-keyboard-close">×</button></header>'
      + '<div class="visual-keyboard-rows"></div></section>';
    modal.addEventListener('mousedown', event => event.stopPropagation());
    const grid = modal.querySelector('.visual-keyboard-rows');
    rows.forEach(row => {
      const line = document.createElement('div');
      line.className = 'visual-keyboard-row';
      row.forEach(key => {
        const button = document.createElement('button');
        button.type = 'button';
        button.className = 'visual-keyboard-key';
        button.disabled = key === 'Forward' || key === 'Back';
        if ('WASD'.includes(key) && key.length === 1) {
          button.classList.add('visual-keyboard-highlight');
        }
        button.textContent = key;
        button.addEventListener('click', () => chooseKey(key));
        line.appendChild(button);
      });
      grid.appendChild(line);
    });
    modal.querySelector('.visual-keyboard-close')
      .addEventListener('click', () => { modal.hidden = true; });
    modal.addEventListener('click', event => {
      if (event.target === modal) modal.hidden = true;
    });
    document.body.appendChild(modal);
    document.addEventListener('focusin', event => {
      if (event.target.matches('input[type=text]')
          && (event.target.id === 'switchKeyGlobal'
          || event.target.id === 'mappingKeyProperties'
          || event.target.closest('#mappingPropertiesSection'))) {
        target = event.target;
      }
    });
    const picker = document.createElement('button');
    picker.type = 'button';
    picker.className = 'btn btn-outline-secondary';
    picker.textContent = 'Teclado visual';
    picker.title = 'Selecionar uma tecla pelo teclado e mouse da tela';
    picker.addEventListener('mousedown', event => event.stopPropagation());
    picker.addEventListener('click', () => { modal.hidden = false; });
    const navbar = document.querySelector('.navbar .container-fluid');
    if (navbar) navbar.appendChild(picker);
  });
}());

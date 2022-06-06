import './styles/styles_popover.scss';
import jquery from 'jquery';
import 'tippy.js/dist/backdrop.css';
import 'tippy.js/animations/shift-away.css';

import designIcon from '../../../img/sales/projectDropdown/design.svg';
import lighting from '../../../img/sales/projectDropdown/lighting.svg';
import paket from '../../../img/sales/projectDropdown/paket.svg';
import managerB from '../../../img/sales/managerB.svg';
import managerS from '../../../img/sales/managerS.svg';
import { routes } from '../../../routes';
import dateFormat from 'dateformat';

let createPopoverHtml = ({
                           projectName,
                           soldStatus,
                           soldClass,
                           soldDate,
                           packageName,
                           hasBranding,
                           hasLightning,
                           salesManager,
                           backOfficeManager,
                           comment,
                         }) =>
  `<div>
  <div class="project-card-close-button">
  <div class="project-card-close-button2">
    <svg version="1.0" viewBox="0 0 24 24">
      <line x1="0" y1="24" x2="24" y2="0"/>
      <line x1="24" y1="24" x2="0" y2="0"/>
    </svg>
  </div>
</div>
  <div class="ant-space-item" style="margin-right: 8px;">
    <div class="project-card-root-style-2">
      <div class="project-card-root-style-3">
        <span class="project-card-style">Проект ${projectName}</span>
        <p class="project-card-reservation-status">
          <span class="${soldClass}">${soldStatus}</span>
          <span>${soldDate}</span>
        </p>
      </div>
      <button type="button" class="ant-btn ant-btn-primary project-card-button">
        <span>Открыть Проект</span>
      </button>
    </div>
  </div>
  <div class="ant-space-item" style="margin-right: 8px;">
    <div class="project-card-package-description">
      <div class="sectionItem" class="project-card-package-description-inner">
        <img src="${paket}" alt="paket icon"/>
        <span>Пакет:</span>
        <span style="font-weight: bold;">${packageName}</span>
      </div>
      <div class="sectionItem project-card-selection-item">
        <img src="${designIcon}" alt="design icon"/>
        <span>Брендинг:</span>
        <span style="font-weight: bold;">${hasBranding}</span>
      </div>
      <div class="sectionItem">
        <img src="${lighting}" alt="lighting icon"/>
        <span>Освещение:</span>
        <span style="font-weight: bold;">${hasLightning}</span>
      </div>
    </div>
  </div>
  <div class="ant-space-item" style="margin-right: 8px;">
    <div class="project-card-peoples-root">
      <div class="project-card-peoples-root-inner">
        <div class="project-card-peoples-manager-role">
          <img src="${managerS}" alt="icon"/>
          <span>Менеджер по продажам:</span>
        </div>
        <span class="project-card-people-manager-name">${salesManager}</span>
      </div>
      <div class="project-card-peoples-root-backoffice-inner">
        <div class="project-card-peoples-manager-role">
          <img src="${managerB}" alt="icon"/>
          <span>Менеджер бэк-офиса:</span>
        </div>
        <span class="project-card-people-manager-name">${backOfficeManager}</span>
      </div>
    </div>
  </div>
  <div class="ant-space-item" style="margin-right: 8px;">
    <div class="project-card-comment">
      <input placeholder="Комментарий" type="text" class="ant-input ant-input-lg" value=${comment ? comment : ''}>
    </div>
  </div>
  <div class="ant-space-item">
  </div>
</div>`;

let index = null;

let getPopoverStatusClass = (barClass) => {
  // console.log(barClass);
  if (barClass === 'Свободно')
    return 'free';
  if (barClass === 'Забронировано')
    return 'reserved';
  if (barClass === 'Утверждено')
    return 'confirmed';
  if (barClass === 'Продано')
    return 'sold';
  if (barClass === 'unavailable')
    return 'unavailable';
  return '';
};

export function createPopover(domNode, taskItem) {
  // console.log(taskItem);
  let popoverHtml = createPopoverHtml({
    projectName: taskItem.project,
    soldStatus: taskItem.type,
    soldClass: getPopoverStatusClass(taskItem.type),
    soldDate: `до ${dateFormat(taskItem.finish, 'dd.mm.yyyy')}`,
    packageName: taskItem.package,
    hasBranding: taskItem.branding ? 'да' : 'нет',
    hasLightning: taskItem.status,
    salesManager: taskItem.salesManager,
    backOfficeManager: taskItem.backOfficeManager,
    comment: taskItem.comment,
  });

  let popupDomTree = jquery.parseHTML(popoverHtml)[0];
  jquery(popupDomTree)
    .find('button.ant-btn')
    .click(() => {
      jquery(domNode).popover('hide');
      taskItem.history.push(routes.sales.project_card.url(taskItem.projectId));
    });

  jquery(domNode).popover({
    content: popupDomTree,
    html: true,
    trigger: 'click',
    placement: 'bottom',
    fallbackPlacement: ['bottom'],
  });

  let shown = false;

  jquery(window).click(function(event) {
    const comment = typeof event.target.className === 'string' ? event.target.className.includes('ant-input') : false;
    if (shown && !comment) {
      jquery(domNode).popover('hide');
    }
  });

  jquery(domNode).on('shown.bs.popover', function() {
    if (index !== taskItem.index) {
      jquery(document.getElementById('popover' + index)).popover('hide');
    }
    if (!shown) {
      shown = true;
      index = taskItem.index;
    }
  });

  jquery(domNode).click(function(event) {
    event.stopPropagation();
  });

  jquery(domNode).on('hidden.bs.popover', function() {
    if (shown) {
      shown = false;
    }
  });

  jquery(popupDomTree)
    .find('.project-card-close-button')
    .click(() => {
      jquery(domNode).popover('hide');
    });

  jquery(domNode).popover('show');
}

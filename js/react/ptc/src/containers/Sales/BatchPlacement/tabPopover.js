import './styles_popover.scss'
import jquery from 'jquery'
import 'tippy.js/dist/backdrop.css';
import 'tippy.js/animations/shift-away.css';

import designIcon from '../../../img/sales/projectDropdown/design.svg';
import lighting from '../../../img/sales/projectDropdown/lighting.svg';
import paket from '../../../img/sales/projectDropdown/paket.svg';
import managerB from '../../../img/sales/managerB.svg';
import managerS from '../../../img/sales/managerS.svg';


let createPopoverHtml = ({projectName, soldStatus, soldClass, soldDate, packageName,
                          hasDesign, hasLightning, salesManager, backOfficeManager}) => (
`<div>
  <div class="ant-space-item" style="margin-right: 8px;">
    <div class="project-card-root-style-2">
      <div class="project-card-root-style-3">
        <span class="project-card-style">Проект ${projectName}</span>
        <p>
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
        <span>Дизайн:</span>
        <span style="font-weight: bold;">${hasDesign}</span>
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
      <input placeholder="Комментарий" type="text" class="ant-input ant-input-lg" value="">
    </div>
  </div>
  <div class="ant-space-item">
  </div>
</div>`
);


export function createPopover(domNode, taskItem, sliderState) {
  // console.log(domNode, taskItem)
  let popoverHtml = createPopoverHtml({
    projectName: "СocaCola",
    soldStatus: "Продано",
    soldClass: "sold",
    soldDate: "до 24.07.2020",
    packageName: "A2",
    hasDesign: "да",
    hasLightning: "да",
    salesManager: "Иванов Иван Иванович",
    backOfficeManager: "Иванов Иван Иванович",
  })
  jquery(domNode).popover({
    content: jquery.parseHTML(popoverHtml)[0],
    html: true,
    // trigger: "hover", // "click"
    trigger: "click", // "click"
    placement: "top",
  });
  jquery(domNode).popover('show')
  jquery(domNode).on('click', () => {
    console.log('[sliderState]', sliderState);
    sliderState && sliderState.setAddShowed(true);
  })
}


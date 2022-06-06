// css-классы для вертикальной разлиновки многострочного заголовка
const CLS_MULTI_ROW = 'colhead-multiline-multirow';
const CLS_MULTILINE_ROW = 'colhead-multiline-row';
const CLS_ML_FIRST = 'colhead-first-line-middle';
const CLS_ML_MIDDLE = 'colhead-first-line-middle';
const CLS_ML_LAST = 'colhead-first-line-last';

const addThMultilineClass = (domNode, isFirst) => {
  if (isFirst)
    domNode.classList.add(CLS_ML_FIRST);
  else
    domNode.classList.add(CLS_ML_MIDDLE);
};

const addThMultilineClassLast = (domNode) => {
  domNode.classList.add(CLS_ML_LAST);
};

const checkItemIsOnlyText = (domNode) => {
  return (
    !domNode.hasChildNodes ||
    (domNode.childNodes && domNode.childNodes.length === 1 && domNode.childNodes[0].nodeType === Node.TEXT_NODE)
  );
};

// Выполнить стилизацию многострочного заголовка
export const doStylingHeader = () => {
  let x = document.querySelectorAll('.custom-tablea-antd .ant-table-thead > tr');
  let item = x[0];
  item.classList.remove(CLS_MULTILINE_ROW);
  if (x && x.length < 2)
    return;
  if (!item.hasChildNodes)
    return;
  item.classList.add(CLS_MULTILINE_ROW);
  let childList = item.childNodes;
  // let isFirst = true;
  let lastI = childList.length - 1;
  for (let i = 0; i < childList.length; ++i) {
    let childItem = childList[i];
    childItem.classList.remove(CLS_MULTI_ROW);
    childItem.classList.remove(CLS_ML_FIRST);
    childItem.classList.remove(CLS_ML_MIDDLE);
    childItem.classList.remove(CLS_ML_LAST);
    if (checkItemIsOnlyText(childItem)) {
      if (i === lastI || !checkItemIsOnlyText(childList[i + 1]))
        addThMultilineClassLast(childItem);
      else {
        addThMultilineClass(childItem, false);
        // isFirst = false;
      }
    }
    else {
      if (i > 1)
        // пропустить первый чекбокс
        childItem.classList.add(CLS_MULTI_ROW);
      // isFirst = true;
    }
  }
};

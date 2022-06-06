/* eslint-disable */
import { GanttChartView } from './DlhGanttChart';
let getBooleanNode = undefined;
let ToolTip = undefined;

export const ScheduleChartView = (function () {
  var ScheduleChartViewConstructor = function (rootDomNode, items, copiedSettings) {
      rootDomNode.isScheduleChartInitializing = true;
      let tmpItems = items;
      items = [];
      let tmpCopiedSettings = 0;
      let visibilityIdx = 0;
      let itemIdx = 0;
      for (; itemIdx < tmpItems.length; itemIdx++) {
        var item = tmpItems[itemIdx];
        if (!(typeof item.scheduleChartItem !== 'undefined' && item.scheduleChartItem != item)) {
          item.scheduleChartIndex = tmpCopiedSettings++;
          if (!item.isHidden) item.scheduleChartVisibilityIndex = visibilityIdx++;
          item.isBarVisible = false;
          items.push(item);
          if (typeof item.ganttChartItems !== 'undefined')
            for (var taskIdx = 0; taskIdx < item.ganttChartItems.length; taskIdx++) {
              var task = item.ganttChartItems[taskIdx];
              task.scheduleChartItem = item;
              if (item.isHidden) task.isHidden = true;
              task.displayRowIndex = item.scheduleChartVisibilityIndex;
              task.indentation = item.indentation;
              items.push(task);
            }
        }
      }
      typeof copiedSettings !== 'object' && (copiedSettings = {});
      GanttChartView.initializeItems(items, copiedSettings);
      tmpCopiedSettings = copiedSettings;
      setDefaultColumnSettings(tmpCopiedSettings);
      if (typeof tmpCopiedSettings.gridWidth === 'undefined') tmpCopiedSettings.gridWidth = '15%';
      if (typeof tmpCopiedSettings.chartWidth === 'undefined')
        tmpCopiedSettings.chartWidth = tmpCopiedSettings.isGridVisible ? '85%' : '100%';
      if (typeof tmpCopiedSettings.columns === 'undefined')
        tmpCopiedSettings.columns = getDefaultColumns(items, tmpCopiedSettings);
      if (typeof tmpCopiedSettings.areTaskDependenciesVisible === 'undefined')
        tmpCopiedSettings.areTaskDependenciesVisible = false;
      if (typeof tmpCopiedSettings.isBaselineVisible === 'undefined') tmpCopiedSettings.isBaselineVisible = false;
      if (typeof tmpCopiedSettings.areAssignmentsReadOnly === 'undefined')
        tmpCopiedSettings.areAssignmentsReadOnly = false;
      if (typeof tmpCopiedSettings.assignmentThumbStyle === 'undefined')
        switch (tmpCopiedSettings.theme) {
          default:
            tmpCopiedSettings.assignmentThumbStyle =
              'fill: none; stroke: #3b87d9; stroke-width: 0.65px; stroke-dasharray: 2, 2';
            break;
          case 'Aero':
            tmpCopiedSettings.assignmentThumbStyle = 'fill: none; stroke: Blue; stroke-dasharray: 2, 2';
        }
      if (typeof tmpCopiedSettings.temporaryAssignmentThumbStyle === 'undefined')
        switch (tmpCopiedSettings.theme) {
          default:
            tmpCopiedSettings.temporaryAssignmentThumbStyle =
              'fill: none; stroke: #3b87d9; stroke-width: 0.65px; stroke-dasharray: 2, 2';
            break;
          case 'Aero':
            tmpCopiedSettings.temporaryAssignmentThumbStyle =
              'fill: none; stroke: Blue; stroke-width: 0.65px; stroke-dasharray: 2, 2';
        }
      if (typeof tmpCopiedSettings.assignmentThumbTemplate === 'undefined')
        tmpCopiedSettings.assignmentThumbTemplate = P(items, rootDomNode, tmpCopiedSettings);
      tmpCopiedSettings.internalExtraTaskTemplate = tmpCopiedSettings.assignmentThumbTemplate;
      K(rootDomNode, items, tmpItems, copiedSettings);
      GanttChartView.initialize(rootDomNode, items, copiedSettings, 'init warnings');
      r(rootDomNode, items);
      rootDomNode.isScheduleChartInitializing = false;
      rootDomNode.isScheduleChartInitialized = true;
      return rootDomNode;
    },
    refreshScheduleChartView = function (k) {
      ScheduleChartViewConstructor(k, k.scheduleChartItems, k.settings, k.license);
    },
    setDefaultColumnSettings = function (settings) {
      if (typeof settings.useUpdatingToolTips === 'undefined') settings.useUpdatingToolTips = true;
      if (typeof settings.target === 'undefined') settings.target = 'Standard';
      if (typeof settings.theme === 'undefined') settings.theme = 'Modern';
      if (typeof settings.isGridVisible === 'undefined')
        switch (settings.target) {
          default:
            settings.isGridVisible = true;
            break;
          case 'Phone':
            settings.isGridVisible = false;
        }
      if (typeof settings.interaction === 'undefined')
        settings.interaction = settings.target != 'Phone' ? 'Standard' : 'TouchEnabled';
      if (typeof settings.isReadOnly === 'undefined') settings.isReadOnly = false;
      if (typeof settings.isGridReadOnly === 'undefined') settings.isGridReadOnly = false;
      if (typeof settings.isContentReadOnly === 'undefined') settings.isContentReadOnly = false;
      if (typeof settings.selectionMode === 'undefined') settings.selectionMode = 'Focus';
      if (typeof settings.isVirtualizing === 'undefined') settings.isVirtualizing = true;
      if (settings.target == 'Phone' && typeof settings.areTaskAssignmentsVisible === 'undefined')
        settings.areTaskAssignmentsVisible = false;
    },
    getDefaultColumns = function (items, settings) {
      typeof settings !== 'object' && (settings = {});
      setDefaultColumnSettings(settings);
      var headerArr = [
        { header: '', width: 40, isSelection: true },
        // {
        //   header: 'Код', width: 120, isTreeView: true
        // }
      ];
      headerArr[0].cellTemplate = returnFirstCheckBox(settings, headerArr[0], items);
      // headerArr[1].cellTemplate = O(j, headerArr[1], k);
      // headerArr[1].exportCellTemplate = O(j, headerArr[1], k, false);
      settings.selectionMode != 'Single' &&
        settings.selectionMode != 'Extended' &&
        settings.selectionMode != 'ExtendedFocus' &&
        headerArr.splice(0, 1);
      return headerArr;
    },
    returnFirstCheckBox = function (k, j) {
      return function (k) {
        return !j.isReadOnly
          ? createFirstCheckBox(k)
          : getBooleanNode && getBooleanNode(k.ganttChartView.ownerDocument, k.isSelected);
      };
    },
    createFirstCheckBox = function (k) {
      var j = k.ganttChartView.ownerDocument,
        checkBoxInput;
      var containerChkbox;
      if (typeof k.selectionInput === 'undefined') {
        containerChkbox = j.createElement('div');
        checkBoxInput = j.createElement('input');
        var labelChkbox = j.createElement('label');
        labelChkbox.classList.add('gantt-checkbox-2-label');
        labelChkbox.appendChild(checkBoxInput);
        var chkboxSpan = j.createElement('span');
        labelChkbox.appendChild(chkboxSpan);
        k.selectionInputContainer = containerChkbox;
        k.selectionInput = checkBoxInput;
        checkBoxInput.classList.add('gantt-checkbox-2');
        checkBoxInput.type = 'checkbox';
        containerChkbox.appendChild(labelChkbox);
        containerChkbox.classList.add('gantt-checkbox-2-label-container');
        /* o.setAttribute('style', 'margin: 0px'); */
      } else {
        checkBoxInput = k.selectionInput;
        containerChkbox = k.selectionInputContainer;
      }
      if (k.isSelected) {
        checkBoxInput.setAttribute('checked', 'checked');
        if (!checkBoxInput.checked) checkBoxInput.checked = true;
      } else if (checkBoxInput.checked) checkBoxInput.checked = false;
      var p = function () {
        console.log('checkBoxInput.checked', checkBoxInput.checked);
        checkBoxInput.checked
          ? S(k)
          : typeof k.scheduleChartView !== 'undefined' && k.scheduleChartView.unselectItem(k);
      };
      typeof checkBoxInput.changeEventListener !== 'undefined' &&
        checkBoxInput.removeEventListener('change', checkBoxInput.changeEventListener, true);
      checkBoxInput.addEventListener('change', p, true);
      checkBoxInput.changeEventListener = p;
      j = function (j) {
        if (j.keyCode == 13) {
          j.preventDefault();
          j.stopPropagation();
          p(j);
        }
      };
      typeof checkBoxInput.keyPressEventListener !== 'undefined' &&
        checkBoxInput.removeEventListener('keypress', checkBoxInput.keyPressEventListener, true);
      checkBoxInput.addEventListener('keypress', j, true);
      checkBoxInput.keyPressEventListener = j;
      return containerChkbox;
    },
    S = function (k) {
      typeof k.scheduleChartView !== 'undefined' && k.scheduleChartView.selectItem(k);
    },
    $ = function (k, j) {
      if (typeof k.scheduleChartView !== 'undefined') k.scheduleChartView.currentItem = k;
      (j.selectionMode == 'Focus' || j.selectionMode == 'ExtendedFocus') && !k.isSelected && S(k);
    },
    O = function (k, j, o, p) {
      var t = function (j) {
        var k = j.content,
          j = j.ganttChartView.ownerDocument.createElement('span');
        j.innerHTML = k;
        return j;
      };
      return (typeof p === 'undefined' || p) && !k.isReadOnly && !k.isGridReadOnly && !k.isContentReadOnly
        ? function (o) {
            return !j.isReadOnly && (typeof o.isReadOnly === 'undefined' || !o.isReadOnly)
              ? V(
                  o,
                  Math.max(
                    0,
                    j.width -
                      o.indentation * o.ganttChartView.settings.indentationLevelWidth -
                      o.ganttChartView.settings.toggleButtonAreaWidth -
                      16,
                  ),
                  k,
                )
              : t(o);
          }
        : t;
    },
    V = function (k, j, o) {
      var p = k.ganttChartView.ownerDocument,
        t;
      if (typeof k.contentInput === 'undefined') {
        t = p.createElement('input');
        k.contentInput = t;
        t.type = 'text';
        t.addEventListener(
          'focus',
          function () {
            $(k, o);
          },
          false,
        );
        var w = function () {
          k.content = t.value;
          k.scheduleChartView.onItemPropertyChanged(k, 'content', true, true);
          k.scheduleChartView.refreshItem(k);
        };
        typeof t.changeEventListener !== 'undefined' && t.removeEventListener('change', t.changeEventListener, true);
        t.addEventListener('change', w, true);
        t.changeEventListener = w;
        p = function (j) {
          if (j.keyCode == 13) {
            j.preventDefault();
            j.stopPropagation();
            w(j);
          }
        };
        typeof t.keyPressEventListener !== 'undefined' &&
          t.removeEventListener('keypress', t.keyPressEventListener, true);
        t.addEventListener('keypress', p, true);
        t.keyPressEventListener = p;
        t.addEventListener(
          'focus',
          function () {
            t.style.backgroundColor = 'White';
          },
          false,
        );
        t.addEventListener(
          'blur',
          function () {
            t.style.backgroundColor = 'Transparent';
          },
          false,
        );
      } else t = k.contentInput;
      t.value = k.content;
      p = '';
      if (k.hasChildren && (typeof k.isSummaryEnabled === 'undefined' || k.isSummaryEnabled)) p = '; font-weight: bold';
      t.setAttribute('style', 'background-color: Transparent; width: ' + j + 'px; border-width: 0px; padding: 0px' + p);
      return t;
    },
    P = function (k, j, o) {
      return function (p) {
        var t = p.ganttChartView.ownerDocument,
          w = t.createElementNS('http://www.w3.org/2000/svg', 'g');
        if (
          !o.isReadOnly &&
          !o.isChartReadOnly &&
          (typeof p.isReadOnly === 'undefined' || !p.isReadOnly) &&
          !o.areAssignmentsReadOnly
        ) {
          var x = j.getChartPosition(p.start, o),
            r = Math.max(j.getChartPosition(p.finish, o), x + 4),
            z = t.createElementNS('http://www.w3.org/2000/svg', 'line');
          z.setAttribute('x1', x);
          z.setAttribute('y1', o.barMargin + o.barHeight + 2);
          z.setAttribute('x2', r - 1);
          z.setAttribute('y2', o.barMargin + o.barHeight + 2);
          var B = o.assignmentThumbClass;
          typeof B !== 'undefined' && z.setAttribute('class', B);
          z.setAttribute('style', o.assignmentThumbStyle);
          z.style.visibility = 'hidden';
          w.appendChild(z);
          t = t.createElementNS('http://www.w3.org/2000/svg', 'rect');
          t.setAttribute('x', x);
          t.setAttribute('y', o.barMargin + o.barHeight - 2);
          t.setAttribute('width', Math.max(0, r - x - 1));
          t.setAttribute('height', 7);
          t.setAttribute('style', 'fill: White; fill-opacity: 0; cursor: move');
          t.addEventListener(
            'mouseover',
            function () {
              if (typeof j.draggingItem === 'undefined') z.style.visibility = 'visible';
            },
            true,
          );
          t.addEventListener(
            'mouseout',
            function () {
              z.style.visibility = 'hidden';
            },
            true,
          );
          w.appendChild(t);
          G(t, w, p, x, r, k, j, o);
        }
        return w;
      };
    },
    J = function (k) {
      var j = k.scheduleChartView;
      k.itemTop = k.scheduleChartVisibilityIndex * j.settings.itemHeight;
      j.refreshGridItem(k);
      if (typeof k.ganttChartItems !== 'undefined')
        for (var o = 0; o < k.ganttChartItems.length; o++) {
          var p = k.ganttChartItems[o];
          p.itemTop = k.itemTop;
          j.refreshChartItem(p);
        }
    },
    G = function (k, j, o, p, t, w, x, r) {
      var z = o.ganttChartView.ownerDocument;
      k.addEventListener(
        'mousedown',
        function (w) {
          if (w.button == 0) {
            delete x.cancelDrag;
            x.draggingItem = o;
            x.dragType = 'Assignment';
            x.style.cursor = k.style.cursor;
            x.draggingInitialY = w.clientY;
            x.draggingInitialThumbPosition = r.barMargin;
            w.preventDefault();
            j.itemLeft = p;
            j.itemRight = t;
            o.assignmentThumb = j;
            if (ToolTip && r.useUpdatingToolTips) {
              let toolTip =
                ToolTip.get(k) ||
                ToolTip.initialize(
                  void 0,
                  k,
                  {
                    duration: NaN,
                    containerStyle:
                      'cursor: default; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; border: 1px solid ' +
                      r.border +
                      '; background-color: White; color: Black; font-family: Arial; font-size: 12px; padding: 4px; margin-top: 1px',
                  },
                  'init warnings',
                );
              toolTip.setContent(o.content + ' \u2013');
              toolTip.show();
              toolTip.setPosition(toolTip.x + (t - p) + 4, toolTip.y - r.itemHeight - 1);
              toolTip.originalY = toolTip.y;
              x.toolTip = toolTip;
            }
          }
        },
        true,
      );
      if (typeof x.draggableAssignmentItems === 'undefined') x.draggableAssignmentItems = [];
      for (var B = false, G = 0; G < x.draggableAssignmentItems.length; G++)
        if (x.draggableAssignmentItems[G] == o) {
          B = true;
          break;
        }
      if (!B) {
        x.addEventListener(
          'mousemove',
          function (k) {
            if (!(typeof x.draggingItem === 'undefined' || x.draggingItem != o || x.dragType != 'Assignment')) {
              var B = Math.ceil((k.clientY - x.draggingInitialY) / r.itemHeight) * r.itemHeight;
              U(k.clientY, x);
              delete x.draggingItem;
              j = o.assignmentThumb;
              p = j.itemLeft;
              t = j.itemRight;
              if (typeof x.temporaryAssignmentThumb !== 'undefined') {
                try {
                  j.removeChild(x.temporaryAssignmentThumb);
                } catch (G) {}
                delete x.temporaryAssignmentThumb;
              }
              if (x.cancelDrag) {
                delete x.cancelDrag;
                delete x.draggingItem;
                x.style.cursor = 'default';
              } else {
                if (r.temporaryAssignmentThumbTemplate)
                  k = r.temporaryAssignmentThumbTemplate(
                    x,
                    p,
                    x.draggingInitialThumbPosition + B,
                    Math.max(4, t - p - 1),
                    r.barHeight,
                  );
                else {
                  k = z.createElementNS('http://www.w3.org/2000/svg', 'rect');
                  k.setAttribute('x', p);
                  k.setAttribute('y', x.draggingInitialThumbPosition + B);
                  k.setAttribute('width', Math.max(4, t - p - 1));
                  k.setAttribute('height', r.barHeight);
                  typeof r.temporaryAssignmentThumbClass !== 'undefined' &&
                    k.setAttribute('class', r.temporaryAssignmentThumbClass);
                  k.setAttribute('style', r.temporaryAssignmentThumbStyle);
                }
                x.temporaryAssignmentThumb = k;
                j.appendChild(k);
                x.draggingItem = o;
                if (ToolTip && r.useUpdatingToolTips) {
                  let toolTip = x.toolTip;
                  var k = Math.floor((o.itemTop + x.draggingInitialThumbPosition + B) / r.itemHeight),
                    J = null,
                    E = 0,
                    F;
                  for (F = 0; F < w.length; F++) {
                    var I = w[F];
                    if (
                      I.isVisible &&
                      !(typeof I.isHidden !== 'undefined' && I.isHidden) &&
                      typeof I.displayRowIndex === 'undefined'
                    ) {
                      if (E == k) {
                        I.hasChildren || (J = I);
                        break;
                      }
                      E++;
                    }
                  }
                  toolTip.setContent(o.content + ' \u2013' + (J != null ? ' ' + J.content : ''));
                  toolTip.setVerticalPosition(
                    Math.max(W(x) + x.chartHeaderContainer.clientHeight, toolTip.originalY + B),
                  );
                  if (typeof toolTip.originalX === 'undefined') toolTip.originalX = toolTip.x;
                  toolTip.setHorizontalPosition(toolTip.originalX);
                }
              }
            }
          },
          true,
        );
        z.addEventListener(
          'mouseup',
          function (k) {
            if (
              !(
                k.button != 0 ||
                typeof x.draggingItem === 'undefined' ||
                x.draggingItem != o ||
                x.dragType != 'Assignment'
              )
            ) {
              j = o.assignmentThumb;
              if (typeof x.temporaryAssignmentThumb !== 'undefined') {
                try {
                  j.removeChild(x.temporaryAssignmentThumb);
                } catch (p) {}
                delete x.temporaryAssignmentThumb;
              }
              var k = Math.ceil((k.clientY - x.draggingInitialY) / r.itemHeight) * r.itemHeight,
                k = Math.floor((o.itemTop + x.draggingInitialThumbPosition + k) / r.itemHeight),
                t = null,
                z = 0,
                B;
              for (B = 0; B < w.length; B++) {
                var F = w[B];
                if (
                  F.isVisible &&
                  !(typeof F.isHidden !== 'undefined' && F.isHidden) &&
                  typeof F.displayRowIndex === 'undefined'
                ) {
                  if (z == k) {
                    F.hasChildren || (t = F);
                    break;
                  }
                  z++;
                }
              }
              if (t != null) {
                var z = o.scheduleChartItem,
                  F = z.ganttChartItems,
                  G = [];
                for (B = 0; B < F.length; B++) F[B] != o && G.push(F[B]);
                z.ganttChartItems = G;
                x.onItemPropertyChanged(z, 'ganttChartItems', false, true);
                if (typeof t.ganttChartItems === 'undefined') t.ganttChartItems = [];
                t.ganttChartItems.push(o);
                x.onItemPropertyChanged(t, 'ganttChartItems', true, true);
                o.scheduleChartItem = t;
                x.onItemPropertyChanged(o, 'scheduleChartItem', true, true);
                o.displayRowIndex = k;
                o.isVirtuallyVisible = true;
                x.refreshChartItem(o);
                $(t, r);
              }
              delete x.draggingItem;
              x.style.cursor = 'default';
            }
          },
          true,
        );
        x.draggableAssignmentItems.push(o);
      }
    },
    W = function (k) {
      var j = 0;
      if (k.offsetParent) {
        do {
          j = j + k.offsetTop;
          k = k.offsetParent;
        } while (k);
      }
      return j;
    },
    U = function (k, j) {
      if (typeof j.draggingItem !== 'undefined') {
        var k = k - W(j),
          o,
          p;
        if (k < j.chartHeaderContainer.clientHeight + 24) {
          o = j.chartContentContainer.scrollTop;
          j.chartContentContainer.scrollTop = j.chartContentContainer.scrollTop - 20;
          if (typeof j.isDuringVerticalScrolling === 'undefined') {
            j.isDuringVerticalScrolling = true;
            setTimeout(function () {
              p = o - j.chartContentContainer.scrollTop;
              j.draggingInitialThumbPosition = j.draggingInitialThumbPosition - p;
              delete j.isDuringVerticalScrolling;
            }, 0);
          }
        } else if (k >= j.chartHeaderContainer.clientHeight + j.chartContentContainer.clientHeight - 24) {
          o = j.chartContentContainer.scrollTop;
          j.chartContentContainer.scrollTop = j.chartContentContainer.scrollTop + 20;
          if (typeof j.isDuringVerticalScrolling === 'undefined') {
            j.isDuringVerticalScrolling = true;
            setTimeout(function () {
              p = j.chartContentContainer.scrollTop - o;
              j.draggingInitialThumbPosition = j.draggingInitialThumbPosition + p;
              delete j.isDuringVerticalScrolling;
            }, 0);
          }
        }
      }
    },
    ja = function (k, j, o, p) {
      var t = p.indexOf(k);
      if (!(t < 0 || j < 0 || j == t || j >= p.length)) {
        p.splice(t, 1);
        p.splice(j, 0, k);
        refreshScheduleChartView(o);
        typeof o.settings.itemMoveHandler !== 'undefined' && o.settings.itemMoveHandler(k, t, j);
      }
    },
    K = function (rootDomNode, items, chartItems, copiedSettings) {
      for (var itemIdx = 0; itemIdx < items.length; itemIdx++) items[itemIdx].scheduleChartView = rootDomNode;
      rootDomNode.scheduleChartItems = chartItems;
      rootDomNode.settings = copiedSettings;
      rootDomNode.refreshScheduleChartItem = J;
      rootDomNode.initializeAssignmentDraggingThumb = function (j, o, p, t, r) {
        G(j, o, p, t, r, rootDomNode.items, rootDomNode, rootDomNode.settings);
      };
      rootDomNode.insertScheduleChartItem = function (p, t) {
        var r = p;
        t.scheduleChartView = rootDomNode;
        t.isVirtuallyVisible = true;
        t.scheduleChartIndex = r;
        var z = 0,
          B;
        for (B = 0; B < r; B++) chartItems[B].isHidden || z++;
        t.scheduleChartVisibilityIndex = z;
        t.isBarVisible = false;
        r = r < chartItems.length ? chartItems[r].index : items.length;
        chartItems.splice(t.scheduleChartIndex, 0, t);
        rootDomNode.insertItem(r++, t);
        if (typeof t.ganttChartItems !== 'undefined')
          for (B = 0; B < t.ganttChartItems.length; B++) {
            z = t.ganttChartItems[B];
            z.scheduleChartView = rootDomNode;
            z.scheduleChartItem = t;
            z.isHidden = t.isHidden;
            z.displayRowIndex = t.scheduleChartVisibilityIndex;
            z.indentation = t.indentation;
            rootDomNode.insertItem(r++, z);
          }
        for (B = t.scheduleChartIndex + 1; B < chartItems.length; B++) {
          r = chartItems[B];
          r.scheduleChartIndex = B;
          typeof r.scheduleChartVisibilityIndex !== 'undefined' && r.scheduleChartVisibilityIndex++;
          if (typeof r.ganttChartItems !== 'undefined')
            for (z = 0; z < r.ganttChartItems.length; z++)
              r.ganttChartItems[z].displayRowIndex = r.scheduleChartVisibilityIndex;
          J(r);
        }
      };
      rootDomNode.addScheduleChartItem = function (j) {
        rootDomNode.insertScheduleChartItem(chartItems.length, j);
      };
      rootDomNode.insertScheduleChartItems = function (j, o) {
        for (var p = 0; p < o.length; p++) rootDomNode.insertScheduleChartItem(j++, o[p]);
      };
      rootDomNode.addScheduleChartItems = function (j) {
        for (var o = 0; o < j.length; o++) rootDomNode.addScheduleChartItem(j[o]);
      };
      rootDomNode.removeScheduleChartItem = function (j) {
        if (typeof j.ganttChartItems !== 'undefined')
          for (var p = 0; p < j.ganttChartItems.length; p++) rootDomNode.removeItem(j.ganttChartItems[p]);
        chartItems.splice(j.scheduleChartIndex, 1);
        for (p = j.scheduleChartIndex; p < chartItems.length; p++) {
          var t = chartItems[p];
          t.scheduleChartIndex = p;
          typeof t.scheduleChartVisibilityIndex !== 'undefined' && t.scheduleChartVisibilityIndex--;
          if (typeof t.ganttChartItems !== 'undefined')
            for (var r = 0; r < t.ganttChartItems.length; r++)
              t.ganttChartItems[r].displayRowIndex = t.scheduleChartVisibilityIndex;
          J(t);
        }
        rootDomNode.removeItem(j);
      };
      rootDomNode.removeScheduleChartItems = function (j) {
        for (var o = 0; o < j.length; o++) rootDomNode.removeScheduleChartItem(j[o]);
      };
      rootDomNode.moveScheduleChartRange = function (j, p, t) {
        if (!(j < 0 || t < 0 || t == j || t > chartItems.length - p)) {
          var r = [],
            B;
          for (B = j; B < j + p; B++) r.push(chartItems[B]);
          chartItems.splice(j, p);
          for (B = 0; B < r.length; B++) chartItems.splice(t + B, 0, r[B]);
          refreshScheduleChartView(rootDomNode);
          if (typeof rootDomNode.settings.itemMoveHandler !== 'undefined')
            for (B = 0; B < r.length; B++) rootDomNode.settings.itemMoveHandler(r[B], j + B, t + B);
        }
      };
      rootDomNode.moveScheduleChartItem = function (j, p) {
        ja(j, p, rootDomNode, chartItems);
      };
      rootDomNode.moveScheduleChartItemUp = function (j) {
        var p = chartItems.indexOf(j);
        p <= 0 || ja(j, p - 1, rootDomNode, chartItems);
      };
      rootDomNode.moveScheduleChartItemDown = function (j) {
        var p = chartItems.indexOf(j);
        p < 0 || p >= chartItems.length - 1 || ja(j, p + 1, rootDomNode, chartItems);
      };
    },
    r = function (k, j) {
      k.items = j;
      k.refresh = function () {
        refreshScheduleChartView(k);
      };
    };
  return {
    initialize: ScheduleChartViewConstructor,
    refresh: refreshScheduleChartView,
    getDefaultColumns: getDefaultColumns,
    getDefaultScales: GanttChartView.getDefaultScales,
    getDefaultStyleDefinitionTemplate: GanttChartView.getDefaultStyleDefinitionTemplate,
    getDefaultStandardTaskTemplate: GanttChartView.getDefaultStandardTaskTemplate,
    getDefaultMilestoneTaskTemplate: GanttChartView.getDefaultMilestoneTaskTemplate,
    getDefaultItemTemplate: GanttChartView.getDefaultItemTemplate,
    getDefaultAssignmentsTemplate: GanttChartView.getDefaultAssignmentsTemplate,
    initializeTaskDraggingThumbs: GanttChartView.initializeTaskDraggingThumbs,
    initializeDependencyDraggingThumb: GanttChartView.initializeDependencyDraggingThumb,
    initializeAssignmentDraggingThumb: function (k, j, o, p, t) {
      o.scheduleChartView.initializeAssignmentDraggingThumb(k, j, o, p, t);
    },
    getWorkingTime: GanttChartView.getWorkingTime,
    getEffort: GanttChartView.getEffort,
    getFinish: GanttChartView.getFinish,
    getStart: GanttChartView.getStart,
    getCompletion: GanttChartView.getCompletion,
    getCompletedFinish: GanttChartView.getCompletedFinish,
    textColumnTemplateBase: GanttChartView.textColumnTemplateBase,
    textInputColumnTemplateBase: GanttChartView.textInputColumnTemplateBase,
    getOutputDate: GanttChartView.getOutputDate,
    getInputDate: GanttChartView.getInputDate,
  };
})();

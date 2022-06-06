/* eslint-disable */
import { GanttChartView } from './DlhGanttChart';
let getBooleanNode = undefined;

export const LoadChartView = (function() {
  var L = function(r, k, j, o) {
      var p;
      r.isLoadChartInitializing = true;
      for (var o = k, k = [], t = (p = 0); t < o.length; t++) {
        var w = o[t];
        if (typeof w.isExported !== 'undefined' && w.isExported) k.push(w);
        else if (!(typeof w.loadChartItem !== 'undefined' && w.loadChartItem != w)) {
          w.loadChartIndex = w.scheduleChartIndex = p++;
          w.isBarVisible = false;
          k.push(w);
          if (typeof w.ganttChartItems !== 'undefined')
            for (var x = 0; x < w.ganttChartItems.length; x++) {
              var A = w.ganttChartItems[x];
              A.loadChartItem = A.scheduleChartItem = w;
              A.displayRowIndex = w.loadChartIndex;
              k.push(A);
            }
        }
      }
      typeof j !== 'object' && (j = {});
      GanttChartView.initializeItems(k, j);
      p = j;
      t = k;
      X(p);
      if (typeof p.gridWidth === 'undefined') p.gridWidth = '15%';
      if (typeof p.chartWidth === 'undefined') p.chartWidth = p.isGridVisible ? '85%' : '100%';
      if (typeof p.columns === 'undefined') p.columns = ba(t, p);
      if (typeof p.normalAllocationBarStyle === 'undefined' && p.normalAllocationBarClass == null)
        switch (p.theme) {
          default:
            p.normalAllocationBarStyle = 'fill: Blue; fill-opacity: 0.8; stroke: Blue; stroke-width: 0.65px';
            break;
          case 'Aero':
            p.normalAllocationBarStyle = 'fill: Blue; stroke: Blue';
        }
      if (typeof p.underAllocationBarStyle === 'undefined' && p.underAllocationBarClass == null)
        switch (p.theme) {
          default:
            p.underAllocationBarStyle = 'fill: Blue; fill-opacity: 0.8; stroke: Blue; stroke-width: 0.65px';
            break;
          case 'Aero':
            p.underAllocationBarStyle = 'fill: Blue; stroke: Blue';
        }
      if (typeof p.overAllocationBarStyle === 'undefined' && p.overAllocationBarClass == null)
        switch (p.theme) {
          default:
            p.overAllocationBarStyle = 'fill: Red; fill-opacity: 0.8; stroke: Red; stroke-width: 0.65px';
            break;
          case 'Aero':
            p.overAllocationBarStyle = 'fill: Red; stroke: Red';
        }
      if (typeof p.maxDisplayedUnits === 'undefined') p.maxDisplayedUnits = 1.5;
      if (typeof p.allocationTemplate === 'undefined') p.allocationTemplate = J();
      p.standardTaskTemplate = p.allocationTemplate;
      p.milestoneTaskTemplate = p.allocationTemplate;
      p.areTaskDependenciesVisible = false;
      p.isBaselineVisible = false;
      p.isTaskCompletedEffortVisible = false;
      p.areTaskAssignmentsVisible = false;
      ja(r, k, o, j);
      GanttChartView.initialize(r, k, j, 'init warning');
      K(r, k, j);
      r.isLoadChartInitializing = false;
      r.isLoadChartInitialized = true;
      return r;
    },
    I = function(r) {
      L(r, r.loadChartItems, r.settings, r.license);
    },
    X = function(r) {
      if (typeof r.target === 'undefined') r.target = 'Standard';
      if (typeof r.theme === 'undefined') r.theme = 'Modern';
      if (typeof r.isGridVisible === 'undefined')
        switch (r.target) {
          default:
            r.isGridVisible = true;
            break;
          case 'Phone':
            r.isGridVisible = false;
        }
      if (typeof r.isReadOnly === 'undefined') r.isReadOnly = true;
      if (typeof r.selectionMode === 'undefined') r.selectionMode = 'Focus';
      if (typeof r.isVirtualizing === 'undefined') r.isVirtualizing = true;
    },
    ba = function(r, k) {
      typeof k !== 'object' && (k = {});
      X(k);
      var headerArr = [
        { header: '', width: 32, isSelection: true },
        {
          header: 'Resource',
          width: 100,
        },
      ];
      headerArr[0].cellTemplate = la(k, headerArr[0], r);
      headerArr[1].cellTemplate = $(k, headerArr[1]);
      headerArr[1].exportCellTemplate = $(k, headerArr[1], false);
      k.selectionMode != 'Single' &&
      k.selectionMode != 'Extended' &&
      k.selectionMode != 'ExtendedFocus' &&
      headerArr.splice(0, 1);
      return headerArr;
    },
    la = function(r, k) {
      return function(j) {
        return !k.isReadOnly ? ea(j) : getBooleanNode && getBooleanNode(j.ganttChartView.ownerDocument, j.isSelected);
      };
    },
    ea = function(r) {
      var k = r.ganttChartView.ownerDocument,
        j;
      if (typeof r.selectionInput === 'undefined') {
        j = k.createElement('input');
        r.selectionInput = j;
        j.type = 'checkbox';
        j.setAttribute('style', 'margin: 0px');
      }
      else j = r.selectionInput;
      if (r.isSelected) {
        j.setAttribute('checked', 'checked');
        if (!j.checked) j.checked = true;
      }
      else if (j.checked) j.checked = false;
      var o = function() {
        j.checked ? S(r) : typeof r.loadChartView !== 'undefined' && r.loadChartView.unselectItem(r);
      };
      typeof j.changeEventListener !== 'undefined' && j.removeEventListener('change', j.changeEventListener, true);
      j.addEventListener('change', o, true);
      j.changeEventListener = o;
      k = function(j) {
        if (j.keyCode == 13) {
          j.preventDefault();
          j.stopPropagation();
          o(j);
        }
      };
      typeof j.keyPressEventListener !== 'undefined' &&
      j.removeEventListener('keypress', j.keyPressEventListener, true);
      j.addEventListener('keypress', k, true);
      j.keyPressEventListener = k;
      return j;
    },
    S = function(r) {
      typeof r.loadChartView !== 'undefined' && r.loadChartView.selectItem(r);
    },
    $ = function(r, k, j) {
      var o = function(j) {
        var k = j.content,
          j = j.ganttChartView.ownerDocument.createElement('span');
        j.innerHTML = k;
        return j;
      };
      return (typeof j === 'undefined' || j) && !r.isReadOnly && !r.isGridReadOnly && !r.isContentReadOnly
        ? function(j) {
          return !k.isReadOnly && (typeof j.isReadOnly === 'undefined' || !j.isReadOnly)
            ? O(j, Math.max(0, k.width - 2), r)
            : o(j);
        }
        : o;
    },
    O = function(r, k, j) {
      var o = r.ganttChartView.ownerDocument,
        p;
      if (typeof r.contentInput === 'undefined') {
        p = o.createElement('input');
        r.contentInput = p;
        p.type = 'text';
        p.addEventListener(
          'focus',
          function() {
            if (typeof r.loadChartView !== 'undefined') r.loadChartView.currentItem = r;
            (j.selectionMode == 'Focus' || j.selectionMode == 'ExtendedFocus') && !r.isSelected && S(r);
          },
          false,
        );
        var t = function() {
          r.content = p.value;
          r.loadChartView.onItemPropertyChanged(r, 'content', true, true);
          r.loadChartView.refreshItem(r);
        };
        typeof p.changeEventListener !== 'undefined' && p.removeEventListener('change', p.changeEventListener, true);
        p.addEventListener('change', t, true);
        p.changeEventListener = t;
        o = function(j) {
          if (j.keyCode == 13) {
            j.preventDefault();
            j.stopPropagation();
            t(j);
          }
        };
        typeof p.keyPressEventListener !== 'undefined' &&
        p.removeEventListener('keypress', p.keyPressEventListener, true);
        p.addEventListener('keypress', o, true);
        p.keyPressEventListener = o;
        p.addEventListener(
          'focus',
          function() {
            p.style.backgroundColor = 'White';
          },
          false,
        );
        p.addEventListener(
          'blur',
          function() {
            p.style.backgroundColor = 'Transparent';
          },
          false,
        );
      }
      else p = r.contentInput;
      p.value = r.content;
      p.setAttribute('style', 'background-color: Transparent; width: ' + k + 'px; border-width: 0px; padding: 0px');
      return p;
    },
    V = function(r) {
      for (var k = 0, j = 0; j < r.length; j++) k = k + r[j].width;
      return k;
    },
    P = function(r) {
      var k = r.loadChartView;
      k.refreshGridItem(r);
      if (typeof r.ganttChartItems !== 'undefined')
        for (var j = 0; j < r.ganttChartItems.length; j++) k.refreshChartItem(r.ganttChartItems[j]);
    },
    J = function(r, k, j) {
      return function(o) {
        var p = typeof k !== 'undefined' ? k : o.loadChartView,
          t = typeof j !== 'undefined' ? j : p.settings,
          r = p.ownerDocument,
          x;
        x = p.ownerDocument;
        if (typeof o.chartItemArea === 'undefined')
          o.chartItemArea = x.createElementNS('http://www.w3.org/2000/svg', 'g');
        for (x = o.chartItemArea.childNodes.length; x-- > 0;)
          o.chartItemArea.removeChild(o.chartItemArea.childNodes[x]);
        x = o.chartItemArea;
        var A = p.getChartPosition(o.start, t),
          p = Math.max(p.getChartPosition(o.finish, t), A + 4),
          r = r.createElementNS('http://www.w3.org/2000/svg', 'rect'),
          z = typeof o.units !== 'undefined' ? o.units : 1,
          B = (Math.min(z, t.maxDisplayedUnits) / t.maxDisplayedUnits) * t.barHeight;
        r.setAttribute('x', A);
        r.setAttribute('y', t.barMargin + (t.barHeight - B));
        r.setAttribute('width', Math.max(0, p - A - 1));
        r.setAttribute('height', B);
        A = z == 1 ? t.normalAllocationBarClass : z < 1 ? t.underAllocationBarClass : t.overAllocationBarClass;
        if (typeof o.allocationBarClass !== 'undefined') A = o.allocationBarClass;
        if (typeof A !== 'undefined') r.setAttribute('class', A);
        else {
          t = z == 1 ? t.normalAllocationBarStyle : z < 1 ? t.underAllocationBarStyle : t.overAllocationBarStyle;
          if (typeof o.allocationBarStyle !== 'undefined') t = o.allocationBarStyle;
          typeof t !== 'undefined' && r.setAttribute('style', t);
        }
        x.appendChild(r);
        return x;
      };
    },
    G = function(r, k, j, o) {
      var p = o.indexOf(r);
      if (!(p < 0 || k < 0 || k == p || k >= o.length)) {
        o.splice(p, 1);
        o.splice(k, 0, r);
        I(j);
        typeof j.settings.itemMoveHandler !== 'undefined' && j.settings.itemMoveHandler(r, p, k);
      }
    },
    W = function(r, k) {
      for (var j in k)
        (j.indexOf('custom') != 0 && j.indexOf('description') != 0) || (typeof r[j] === 'undefined' && (r[j] = k[j]));
    },
    U = function(r, k, j, o, p, t, w, x, A, z, B, G, J, I, K, O, E) {
      var F,
        P = [],
        T;
      if (typeof o !== 'undefined')
        for (F = 0; F < o.length; F++) {
          T = E.columns[o[F]];
          P.push({
            header: T.header,
            width: T.width,
            headerClass: T.headerClass,
            headerStyle: T.headerStyle,
            cellClass: T.cellClass,
            cellStyle: T.cellStyle,
            cellTemplate: typeof T.exportCellTemplate !== 'undefined' ? T.exportCellTemplate : T.cellTemplate,
          });
        }
      else
        for (F = 0; F < E.columns.length; F++) {
          T = E.columns[F];
          T.isSelection ||
          P.push({
            header: T.header,
            width: T.width,
            headerClass: T.headerClass,
            headerStyle: T.headerStyle,
            cellClass: T.cellClass,
            cellStyle: T.cellStyle,
            cellTemplate: typeof T.exportCellTemplate !== 'undefined' ? T.exportCellTemplate : T.cellTemplate,
          });
        }
      if (typeof j === 'undefined') j = E.isGridVisible;
      F = j ? V(P) + 1 : 0;
      if (typeof p !== 'undefined') {
        if (typeof w === 'undefined' || !w) p = new Date(p.valueOf() - p.getTimezoneOffset() * 6e4);
      }
      else p = E.timelineStart;
      if (typeof t !== 'undefined') {
        if (typeof w === 'undefined' || !w) t = new Date(t.valueOf() - t.getTimezoneOffset() * 6e4);
      }
      else t = E.timelineFinish;
      p = {
        isExport: true,
        isReadOnly: true,
        selectionMode: 'None',
        isVirtualizing: false,
        isGridVisible: j,
        isSplitterEnabled: false,
        gridWidth: F + 'px',
        columns: P,
        allowUserToResizeColumns: true,
        isGridRowClickTimeScrollingEnabled: false,
        isMouseWheelZoomEnabled: false,
        timelineStart: p,
        timelineFinish: t,
        hourWidth: typeof x !== 'undefined' ? x : E.hourWidth,
        displayedTime: typeof p !== 'undefined' ? p : E.timelineStart,
        currentTime: E.currentTime,
        isTaskToolTipVisible: false,
        isDependencyToolTipVisible: false,
        areTaskDependenciesVisible: false,
        isBaselineVisible: false,
        isTaskCompletedEffortVisible: false,
        areTaskAssignmentsVisible: false,
        containerClass: E.containerClass,
        containerStyle: E.containerStyle,
        border: E.border,
        theme: E.theme,
        headerBackground: E.headerBackground,
        headerHeight: E.headerHeight,
        itemHeight: E.itemHeight,
        itemClass: E.itemClass,
        itemStyle: E.itemStyle,
        scales: [],
        visibleWeekStart: E.visibleWeekStart,
        visibleWeekFinish: E.visibleWeekFinish,
        workingWeekStart: E.workingWeekStart,
        workingWeekFinish: E.workingWeekFinish,
        visibleDayStart: E.visibleDayStart,
        visibleDayFinish: E.visibleDayFinish,
        specialNonworkingDays: E.specialNonworkingDays,
        barMargin: E.barMargin,
        barHeight: E.barHeight,
        normalAllocationBarClass: E.normalAllocationBarClass,
        underAllocationBarClass: E.underAllocationBarClass,
        overAllocationBarClass: E.overAllocationBarClass,
        normalAllocationBarStyle: E.normalAllocationBarStyle,
        underAllocationBarStyle: E.underAllocationBarStyle,
        overAllocationBarStyle: E.overAllocationBarStyle,
        alternativeItemClass: E.alternativeItemClass,
        alternativeChartItemClass: E.alternativeChartItemClass,
        alternativeItemStyle: E.alternativeItemStyle,
        alternativeChartItemStyle: E.alternativeChartItemStyle,
        gridLines: E.gridLines,
        horizontalGridLines: E.horizontalGridLines,
        verticalGridLines: E.verticalGridLines,
        horizontalChartLines: E.horizontalChartLines,
        target: E.target,
        months: E.months,
        daysOfWeek: E.daysOfWeek,
        weekStartDay: E.weekStartDay,
        dateFormatter: E.dateFormatter,
        dateTimeFormatter: E.dateTimeFormatter,
        isRelativeToTimezone: E.isRelativeToTimezone,
        allocationTemplate: E.allocationTemplate,
      };
      p.timelineStart = GanttChartView.getWeekStart(p.timelineStart, p.weekStartDay);
      p.timelineFinish = GanttChartView.getWeekFinish(p.timelineFinish, p.weekStartDay);
      K = K.getChartPosition(p.timelineFinish, p) - K.getChartPosition(p.timelineStart, p);
      p.chartWidth = K + 'px';
      var U = F + K + 2 + (j ? 1 : 0);
      for (F = 0; F < E.scales.length; F++) {
        j = E.scales[F];
        p.scales.push({
          scaleType: j.scaleType,
          isHeaderVisible: j.isHeaderVisible,
          headerHeight: j.headerHeight,
          headerTextFormat: j.headerTextFormat,
          headerClass: j.headerClass,
          headerStyle: j.headerStyle,
          isHighlightingVisible: j.isHighlightingVisible,
          highlightingClass: j.highlightingClass,
          highlightingStyle: j.highlightingStyle,
          isSeparatorVisible: j.isSeparatorVisible,
          separatorClass: j.separatorClass,
          separatorStyle: j.separatorStyle,
        });
      }
      var L,
        S,
        $ = false;
      if (B != null && typeof B.createElement !== 'undefined') L = B;
      else {
        if (B != null && typeof B.focus !== 'undefined') S = B;
        else {
          S = window.open(
            '',
            B != null ? B : '_blank',
            typeof G !== 'undefined' && G && (typeof I === 'undefined' || I)
              ? 'width=320,height=100,location=no,menubar=no,toolbar=no,status=no,scrollbars=yes'
              : '',
          );
          $ = true;
        }
        L = S.document;
        try {
          var X = document.head.getElementsByTagName('link');
          for (F = 0; F < X.length; F++) {
            var ba = X[F],
              ea = L.adoptNode(ba.cloneNode(true));
            ea.href = ba.href;
            L.head.appendChild(ea);
          }
        } catch (ja) {
        }
      }
      L.title = typeof r !== 'undefined' ? r : 'Exported chart' + (typeof G !== 'undefined' && G ? ' (printable)' : '');
      typeof A === 'undefined' && (A = 0);
      typeof z === 'undefined' && (z = O.length - 1);
      r = [];
      for (F = X = 0; F < O.length; F++) {
        B = O[F];
        if (
          !(
            (typeof B.displayRowIndex !== 'undefined' && (B.displayRowIndex < A || B.displayRowIndex > z)) ||
            (typeof B.displayRowIndex === 'undefined' && (X++ < A || X > z + 1))
          )
        ) {
          E = {
            content: B.content,
            start: B.start,
            finish: B.finish,
            units: B.units,
            isBarVisible: B.isBarVisible,
            isRelativeToTimezone: B.isRelativeToTimezone,
            class: B['class'],
            style: B.style,
            barClass: B.barClass,
            barStyle: B.barStyle,
            isSummaryEnabled: B.isSummaryEnabled,
            isParentSummarizationEnabled: B.isParentSummarizationEnabled,
            isHidden: B.isHidden,
            isExported: true,
            tag: B,
          };
          if (typeof B.displayRowIndex !== 'undefined') E.displayRowIndex = B.displayRowIndex - A;
          W(E, B);
          r.push(E);
          B.exportItem = E;
        }
      }
      var ma = L.createElement('p');
      ma.innerHTML = typeof k !== 'undefined' ? k : '';
      L.body.appendChild(ma);
      var ca = L.createElement('div');
      ca.setAttribute('style', 'width: ' + U + 'px');
      try {
        LoadChartView.initialize(ca, r, p, 'init warning');
      } catch (la) {
      }
      setTimeout(function() {
        $ && L.body.setAttribute('style', 'margin: 0px');
        var j = L.createElement('div');
        j.appendChild(ca);
        L.body.replaceChild(j, ma);
        if (J) {
          j.setAttribute('style', 'width: ' + ca.offsetHeight + 'px; height: ' + U + 'px; overflow: hidden');
          j = Math.round((ca.offsetWidth - ca.offsetHeight) / 2);
          ca.setAttribute(
            'style',
            'width: ' +
            U +
            'px; transform: rotate(90deg) translateX(' +
            j +
            'px) translateY(' +
            j +
            'px); -webkit-transform: rotate(90deg) translateX(' +
            j +
            'px) translateY(' +
            j +
            'px)',
          );
        }
        L.close();
        if (typeof S !== void 0) {
          S.focus();
          if (typeof G !== 'undefined' && G) {
            S.print();
            (typeof I === 'undefined' || I) && S.close();
          }
        }
      }, 0);
    },
    ja = function(r, k, j, o) {
      for (var p = 0; p < k.length; p++) k[p].loadChartView = k[p].scheduleChartView = r;
      r.loadChartItems = r.scheduleChartItems = j;
      r.settings = o;
      r.refreshLoadChartItem = P;
      r.insertLoadChartItem = function(o, p) {
        var x = o;
        p.loadChartView = p.scheduleChartView = r;
        p.isVirtuallyVisible = true;
        p.loadChartIndex = p.scheduleChartIndex = x;
        p.isBarVisible = false;
        x = x < j.length ? j[x].index : k.length;
        j.splice(p.loadChartIndex, 0, p);
        r.insertItem(x++, p);
        if (typeof p.ganttChartItems !== 'undefined')
          for (var A = 0; A < p.ganttChartItems.length; A++) {
            var z = p.ganttChartItems[A];
            z.loadChartView = z.scheduleChartView = r;
            z.loadChartItem = z.scheduleChartItem = p;
            z.displayRowIndex = p.loadChartIndex;
            r.insertItem(x++, z);
          }
        for (x = p.loadChartIndex + 1; x < j.length; x++) {
          A = j[x];
          A.loadChartIndex = A.scheduleChartIndex = x;
          if (typeof A.ganttChartItems !== 'undefined')
            for (z = 0; z < A.ganttChartItems.length; z++) A.ganttChartItems[z].displayRowIndex = A.loadChartIndex;
          P(A);
        }
      };
      r.addLoadChartItem = function(k) {
        r.insertLoadChartItem(j.length, k);
      };
      r.insertLoadChartItems = function(j, k) {
        for (var o = 0; o < k.length; o++) r.insertLoadChartItem(j++, k[o]);
      };
      r.addLoadChartItems = function(j) {
        for (var k = 0; k < j.length; k++) r.addLoadChartItem(j[k]);
      };
      r.removeLoadChartItem = function(k) {
        if (typeof k.ganttChartItems !== 'undefined')
          for (var o = 0; o < k.ganttChartItems.length; o++) r.removeItem(k.ganttChartItems[o]);
        r.removeItem(k);
        j.splice(k.loadChartIndex, 1);
        for (k = k.loadChartIndex; k < j.length; k++) {
          o = j[k];
          o.loadChartIndex = o.scheduleChartIndex = k;
          if (typeof o.ganttChartItems !== 'undefined')
            for (var p = 0; p < o.ganttChartItems.length; p++) o.ganttChartItems[p].displayRowIndex = o.loadChartIndex;
          P(o);
        }
      };
      r.removeLoadChartItems = function(j) {
        for (var k = 0; k < j.length; k++) r.removeLoadChartItem(j[k]);
      };
      r.moveLoadChartRange = function(k, o, p) {
        if (!(k < 0 || p < 0 || p == k || p > j.length - o)) {
          var A = [],
            z;
          for (z = k; z < k + o; z++) A.push(j[z]);
          j.splice(k, o);
          for (z = 0; z < A.length; z++) j.splice(p + z, 0, A[z]);
          I(r);
          if (typeof r.settings.itemMoveHandler !== 'undefined')
            for (z = 0; z < A.length; z++) r.settings.itemMoveHandler(A[z], k + z, p + z);
        }
      };
      r.moveLoadChartItem = function(k, o) {
        G(k, o, r, j);
      };
      r.moveLoadChartItemUp = function(k) {
        var o = j.indexOf(k);
        o <= 0 || G(k, o - 1, r, j);
      };
      r.moveLoadChartItemDown = function(k) {
        var o = j.indexOf(k);
        o < 0 || o >= j.length - 1 || G(k, o + 1, r, j);
      };
    },
    K = function(r, k, j) {
      r.items = k;
      r.refresh = function() {
        I(r);
      };
      r.exportContent = function(o, p) {
        typeof o === 'undefined' && (o = {});
        U(
          o.title,
          o.preparingMessage,
          o.isGridVisible,
          o.columnIndexes,
          o.timelineStart,
          o.timelineFinish,
          o.isRelativeToTimezone,
          o.hourWidth,
          o.startRowIndex,
          o.endRowIndex,
          p,
          false,
          o.rotate,
          false,
          r,
          k,
          j,
        );
      };
      r.print = function(o) {
        typeof o === 'undefined' && (o = {});
        U(
          o.title,
          o.preparingMessage,
          o.isGridVisible,
          o.columnIndexes,
          o.timelineStart,
          o.timelineFinish,
          o.isRelativeToTimezone,
          o.hourWidth,
          o.startRowIndex,
          o.endRowIndex,
          null,
          true,
          o.rotate,
          o.autoClose,
          r,
          k,
          j,
        );
      };
    };
  return {
    initialize: L,
    refresh: I,
    getDefaultColumns: ba,
    getDefaultScales: GanttChartView.getDefaultScales,
    getDefaultAllocationTemplate: J,
    getDefaultItemTemplate: GanttChartView.getDefaultItemTemplate,
    getWorkingTime: GanttChartView.getWorkingTime,
    textColumnTemplateBase: GanttChartView.textColumnTemplateBase,
    textInputColumnTemplateBase: GanttChartView.textInputColumnTemplateBase,
    getOutputDate: GanttChartView.getOutputDate,
    getInputDate: GanttChartView.getInputDate,
  };
})();

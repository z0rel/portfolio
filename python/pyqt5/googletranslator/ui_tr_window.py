# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'tr_window.ui'
#
# Created by: PyQt5 UI code generator 5.7.1
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets

class Ui_Form(object):
    def setupUi(self, Form):
        Form.setObjectName("Form")
        Form.resize(1107, 751)
        icon = QtGui.QIcon()
        icon.addPixmap(QtGui.QPixmap(":/head_icon/books online.ico"), QtGui.QIcon.Normal, QtGui.QIcon.Off)
        Form.setWindowIcon(icon)
        self.verticalLayout = QtWidgets.QVBoxLayout(Form)
        self.verticalLayout.setObjectName("verticalLayout")
        self.horizontalLayout = QtWidgets.QHBoxLayout()
        self.horizontalLayout.setContentsMargins(0, 0, 0, 0)
        self.horizontalLayout.setObjectName("horizontalLayout")
        self.checkBox_scanning = QtWidgets.QCheckBox(Form)
        self.checkBox_scanning.setChecked(True)
        self.checkBox_scanning.setObjectName("checkBox_scanning")
        self.horizontalLayout.addWidget(self.checkBox_scanning)
        spacerItem = QtWidgets.QSpacerItem(40, 20, QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.label = QtWidgets.QLabel(Form)
        self.label.setIndent(5)
        self.label.setObjectName("label")
        self.horizontalLayout.addWidget(self.label)
        self.comboBox_dir = QtWidgets.QComboBox(Form)
        self.comboBox_dir.setObjectName("comboBox_dir")
        self.comboBox_dir.addItem("")
        self.comboBox_dir.addItem("")
        self.comboBox_dir.addItem("")
        self.horizontalLayout.addWidget(self.comboBox_dir)
        self.checkBox_strip = QtWidgets.QCheckBox(Form)
        self.checkBox_strip.setChecked(True)
        self.checkBox_strip.setObjectName("checkBox_strip")
        self.horizontalLayout.addWidget(self.checkBox_strip)
        self.spinBox_textsize = QtWidgets.QSpinBox(Form)
        self.spinBox_textsize.setMinimum(1)
        self.spinBox_textsize.setProperty("value", 12)
        self.spinBox_textsize.setObjectName("spinBox_textsize")
        self.horizontalLayout.addWidget(self.spinBox_textsize)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.tabWidget = QtWidgets.QTabWidget(Form)
        self.tabWidget.setObjectName("tabWidget")
        self.tab_tr = QtWidgets.QWidget()
        self.tab_tr.setObjectName("tab_tr")
        self.verticalLayout_2 = QtWidgets.QVBoxLayout(self.tab_tr)
        self.verticalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_2.setObjectName("verticalLayout_2")
        self.textEdit_tr = QtWidgets.QTextEdit(self.tab_tr)
        self.textEdit_tr.setReadOnly(False)
        self.textEdit_tr.setObjectName("textEdit_tr")
        self.verticalLayout_2.addWidget(self.textEdit_tr)
        self.tabWidget.addTab(self.tab_tr, "")
        self.tab_log = QtWidgets.QWidget()
        self.tab_log.setObjectName("tab_log")
        self.verticalLayout_3 = QtWidgets.QVBoxLayout(self.tab_log)
        self.verticalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_3.setObjectName("verticalLayout_3")
        self.textEdit_log = QtWidgets.QTextEdit(self.tab_log)
        self.textEdit_log.setReadOnly(True)
        self.textEdit_log.setObjectName("textEdit_log")
        self.verticalLayout_3.addWidget(self.textEdit_log)
        self.tabWidget.addTab(self.tab_log, "")
        self.verticalLayout.addWidget(self.tabWidget)

        self.retranslateUi(Form)
        self.comboBox_dir.setCurrentIndex(0)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(Form)

    def retranslateUi(self, Form):
        _translate = QtCore.QCoreApplication.translate
        Form.setWindowTitle(_translate("Form", "GtkTr"))
        self.checkBox_scanning.setText(_translate("Form", "Сканирование буфера"))
        self.label.setText(_translate("Form", "Направление перевода:"))
        self.comboBox_dir.setToolTip(_translate("Form", "Направление перевода"))
        self.comboBox_dir.setItemText(0, _translate("Form", "auto"))
        self.comboBox_dir.setItemText(1, _translate("Form", "en→ru"))
        self.comboBox_dir.setItemText(2, _translate("Form", "ru→en"))
        self.checkBox_strip.setText(_translate("Form", "Объединять строки абзацев"))
        self.spinBox_textsize.setPrefix(_translate("Form", "Размер шрифта: "))
        self.textEdit_tr.setHtml(_translate("Form", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'Noto Sans\'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Test text... <a name=\"darkcyan\"></a><span style=\" color:#008b8b;\">D</span><span style=\" color:#008b8b;\">arkcyan text. </span><a name=\"darkcyan\"></a><span style=\" color:#008b8b;\">B</span><span style=\" font-weight:600;\">old text. </span>Default text.</p></body></html>"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_tr), _translate("Form", "Tr"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_log), _translate("Form", "Log"))

import tr_resources_rc

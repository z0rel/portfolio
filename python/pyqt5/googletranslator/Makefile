
RESOURCES_OBJ = tr_resources_rc.py
UI_OBJ = ui_tr_window.py


$(RESOURCES_OBJ): tr_resources.qrc
		pyrcc5 $< -o $@
                 

$(UI_OBJ): tr_window.ui
		pyuic5 $< > $@


all: $(RESOURCES_OBJ) $(UI_OBJ)


.PHONY: all
.DEFAULT_GOAL := all	


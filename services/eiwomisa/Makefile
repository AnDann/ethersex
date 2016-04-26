TOPDIR ?= ../..
include $(TOPDIR)/.config

$(EIWOMISA_SUPPORT)_SRC += services/eiwomisa/eiwomisa.c services/eiwomisa/eiwomisa_pwm.c
$(EIWOMISA_SUPPORT)_ECMD_SRC += services/eiwomisa/eiwomisa_ecmd.c
$(EIWOMISA_MQTT_SUPPORT)_SRC += services/eiwomisa/eiwomisa_mqtt.c

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk

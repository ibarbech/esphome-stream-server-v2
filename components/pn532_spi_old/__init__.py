import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi, pn532_old
from esphome.const import CONF_ID

AUTO_LOAD = ['pn532_old']
CODEOWNERS = ['@OttoWinter', '@jesserockz']
DEPENDENCIES = ['spi']

pn532_spi_ns = cg.esphome_ns.namespace('pn532_spi_old')
PN532Spi = pn532_spi_ns.class_('PN532Spi', pn532_old.PN532, spi.SPIDevice)

CONFIG_SCHEMA = cv.All(pn532_old.PN532_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(PN532Spi),
}).extend(spi.spi_device_schema(cs_pin_required=True)))


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield pn532_old.setup_pn532(var, config)
    yield spi.register_spi_device(var, config)
